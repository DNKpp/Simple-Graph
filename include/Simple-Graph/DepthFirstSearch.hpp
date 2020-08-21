//          Copyright Dominic Koepke 2019 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SL_GRAPH_DFS_HPP
#define SL_GRAPH_DFS_HPP

#pragma once

#include "Simple-Graph/Utility.hpp"

#include <cassert>
#include <compare>
#include <map>
#include <optional>
#include <stack>

namespace sl::graph::detail::dfs
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
	concept NeighbourSearcherFor = requires { typename TNodeInfo::Vertex_t; } && std::is_invocable_r_v<
		std::optional<typename TNodeInfo::Vertex_t>, T, const typename TNodeInfo::Vertex_t&, const TNodeInfo&, DummyCallable<typename TNodeInfo::Vertex_t>>;

	template <class T, class TNodeInfo>
	concept StateMapFor = detail::StateMapWith<T, typename TNodeInfo::Vertex_t, TNodeInfo>;

	template <class T, class TNodeInfo>
	concept CallbackFor = requires { typename TNodeInfo::Vertex_t; } && std::invocable<T, typename TNodeInfo::Vertex_t, TNodeInfo>;
}

namespace sl::graph
{
	template <detail::Vertex TVertex>
	using DfsNodeInfo_t = detail::dfs::NodeInfo<TVertex>;
	template <detail::Vertex TVertex>
	using DefaultDfsStateMap_t = std::map<TVertex, DfsNodeInfo_t<TVertex>>;

	template <detail::Vertex TVertex,
		detail::dfs::NeighbourSearcherFor<DfsNodeInfo_t<TVertex>> TNeighbourSearcher,
		detail::dfs::StateMapFor<DfsNodeInfo_t<TVertex>> TStateMap = DefaultDfsStateMap_t<TVertex>,
		detail::dfs::CallbackFor<DfsNodeInfo_t<TVertex>> TPreOrderCallback = EmptyCallback,
		detail::dfs::CallbackFor<DfsNodeInfo_t<TVertex>> TPostOrderCallback = EmptyCallback>
	void traverseDepthFirstSearchIterative(
		const TVertex& start,
		TNeighbourSearcher neighbourSearcher,
		TStateMap&& stateMap,
		TPreOrderCallback preCallback = TPreOrderCallback{},
		TPostOrderCallback postCallback = TPostOrderCallback{}
	)
	{
		using NodeInfo = detail::dfs::NodeInfo<TVertex>;

		std::stack<TVertex> stack;
		stack.push(start);
		int depth = 0;
		if (detail::shallReturn(preCallback, start, stateMap[start] = NodeInfo{ std::nullopt, depth, NodeState::open }))
			return;
		while (!std::empty(stack))
		{
			auto currentVertex = stack.top();
			auto& nodeState = stateMap[currentVertex];
			if (auto child = neighbourSearcher(
												currentVertex,
												nodeState,
												[&stateMap](const TVertex& vertex)
												{
													return stateMap[vertex].state == NodeState::none;
												}
											))
			{
				++depth;
				auto& childState{ stateMap[*child] = NodeInfo{ std::nullopt, depth, NodeState::open } };
				if (detail::shallReturn(preCallback, *child, childState))
					return;
				stack.push(*child);
			}
			else
			{
				nodeState.state = NodeState::closed;
				if (detail::shallReturn(postCallback, currentVertex, nodeState))
					return;
				--depth;
				stack.pop();
			}
		}
		assert(depth == -1);
	}
}

#endif
