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

	template <class T>
	concept BooleanReference = std::is_convertible_v<T, bool> && std::equality_comparable_with<T, bool>;
	template <class T, class TVertex>
	concept StateMapFor = detail::StateMapWith<T, TVertex, NodeInfo<TVertex>> ||
	requires(std::remove_cvref_t<T>& stateMap)
	{
		{
			stateMap[std::declval<TVertex>()]
		} -> BooleanReference;
	};

	template <class T, class TNodeInfo>
	concept CallbackFor = requires { typename TNodeInfo::Vertex_t; } && std::invocable<T, typename TNodeInfo::Vertex_t, TNodeInfo>;

	template <class TStateMap, class TVertex, class TState>
	decltype(auto) setState(TStateMap& stateMap, const TVertex& vertex, const TState& state) noexcept
	{
		auto&& element = stateMap[vertex];
		if constexpr (std::is_assignable_v<decltype(element), bool>)
		{
			element = true;
		}
		else
		{
			return element = state;
		}
		return std::forward<decltype(element)>(element);
	}

	template <class TState>
	TState& setClosed(TState& state) noexcept
	{
		if constexpr (std::is_convertible_v<TState, bool>)
		{
			state = true;
		}
		else
		{
			state.state = NodeState::closed;
		}
		return state;
	}

	template <class TState>
	bool isUnVisited(const TState& state) noexcept
	{
		if constexpr (std::is_convertible_v<TState, bool>)
		{
			return !state;
		}
		else
		{
			return state.state == NodeState::none;
		}
	}
}

namespace sl::graph
{
	template <detail::Vertex TVertex>
	using DfsNodeInfo_t = detail::dfs::NodeInfo<TVertex>;
	template <detail::Vertex TVertex>
	using DefaultDfsStateMap_t = std::map<TVertex, bool>;
	template <detail::Vertex TVertex>
	using DfsNodeInfoStateMap_t = std::map<TVertex, DfsNodeInfo_t<TVertex>>;

	template <detail::Vertex TVertex,
		detail::dfs::NeighbourSearcherFor<DfsNodeInfo_t<TVertex>> TNeighbourSearcher,
		detail::dfs::StateMapFor<TVertex> TStateMap = DefaultDfsStateMap_t<TVertex>,
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
		if (detail::shallReturn(preCallback, start, detail::dfs::setState(stateMap, start, NodeInfo{ std::nullopt, depth, NodeState::open })))
			return;
		while (!std::empty(stack))
		{
			auto currentVertex = stack.top();
			auto&& nodeState = stateMap[currentVertex];
			if (auto child = neighbourSearcher(
												currentVertex,
												nodeState,
												[&stateMap](const TVertex& vertex)
												{
													return detail::dfs::isUnVisited(stateMap[vertex]);
												}
											))
			{
				++depth;
				auto&& childState{ detail::dfs::setState(stateMap, *child, NodeInfo{ std::nullopt, depth, NodeState::open }) };
				if (detail::shallReturn(preCallback, *child, childState))
					return;
				stack.push(*child);
			}
			else
			{
				detail::dfs::setClosed(nodeState);
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
