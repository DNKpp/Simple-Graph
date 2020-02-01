#ifndef SL_GRAPH_ALGORITHM_HPP
#define SL_GRAPH_ALGORITHM_HPP

#pragma once

#include <stack>
#include <vector>
#include <algorithm>
#include <cassert>
#include <deque>
#include <queue>
#include <optional>

namespace sl::graph
{
	template <class TVertex>
	struct DFSNode
	{
		int depth;
		TVertex vertex;
	};

	template <class TVertex>
	using BFSNode = DFSNode<TVertex>;

	template <class TVertex, class TWeight>
	struct DijkstraNode
	{
	    using Vertex_t = TVertex;
	    using Weight_t = TWeight;
	    
	    Vertex_t vertex;
	    std::optional<Vertex_t> parent;
	    
	    Weight_t weight_sum;
	    
	    bool operator >(const DijkstraNode& _other) const
	    {
	        return weight_sum > _other.weight_sum;
	    }
	};
	
	struct EmptyCallback
	{
	    template <class... Args>
	    constexpr void operator ()(Args&&...) const noexcept {}  
	};

namespace _detail
{
	template <class TFunction, class... TParams>
	bool shall_return(TFunction& _func, TParams... _params)
	{
		if constexpr (std::is_same_v<bool, std::invoke_result_t<decltype(_func), TParams...>>)
		{
			return _func(std::forward<TParams>(_params)...);
		}
		else
		{
			_func(std::forward<TParams>(_params)...);
			return false;
		}
	}
}

	template <class TVertex, class TNeighbourSearcher, class TVisitationTracker, class TPreOrderCallback = EmptyCallback, class TPostOrderCallback = EmptyCallback>
	void traverse_dfs(const TVertex& _begin, TNeighbourSearcher _neighbourSearcher, TVisitationTracker _visitationTracker,
	    TPreOrderCallback _preCB = TPreOrderCallback{}, TPostOrderCallback _postCB = TPostOrderCallback{})
	{
		using Node = DFSNode<TVertex>;
	
	    std::stack<TVertex> stack;
	    _visitationTracker[_begin] = true;
	    stack.push(_begin);
	    int depth = 0;
		if (_detail::shall_return(_preCB, Node{ depth, _begin }))
			return;
	    while (!std::empty(stack))
	    {
	        auto v = stack.top();
	        if (auto child = _neighbourSearcher(Node{ depth, v },
	            [&_visitationTracker](const TVertex& _vertex)
	            {
	                return !_visitationTracker[_vertex];
	            }
	        ))
	        {
	        	++depth;
	        	if (_detail::shall_return(_preCB, Node{ depth, *child }))
					return;
	            _visitationTracker[*child] = true;
	            stack.push(*child);
	        }
	        else
	        {
	            if (_detail::shall_return(_postCB, Node{ depth, v }))
					return;
	            --depth;
	            stack.pop();
	        }
	    }
	    assert(depth == -1);
	}

	template <class TVertex, class TNeighbourSearcher, class TVisitationTracker, class TCallback = EmptyCallback>
	void traverse_bfs(const TVertex& _begin, TNeighbourSearcher _neighbourSearcher, TVisitationTracker _visitationTracker,
	    TCallback _callback = TCallback{})
	{
	    using Node = BFSNode<TVertex>;
	    
	    std::deque<Node> openList{ { 0, _begin } };
	    _visitationTracker[_begin] = true;
	    while (!std::empty(openList))
	    {
	        auto node = openList.back();
	        openList.pop_back();
	        if (_detail::shall_return(_callback, node))
				return;
	        _neighbourSearcher(node,
	            [&_visitationTracker, &openList, depth = node.depth + 1](const TVertex& _vertex)
	            {
	        		auto&& check = _visitationTracker[_vertex];
	                if (!check)
	                {
	                    openList.emplace_front(Node{ depth, _vertex });
	                    check = true;
	                }
	            }
	        );
	    }
	}

namespace _detail
{
	template <class TNode, class TNeighbourSearcher, class TVisitationTracker, class TNodeWeight, class TEdgeWeight, class TCallback>
	void traverse_generic(TNode _begin, TNeighbourSearcher _neighbourSearcher, TVisitationTracker _visitationTracker,
		TNodeWeight _nodeWeight, TEdgeWeight _edgeWeight, TCallback _callback)
	{
	    using Vertex_t = typename TNode::Vertex_t;
	    
	    std::priority_queue<TNode, std::vector<TNode>, std::greater<>> openList;
	    openList.emplace(_begin);
	    while (!std::empty(openList))
	    {
	        auto node = openList.top();
	        openList.pop();
	        if (auto&& track = _visitationTracker[node.vertex]; !track)
	        {
	            track = true;
	            if (_detail::shall_return(_callback, node))
					return;
	            _neighbourSearcher(node,
	                [&openList, &node, &_nodeWeight, &_edgeWeight](const Vertex_t& _vertex)
	                {
	                    auto nodeWeight = _nodeWeight(_vertex);
	                    auto edgeWeight = _edgeWeight(node.vertex, _vertex);
	                    openList.emplace(TNode{ _vertex, node.vertex, node.weight_sum + nodeWeight + edgeWeight });
	                }
	            );
	        }
	    }
	}
}

	template <auto WEIGHT>
	struct ConstWeight
	{
	    template <class... TParams>
	    constexpr decltype(WEIGHT) operator()(TParams&&...) const noexcept
	    {
	        return WEIGHT;
	    }
	};
	
	template <class TVertex, class TNeighbourSearcher, class TVisitationTracker,
		class TNodeWeight = ConstWeight<1>, class TEdgeWeight = ConstWeight<0>, class TCallback = EmptyCallback>
	void traverse_dijkstra(const TVertex& _begin, TNeighbourSearcher _neighbourSearcher, TVisitationTracker _visitationTracker, 
		TNodeWeight _nodeWeight = TEdgeWeight{}, TEdgeWeight _edgeWeight = TEdgeWeight{}, TCallback _callback = TCallback{})
	{
		using Weight_t = std::common_type_t<std::invoke_result_t<TNodeWeight>, std::invoke_result_t<TEdgeWeight>>;
		static_assert(!std::is_same_v<void, Weight_t>);
		using Node_t = DijkstraNode<TVertex, Weight_t>;
	    _detail::traverse_generic(Node_t{ _begin, std::nullopt, {} }, _neighbourSearcher, _visitationTracker, _nodeWeight, _edgeWeight, _callback);
	}
}
#endif 
