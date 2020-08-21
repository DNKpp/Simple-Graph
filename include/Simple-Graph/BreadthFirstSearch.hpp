//          Copyright Dominic Koepke 2019 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SL_GRAPH_BFS_HPP
#define SL_GRAPH_BFS_HPP

#pragma once

#include "Simple-Graph/Utility.hpp"

#include <compare>
#include <deque>
#include <map>
#include <optional>

namespace sl::graph::detail::bfs
{
	template <class TVertex>
	struct NodeInfo
	{
		using Vertex_t = TVertex;

		std::optional<Vertex_t> parent;
		int depth = 0;
		NodeState state = NodeState::none;

		[[nodiscard]] constexpr bool operator ==(const NodeInfo&) const noexcept(detail::IsNothrowComparable_v<TVertex>) = default;

		[[nodiscard]] constexpr std::strong_ordering operator <=>(const NodeInfo& other) const noexcept
		{
			return depth <=> other.depth;
		}
	};

	template <class T, class TNodeInfo>
	concept NeighbourSearcherFor = requires { typename TNodeInfo::Vertex_t; } && NeighbourSearcherWith<T, const typename TNodeInfo::Vertex_t&, const TNodeInfo&>;

	template <class T, class TNodeInfo>
	concept StateMapFor = detail::StateMapWith<T, typename TNodeInfo::Vertex_t, TNodeInfo>;

	template <class T, class TNodeInfo>
	concept CallbackFor = requires { typename TNodeInfo::Vertex_t; } && std::invocable<T, typename TNodeInfo::Vertex_t, TNodeInfo>;
}

namespace sl::graph
{
	template <detail::Vertex TVertex>
	using BfsNodeInfo_t = detail::bfs::NodeInfo<TVertex>;
	template <detail::Vertex TVertex>
	using DefaultBfsStateMap_t = std::map<TVertex, BfsNodeInfo_t<TVertex>>;

	template <detail::Vertex TVertex,
		detail::bfs::NeighbourSearcherFor<BfsNodeInfo_t<TVertex>> TNeighbourSearcher,
		detail::bfs::StateMapFor<BfsNodeInfo_t<TVertex>> TStateMap = DefaultBfsStateMap_t<TVertex>,
		detail::bfs::CallbackFor<BfsNodeInfo_t<TVertex>> TCallback = EmptyCallback>
	void traverseBreadthFirstSearch(
		const TVertex& start,
		TNeighbourSearcher neighbourSearcher,
		TStateMap&& stateMap,
		TCallback callback = TCallback{}
	)
	{
		using NodeInfo_t = BfsNodeInfo_t<TVertex>;

		std::deque<TVertex> openList{ start };
		stateMap[start] = NodeInfo_t{ std::nullopt, 0, NodeState::open };
		while (!std::empty(openList))
		{
			auto vertex = openList.back();
			openList.pop_back();
			auto& nodeState = stateMap[vertex];
			nodeState.state = NodeState::closed;
			if (detail::shallReturn(callback, vertex, nodeState))
				return;
			neighbourSearcher(
							vertex,
							nodeState,
							[&stateMap, &openList, &parentVertex=std::as_const(vertex), &parentInfo=std::as_const(nodeState)](const TVertex& vertex)
							{
								if (auto& vertexState = stateMap[vertex]; vertexState.state == NodeState::none)
								{
									openList.emplace_front(vertex);
									vertexState.state = NodeState::open;
									vertexState.depth = parentInfo.depth + 1;
									vertexState.parent = parentVertex;
								}
							}
							);
		}
	}
}

#endif
