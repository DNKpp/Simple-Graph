#ifndef SL_SEARCHER_HPP
#define SL_SEARCHER_HPP

#pragma once

#include <algorithm>
#include <cassert>
#include <iterator>
#include <optional>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace sl
{
namespace graph
{
	template <class TVertexType, class CostType>
	struct BaseNodeData
	{
		std::optional<TVertexType> parent;
		CostType cost;

		friend bool operator <(const BaseNodeData& _lhs, const BaseNodeData& _rhs)
		{
			return _lhs.cost < _rhs.cost;
		}

		friend bool operator >(const BaseNodeData& _lhs, const BaseNodeData& _rhs)
		{
			return _lhs.cost > _rhs.cost;
		}
	};

	template <class TVertexType, class CostType>
	struct HeuristicNodeData
	{
		std::optional<TVertexType> parent;
		CostType cost;
		CostType heuristic;

		int combined_cost() const
		{
			return cost + heuristic;
		}

		friend bool operator <(const HeuristicNodeData& _lhs, const HeuristicNodeData& _rhs)
		{
			return _lhs.combined_cost() < _rhs.combined_cost();
		}

		friend bool operator >(const HeuristicNodeData& _lhs, const HeuristicNodeData& _rhs)
		{
			return _lhs.combined_cost() > _rhs.combined_cost();
		}
	};

	template <class TVertexType, class TNodeData>
	struct Node
	{
		TVertexType vertex;
		TNodeData data;
	};
	
namespace detail
{
	template <class TVertexType, class TClosedList>
	std::optional<std::vector<TVertexType>> _extract_path(const TVertexType& _vertex, const TClosedList& _closedList)
	{
		if (auto curNode = _closedList.find(_vertex))
		{
			std::vector<TVertexType> path{ _vertex };
			while (curNode && curNode->parent)
			{
				path.emplace_back(*curNode->parent);
				curNode = _closedList.find(*curNode->parent);
			}
			std::reverse(std::begin(path), std::end(path));
			return path;
		}
		return std::nullopt;
	}

	template <class TVertexType, class UnaryFunction, class TNeighbourSearcher, class TPathFinder, class TOpenList, class TClosedList>
	std::optional<TVertexType> _conditional_visit(const TVertexType& _from, UnaryFunction&& _func, TNeighbourSearcher& _neighbourSearcher, TPathFinder& _pathFinder, TOpenList& _openList, TClosedList& _closedList)
	{
		using NodeData = typename TPathFinder::NodeDataType;
		assert(std::empty(_openList) && std::empty(_closedList));
		_openList.insert({ _from, NodeData() }, std::less<>());	// because the openList will be empty, we can insert a default constructed node and pass a dummy compare function
		while (auto current = _pathFinder(_neighbourSearcher, _openList, _closedList))
		{
			if (_func(*current))
				return current->vertex;
		}
		return std::nullopt;
	}

	template <class TVertexType, class TCallable, typename = std::enable_if_t<std::is_invocable_r_v<bool, TCallable, const TVertexType&>>>
	bool _reached_destination(const TVertexType& _current, TCallable&& _func)
	{
		return _func(_current);
	}

	template <class TVertexType>
	bool _reached_destination(const TVertexType& _current, const TVertexType& _destination)
	{
		return _current == _destination;
	}

	template <class TCostCalculator, class TVertexType, typename = std::enable_if_t<std::is_invocable_v<TCostCalculator, const TVertexType&, const TVertexType&>>>
	using CostCalculatorResult = std::invoke_result_t<TCostCalculator, const TVertexType&, const TVertexType&>;

	template <class TVertexType, class TNodeData, class TNodeCreator, class TNodeCompare>
	class GenericSearcher
	{
	public:
		using NodeDataType = TNodeData;
		using VertexType = TVertexType;
		using NodeType = Node<TVertexType, TNodeData>;

		explicit GenericSearcher(TNodeCreator _nodeCreator, TNodeCompare _nodeComp) :
			m_NodeCreator(std::move(_nodeCreator)),
			m_NodeComp(std::move(_nodeComp))
		{
		}

		template <class TNeighbourSearcher, class TOpenList, class TClosedList>
		std::optional<NodeType> operator ()(TNeighbourSearcher& _neighbourSearcher, TOpenList& _openList, TClosedList& _closedList)
		{
			if (std::empty(_openList))
				return std::nullopt;
			auto node = _openList.take_node(m_NodeComp);
			_closedList.insert(node, m_NodeComp);
			_neighbourSearcher(node,
				[&node, &_openList, &_closedList, &nodeComp = m_NodeComp, &nodeCreator = m_NodeCreator]
			(const auto& _vertex)
			{
				if (!_closedList.contains(_vertex))
				{
					_openList.insert({ _vertex, nodeCreator(node, _vertex) }, nodeComp);
				}
			}
			);
			return node;
		}

	private:
		TNodeCreator m_NodeCreator;
		TNodeCompare m_NodeComp;
	};

	template <class T>
	struct ConstantCost
	{
		const T cost;

		constexpr ConstantCost(T _cost = 0) :
			cost(_cost)
		{
		}

		template <class TVertexType>
		constexpr T operator ()(const TVertexType& _from, const TVertexType& _to) const noexcept
		{
			return cost;
		}
	};

	template <class TCostCalculator, class THeuristicCalculator>
	class GenericNodeCreator
	{
	public:
		template <class TVertexType>
		using CostCalculatorResult = detail::CostCalculatorResult<TCostCalculator, TVertexType>;

		GenericNodeCreator(TCostCalculator _costCalculator = TCostCalculator(), THeuristicCalculator _heuristicCalculator = THeuristicCalculator()) :
			m_CostCalculator(std::move(_costCalculator)),
			m_HeuristicCalculator(std::move(_heuristicCalculator))
		{
		}

		template <class TVertexType>
		auto operator ()(const Node<TVertexType, BaseNodeData<TVertexType, CostCalculatorResult<TVertexType>>>& _parentNode, const TVertexType& _childVertex)
		{
			return _create_child(_parentNode, _childVertex);
		}

		template <class TVertexType>
		auto operator ()(const Node<TVertexType, HeuristicNodeData<TVertexType, CostCalculatorResult<TVertexType>>>& _parentNode, const TVertexType& _childVertex)
		{
			auto childNode = _create_child(_parentNode, _childVertex);
			childNode.heuristic = m_HeuristicCalculator(_parentNode.vertex, _childVertex);
			return childNode;
		}

	private:
		TCostCalculator m_CostCalculator;
		THeuristicCalculator m_HeuristicCalculator;

		template <class TVertexType, class TNodeData>
		TNodeData _create_child(const Node<TVertexType, TNodeData>& _parentNode, const TVertexType& _childVertex)
		{
			auto childData(_parentNode.data);
			childData.parent = _parentNode.vertex;
			childData.cost += m_CostCalculator(_parentNode.vertex, _childVertex);
			return childData;
		}
	};

	template <class TCostType>
	using IncrementalCostChildNodeCreator = GenericNodeCreator<ConstantCost<TCostType>, ConstantCost<TCostType>>;

	template <class TVertexType, class TCostType, class TNodeCompare>
	struct StaticCostSearcher :
		public GenericSearcher<TVertexType, BaseNodeData<TVertexType, TCostType>, IncrementalCostChildNodeCreator<TCostType>, TNodeCompare>
	{
	private:
		using super = GenericSearcher<TVertexType, BaseNodeData<TVertexType, TCostType>, IncrementalCostChildNodeCreator<TCostType>, TNodeCompare>;

	public:
		StaticCostSearcher(TNodeCompare _nodeComp) :
			super({ { 1 }, { 0 } }, std::move(_nodeComp))
		{}
	};

	template <class TVertexType, class TCostCalculator, class THeuristicCalculator, class TNodeCompare, typename TCostType = detail::CostCalculatorResult<TCostCalculator, TVertexType>>
	struct CalculatedCostSearcher :
		public GenericSearcher<TVertexType, BaseNodeData<TVertexType, TCostType>, GenericNodeCreator<TCostCalculator, THeuristicCalculator>, TNodeCompare>
	{
	private:
		using super = GenericSearcher<TVertexType, BaseNodeData<TVertexType, TCostType>, GenericNodeCreator<TCostCalculator, THeuristicCalculator>, TNodeCompare>;

		static_assert(std::is_same_v<TCostType, detail::CostCalculatorResult<TCostCalculator, TVertexType>>,
			"You are not allowed to change the TCostType template parameter.");
		static_assert(std::is_same_v<detail::CostCalculatorResult<THeuristicCalculator, TVertexType>, detail::CostCalculatorResult<TCostCalculator, TVertexType>>,
			"TCostCalculator and THeuristicCalculator must return the same type.");

	public:
		CalculatedCostSearcher(TCostCalculator _costCalculator, THeuristicCalculator _heuristicCalculator, TNodeCompare _nodeComp) :
			super({ std::move(_costCalculator), std::move(_heuristicCalculator) }, std::move(_nodeComp))
		{
		}
	};
}

	template <class TMap>
	class NodeMap
	{
	public:
		using VertexType = typename TMap::key_type;
		using NodeDataType = typename TMap::mapped_type;
		using NodeType = Node<VertexType, NodeDataType>;

		explicit NodeMap(TMap _map = TMap()) :
			m_Nodes(std::move(_map))
		{
		}

		template <class TNodeCompare>
		void insert(NodeType _node, TNodeCompare&& _nodeComp)
		{
			if (auto result = m_Nodes.insert({ _node.vertex, _node.data });
				!result.second && _nodeComp(_node.data, result.first->second))
			{
				result.first->second = _node.data;
			}
		}

		template <class TNodeCompare>
		NodeType take_node(TNodeCompare&& _nodeComp)
		{
			auto itr = std::min_element(std::begin(m_Nodes), std::end(m_Nodes),
				[&_nodeComp](const auto& _lhs, const auto& _rhs) { return _nodeComp(_lhs.second, _rhs.second); }
			);
			assert(itr != std::end(m_Nodes));
			auto node = std::move(*itr);
			m_Nodes.erase(itr);
			return { node.first, node.second };
		}

		const NodeDataType* find(const VertexType& _key) const
		{
			auto itr = m_Nodes.find(_key);
			return std::end(m_Nodes) == itr ? nullptr : &itr->second;
		}

		bool contains(const VertexType& _key) const
		{
			return m_Nodes.count(_key) != 0;	// ToDo: use std::unordered_map::contains in C++20
		}

		bool empty() const
		{
			return std::empty(m_Nodes);
		}

	private:
		TMap m_Nodes;
	};

	template <class TPathFinder>
	using DefaultNodeMap = NodeMap<std::unordered_map<typename TPathFinder::VertexType, typename TPathFinder::NodeDataType>>;

	template <class TVertexType, class TNodeCompare = std::less<>>
	auto make_breadth_first_searcher(TNodeCompare _nodeComp = TNodeCompare())
	{
		return detail::StaticCostSearcher<TVertexType, int, TNodeCompare>(std::move(_nodeComp));
	}

	template <class TVertexType, class TNodeCompare = std::greater<>>
	auto make_depth_first_searcher(TNodeCompare _nodeComp = TNodeCompare())
	{
		return detail::StaticCostSearcher<TVertexType, int, TNodeCompare>(std::move(_nodeComp));
	}

	template <class TVertexType, class TCostCalculator, class TNodeCompare = std::less<>>
	auto make_dijkstra_searcher(TCostCalculator _costCalculator, TNodeCompare _nodeComp = TNodeCompare())
	{
		using CostType = detail::CostCalculatorResult<TCostCalculator, TVertexType>;
		return detail::CalculatedCostSearcher<TVertexType, TCostCalculator, detail::ConstantCost<CostType>, TNodeCompare>(std::move(_costCalculator), { 0 }, std::move(_nodeComp));
	}

	template <class TVertexType, class TCostCalculator, class THeuristicCalculator, class TNodeCompare = std::less<>>
	auto make_astar_searcher(TCostCalculator _costCalculator, THeuristicCalculator _heuristicCalculator, TNodeCompare _nodeComp = TNodeCompare())
	{
		return detail::CalculatedCostSearcher<TVertexType, TCostCalculator, THeuristicCalculator, TNodeCompare>(std::move(_costCalculator), std::move(_heuristicCalculator), std::move(_nodeComp));
	}

	template <class TVertexType, class UnaryFunction, class TNeighbourSearcher, class TPathFinder, class TOpenList = DefaultNodeMap<TPathFinder>, class TClosedList = DefaultNodeMap<TPathFinder>>
	void visit(const TVertexType& _from, UnaryFunction&& _func, TNeighbourSearcher&& _neighbourSearcher, TPathFinder&& _pathFinder, TOpenList&& _openList = TOpenList(), TClosedList&& _closedList = TClosedList())
	{
		auto condFunc = [&_func](const auto& _node)
		{
			_func(_node);
			return false;
		};
		detail::_conditional_visit(_from, condFunc, std::forward<TNeighbourSearcher>(_neighbourSearcher), std::forward<TPathFinder>(_pathFinder),
			std::forward<TOpenList>(_openList), std::forward<TClosedList>(_closedList));
	}

	template <class TVertexType, class TDestination, class TNeighbourSearcher, class TPathFinder, class TOpenList = DefaultNodeMap<TPathFinder>, class TClosedList = DefaultNodeMap<TPathFinder>>
	std::optional<std::vector<TVertexType>> find_path(const TVertexType& _from, TDestination&& _destination, TNeighbourSearcher&& _neighbourSearcher, TPathFinder&& _pathFinder,
		TOpenList&& _openList = TOpenList(), TClosedList&& _closedList = TClosedList())
	{
		if (auto lastNode = detail::_conditional_visit(_from,
			[&_destination](const auto& _node) { return detail::_reached_destination(_node.vertex, _destination); },
			std::forward<TNeighbourSearcher>(_neighbourSearcher), std::forward<TPathFinder>(_pathFinder),
			std::forward<TOpenList>(_openList), std::forward<TClosedList>(_closedList)))
		{
			return detail::_extract_path(*lastNode, _closedList);
		}
		return std::nullopt;
	}
}
}

#endif 
