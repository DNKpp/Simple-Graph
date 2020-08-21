//          Copyright Dominic Koepke 2019 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SL_GRAPH_ASTAR_HPP
#define SL_GRAPH_ASTAR_HPP

#pragma once

#include "Simple-Graph/UniformCostSearcher.hpp"
#include "Simple-Graph/Utility.hpp"

#include <compare>
#include <map>
#include <optional>
#include <queue>
#include <vector>

namespace sl::graph::detail::astar
{
	template <class TVertex, class TWeight>
	struct NodeInfo
	{
		using Vertex_t = TVertex;
		using Weight_t = TWeight;

		std::optional<Vertex_t> parent;
		Weight_t weightSum{};
		Weight_t heuristic{};
		NodeState state = NodeState::none;

		[[nodiscard]] constexpr bool operator ==(
			const NodeInfo&
		) const noexcept(detail::IsNothrowComparable_v<TVertex> && detail::IsNothrowComparable_v<TWeight>) = default;

		[[nodiscard]] constexpr std::strong_ordering operator <=>(const NodeInfo& other) const noexcept
		{
			return weightSum + heuristic <=> other.weightSum + other.heuristic;
		}
	};

	template <class TVertex, class TWeight>
	struct OpenNode
	{
		TVertex vertex;
		TWeight weightSum;
		TWeight heuristic;

		OpenNode(
			const TVertex& vertex_,
			const NodeInfo<TVertex, TWeight>& info
		) noexcept(std::is_nothrow_copy_constructible_v<TVertex> && std::is_nothrow_copy_constructible_v<TWeight>) :
			vertex{ vertex_ },
			weightSum{ info.weightSum },
			heuristic{ info.heuristic }
		{
		}

		OpenNode(
			const TVertex& vertex_,
			TWeight weightSum_,
			TWeight heuristic_
		) noexcept(std::is_nothrow_copy_constructible_v<TVertex> && std::is_nothrow_copy_constructible_v<TWeight>) :
			vertex{ vertex_ },
			weightSum{ weightSum_ },
			heuristic{ heuristic_ }
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

		[[nodiscard]] bool operator ==(const OpenNode&) const noexcept(IsNothrowComparable_v<TVertex> && IsNothrowComparable_v<TWeight>) = default;

		[[nodiscard]] std::strong_ordering operator <=>(const OpenNode& other) const noexcept
		{
			return weightSum + heuristic <=> other.weightSum + other.heuristic;
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
	concept PropertyMapWith = ucs::PropertyMapWith<T, TVertex> &&
	requires(const std::remove_cvref_t<T>& propertyMap)
	{
		std::same_as<TVertex, typename PropertyMapTraits<T>::VertexType>;
		{
			propertyMap.heuristic(std::declval<TVertex>(), std::declval<TVertex>())
		} -> std::same_as<typename PropertyMapTraits<T>::WeightType>;
	};

	template <class T, class TPropertyMap>
	concept NeighbourSearcherFor = NeighbourSearcherWith<T, typename PropertyMapTraits<TPropertyMap>::VertexType, typename PropertyMapTraits<TPropertyMap
														>::NodeInfoType>;

	template <class T, class TPropertyMap>
	concept StateMapFor = StateMapWith<T, typename PropertyMapTraits<TPropertyMap>::VertexType, typename PropertyMapTraits<TPropertyMap>::NodeInfoType>;

	template <class T, class TPropertyMap>
	concept CallbackFor = std::invocable<T, typename PropertyMapTraits<TPropertyMap>::VertexType, typename PropertyMapTraits<TPropertyMap>::NodeInfoType>;
}

namespace sl::graph
{
	template <detail::Vertex TVertex, std::regular TWeight>
	using AStarNodeInfo_t = detail::astar::NodeInfo<TVertex, TWeight>;
	template <detail::Vertex TVertex, std::regular TWeight>
	using DefaultAStarStateMap_t = std::map<TVertex, AStarNodeInfo_t<TVertex, TWeight>>;

	template <detail::Vertex TVertex,
		detail::astar::PropertyMapWith<TVertex> TPropertyMap,
		detail::astar::NeighbourSearcherFor<TPropertyMap> TNeighbourSearcher,
		detail::astar::StateMapFor<TPropertyMap> TStateMap = DefaultAStarStateMap_t<
			TVertex, typename detail::astar::PropertyMapTraits<TPropertyMap>::WeightType>,
		detail::astar::CallbackFor<TPropertyMap> TCallback = EmptyCallback>
	void traverseAStar(
		const TVertex& start,
		const TVertex& destination,
		const TPropertyMap& propertyMap,
		const TNeighbourSearcher& neighbourSearcher,
		TStateMap&& stateMap = TStateMap{},
		TCallback callback = TCallback{}
	)
	{
		using Weight_t = typename detail::astar::PropertyMapTraits<TPropertyMap>::WeightType;
		using NodeInfo_t = typename detail::astar::PropertyMapTraits<TPropertyMap>::NodeInfoType;
		using OpenNode_t = detail::astar::OpenNode<TVertex, Weight_t>;

		std::priority_queue<OpenNode_t, std::vector<OpenNode_t>, std::greater<>> openList;
		openList.emplace(start, stateMap[start] = { std::nullopt, 0, propertyMap.heuristic(start, destination), NodeState::open });
		detail::traverse(
						[&destination, &propertyMap](const TVertex& vertex, const OpenNode_t& parent) -> NodeInfo_t
						{
							auto weightSum = parent.weightSum + propertyMap.edgeWeight(parent.vertex, vertex) + propertyMap.nodeWeight(vertex);
							return { parent.vertex, weightSum, propertyMap.heuristic(vertex, destination), NodeState::open };
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
