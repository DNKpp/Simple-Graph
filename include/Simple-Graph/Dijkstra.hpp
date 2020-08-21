//          Copyright Dominic Koepke 2019 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SL_GRAPH_DIJKSTRA_HPP
#define SL_GRAPH_DIJKSTRA_HPP

#pragma once

#include "Simple-Graph/Utility.hpp"

#include <cassert>
#include <compare>
#include <map>
#include <optional>
#include <queue>
#include <vector>

namespace sl::graph::detail::dijkstra
{
	template <class TVertex, class TWeight>
	struct NodeInfo
	{
		using Vertex_t = TVertex;
		using Weight_t = TWeight;

		std::optional<Vertex_t> parent;
		Weight_t weightSum{};
		NodeState state = NodeState::none;

		[[nodiscard]] constexpr bool operator ==(
			const NodeInfo&
		) const noexcept(detail::IsNothrowComparable_v<TVertex> && detail::IsNothrowComparable_v<TWeight>) = default;

		[[nodiscard]] constexpr std::strong_ordering operator <=>(const NodeInfo& other) const noexcept
		{
			return weightSum <=> other.weightSum;
		}
	};

	template <class TVertex, class TWeight>
	struct OpenNode
	{
		TVertex vertex;
		TWeight weightSum;

		OpenNode(
			const TVertex& vertex_,
			const NodeInfo<TVertex, TWeight>& info
		) noexcept(std::is_nothrow_copy_constructible_v<TVertex> && std::is_nothrow_copy_constructible_v<TWeight>) :
			vertex{ vertex_ },
			weightSum{ info.weightSum }
		{
		}

		OpenNode(
			const TVertex& vertex_,
			TWeight weightSum_
		) noexcept(std::is_nothrow_copy_constructible_v<TVertex> && std::is_nothrow_copy_constructible_v<TWeight>) :
			vertex{ vertex_ },
			weightSum{ weightSum_ }
		{
		}

		~OpenNode() noexcept = default;

		[[nodiscard]] OpenNode(
			const OpenNode&
		) noexcept(std::is_nothrow_copy_constructible_v<TVertex> && std::is_nothrow_copy_constructible_v<TWeight>) = default;
		[[nodiscard]] OpenNode& operator =(
			const OpenNode&
		) noexcept(std::is_nothrow_copy_assignable_v<TVertex> && std::is_nothrow_copy_assignable_v<TWeight>) = default;
		[[nodiscard]] OpenNode(OpenNode&&) noexcept(std::is_nothrow_move_constructible_v<TVertex> && std::is_nothrow_move_constructible_v<TWeight>) = default;
		[[nodiscard]] OpenNode& operator =(OpenNode&&) noexcept(std::is_nothrow_move_assignable_v<TVertex> && std::is_nothrow_move_assignable_v<TWeight>)
		= default;

		[[nodiscard]] bool operator ==(const OpenNode&) const noexcept(IsNothrowComparable_v<TVertex> && IsNothrowComparable_v<TWeight>)
		= default;

		[[nodiscard]] auto operator <=>(const OpenNode& other) const noexcept
		{
			return weightSum <=> other.weightSum;
		}
	};

	template <class TPropertyMap>
	struct PropertyMapTraits
	{
		using VertexType = typename TPropertyMap::VertexType;
		using WeightType = typename TPropertyMap::WeightType;
		using NodeInfoType = NodeInfo<VertexType, WeightType>;
	};

	template <class T, class TVertex>
	concept PropertyMapWith = requires(const std::remove_cvref_t<T>& propertyMap)
	{
		typename PropertyMapTraits<T>::VertexType;
		typename PropertyMapTraits<T>::WeightType;
		{
			propertyMap.edgeWeight(std::declval<TVertex>(), std::declval<TVertex>())
		} -> std::same_as<typename PropertyMapTraits<T>::WeightType>;
		{
			propertyMap.nodeWeight(std::declval<TVertex>())
		} -> std::same_as<typename PropertyMapTraits<T>::WeightType>;
	};

	template <class T, class TPropertyMap>
	concept NeighbourSearcherFor = NeighbourSearcherWith<T, typename PropertyMapTraits<TPropertyMap>::VertexType, typename PropertyMapTraits<TPropertyMap>::NodeInfoType>;

	template <class T, class TPropertyMap>
	concept StateMapFor = StateMapWith<T, typename PropertyMapTraits<TPropertyMap>::VertexType, typename PropertyMapTraits<TPropertyMap>::NodeInfoType>;

	template <class T, class TPropertyMap>
	concept CallbackFor = std::invocable<T, typename PropertyMapTraits<TPropertyMap>::VertexType, typename PropertyMapTraits<TPropertyMap>::NodeInfoType>;
}

namespace sl::graph::detail
{
	template <class TNodeCreator, class TNeighbourSearcher, class TStateMap, class TOpenList, class TCallback>
	void traverse(TNodeCreator nodeCreator, const TNeighbourSearcher& neighbourSearcher, TStateMap&& stateMap, TOpenList openList, TCallback callback)
	{
		assert(!empty(openList));
		while (!empty(openList))
		{
			auto node = openList.top();
			openList.pop();
			auto& nodeInfo = stateMap[node.vertex];
			assert(nodeInfo.state != NodeState::none);
			if (std::exchange(nodeInfo.state, NodeState::closed) == NodeState::closed) [[unlikely]]
			{
				continue;
			}

			if (callback(node.vertex, nodeInfo))
				return;

			auto neighbourCallback = [&parent=std::as_const(node), &nodeCreator, &openList, &stateMap](const auto& vertex)
			{
				auto& cachedState = stateMap[vertex];
				if (cachedState.state == NodeState::closed)
					return;

				auto currentNode = nodeCreator(vertex, parent);
				assert(parent.weightSum < currentNode.weightSum && "The algorithm is not desined to work with negative weights.");
				if (cachedState.state == NodeState::none || currentNode < cachedState)
				{
					cachedState = currentNode;
					openList.emplace(vertex, currentNode);
				}
			};
			neighbourSearcher(node.vertex, nodeInfo, neighbourCallback);
		}
	}
}

namespace sl::graph
{
	template <detail::Vertex TVertex, std::regular TWeight>
	using DijkstraNodeInfo_t = detail::dijkstra::NodeInfo<TVertex, TWeight>;
	template <detail::Vertex TVertex, std::regular TWeight>
	using DefaultDijkstraStateMap_t = std::map<TVertex, DijkstraNodeInfo_t<TVertex, TWeight>>;

	template <detail::Vertex TVertex,
		detail::dijkstra::PropertyMapWith<TVertex> TPropertyMap,
		detail::dijkstra::NeighbourSearcherFor<TPropertyMap> TNeighbourSearcher,
		detail::dijkstra::StateMapFor<TPropertyMap> TStateMap = DefaultDijkstraStateMap_t<TVertex, typename detail::dijkstra::PropertyMapTraits<TPropertyMap>::WeightType>,
		detail::dijkstra::CallbackFor<TPropertyMap> TCallback = EmptyCallback>
	void traverseDijkstra(
		const TVertex& start,
		const TVertex& destination,
		const TPropertyMap& propertyMap,
		const TNeighbourSearcher& neighbourSearcher,
		TStateMap&& stateMap = TStateMap{},
		TCallback callback = TCallback{}
	)
	{
		using Weight_t = typename detail::dijkstra::PropertyMapTraits<TPropertyMap>::WeightType;
		using NodeInfo_t = typename detail::dijkstra::PropertyMapTraits<TPropertyMap>::NodeInfoType;
		using OpenNode_t = detail::dijkstra::OpenNode<TVertex, Weight_t>;

		std::priority_queue<OpenNode_t, std::vector<OpenNode_t>, std::greater<>> openList;
		openList.emplace(start, stateMap[start] = { std::nullopt, 0, NodeState::open });
		detail::traverse(
						[&propertyMap](const TVertex& vertex, const OpenNode_t& parent) -> NodeInfo_t
						{
							auto weightSum = parent.weightSum + propertyMap.edgeWeight(parent.vertex, vertex) + propertyMap.nodeWeight(vertex);
							return { parent.vertex, weightSum, NodeState::open };
						},
						neighbourSearcher,
						std::forward<TStateMap>(stateMap),
						openList,
						[&callback, &destination](const TVertex& vertex, const auto& nodeInfo)
						{
							return detail::shallReturn(callback, vertex, nodeInfo) || vertex == destination;
						}
						);
	}
}

#endif
