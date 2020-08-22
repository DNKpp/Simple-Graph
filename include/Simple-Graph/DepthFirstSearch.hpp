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
#include <vector>

namespace sl::graph::detail::dfs
{
	template <class TVertex>
	struct NodeInfo
	{
		using Vertex_t = TVertex;

		std::optional<Vertex_t> parent;
		int depth = 0;

		[[nodiscard]] constexpr bool operator ==(const NodeInfo&) const noexcept(detail::IsNothrowComparable_v<TVertex>) = default;

		[[nodiscard]] constexpr std::strong_ordering operator <=>(const NodeInfo& other) const noexcept
		{
			return depth <=> other.depth;
		}
	};

	template <class T, class TNodeInfo>
	concept IterativeNeighbourSearcherFor = requires { typename TNodeInfo::Vertex_t; } && std::is_invocable_r_v<
		std::optional<typename TNodeInfo::Vertex_t>, T, const typename TNodeInfo::Vertex_t&, const TNodeInfo&, DummyCallable<typename TNodeInfo::Vertex_t>>;

	template <class T, class TNodeInfo>
	concept NeighbourSearcherFor = requires { typename TNodeInfo::Vertex_t; } && NeighbourSearcherWith<T, const typename TNodeInfo::Vertex_t&, const TNodeInfo&>;

	template <class T>
	concept BooleanReference = std::same_as<T, std::vector<bool>::reference> ||
	std::is_convertible_v<T, bool> && std::equality_comparable_with<T, bool> && std::is_reference_v<T>;

	template <class T, class TVertex>
	concept VisitationTrackerFor = requires(std::remove_cvref_t<T>& tracker)
	{
		{
			tracker[std::declval<TVertex>()]
		} -> BooleanReference;
	};

	template <class T, class TNodeInfo>
	concept CallbackFor = requires { typename TNodeInfo::Vertex_t; } && std::invocable<T, typename TNodeInfo::Vertex_t, TNodeInfo>;

	template <class TVertex, class TNeighbourSearcher, class TVisitationTracker, class TPreOrderCallback, class TPostOrderCallback>
	bool traverseRecursive(
		const TVertex& current,
		const NodeInfo<TVertex>& currentNodeInfo,
		const TNeighbourSearcher& neighbourSearcher,
		TVisitationTracker& visitationTracker,
		TPreOrderCallback& preCallback,
		TPostOrderCallback& postCallback
	)
	{
		using NodeInfo_t = NodeInfo<TVertex>;

		auto&& currentVisitState = visitationTracker[current];
		if (currentVisitState)
			return false;

		currentVisitState = true;
		if (shallReturn(preCallback, current, currentNodeInfo))
		{
			return true;
		}

		bool interrupt = false;
		neighbourSearcher(
						current,
						currentNodeInfo,
						[&](const TVertex& vertex)
						{
							if (interrupt)
								return;

							interrupt = traverseRecursive(
														vertex,
														{ vertex, currentNodeInfo.depth + 1 },
														neighbourSearcher,
														visitationTracker,
														preCallback,
														postCallback
														);
						}
						);

		if (interrupt || shallReturn(postCallback, current, currentNodeInfo))
		{
			return true;
		}

		return false;
	}
}

namespace sl::graph
{
	template <detail::Vertex TVertex>
	using DfsNodeInfo_t = detail::dfs::NodeInfo<TVertex>;
	template <detail::Vertex TVertex>
	using DefaultDfsVisitationTracker_t = std::map<TVertex, bool>;

	template <detail::Vertex TVertex,
		detail::dfs::NeighbourSearcherFor<DfsNodeInfo_t<TVertex>> TNeighbourSearcher,
		detail::dfs::VisitationTrackerFor<TVertex> TVisitationTracker = DefaultDfsVisitationTracker_t<TVertex>,
		detail::dfs::CallbackFor<DfsNodeInfo_t<TVertex>> TPreOrderCallback = EmptyCallback,
		detail::dfs::CallbackFor<DfsNodeInfo_t<TVertex>> TPostOrderCallback = EmptyCallback>
	void traverseDepthFirstSearch(
		const TVertex& start,
		const TNeighbourSearcher& neighbourSearcher,
		TVisitationTracker& visitationTracker = TVisitationTracker{},
		TPreOrderCallback preCallback = TPreOrderCallback{},
		TPostOrderCallback postCallback = TPostOrderCallback{}
	)
	{
		detail::dfs::traverseRecursive(start, { std::nullopt, 0 }, neighbourSearcher, visitationTracker, preCallback, postCallback);
	}

	template <detail::Vertex TVertex,
		detail::dfs::IterativeNeighbourSearcherFor<DfsNodeInfo_t<TVertex>> TNeighbourSearcher,
		detail::dfs::VisitationTrackerFor<TVertex> TVisitationTracker = DefaultDfsVisitationTracker_t<TVertex>,
		detail::dfs::CallbackFor<DfsNodeInfo_t<TVertex>> TPreOrderCallback = EmptyCallback,
		detail::dfs::CallbackFor<DfsNodeInfo_t<TVertex>> TPostOrderCallback = EmptyCallback>
	void traverseDepthFirstSearchIterative(
		const TVertex& start,
		const TNeighbourSearcher& neighbourSearcher,
		TVisitationTracker& visitationTracker = TVisitationTracker{},
		TPreOrderCallback preCallback = TPreOrderCallback{},
		TPostOrderCallback postCallback = TPostOrderCallback{}
	)
	{
		using NodeInfo = detail::dfs::NodeInfo<TVertex>;
		using StackNode = std::pair<TVertex, std::optional<TVertex>>;

		visitationTracker[start] = true;
		if (detail::shallReturn(preCallback, start, NodeInfo{ std::nullopt, 0 }))
			return;
		
		std::stack<StackNode> stack;
		stack.emplace(start, std::nullopt);
		while (!std::empty(stack))
		{
			auto& currentNode = stack.top();
			if (auto child = neighbourSearcher(
												currentNode.first,
												NodeInfo{ currentNode.second, static_cast<int>(std::size(stack) - 1) },
												[&visitationTracker](const TVertex& vertex)
												{
													// might appear silly, but we need this kind of trick when using std::vector<bool>
													auto&& visited = visitationTracker[vertex];
													const bool old = visited;
													visited = true;
													return !old;
												}
											))
			{
				if (detail::shallReturn(preCallback, *child, NodeInfo{ currentNode.first, static_cast<int>(std::size(stack)) }))
					return;
				stack.emplace(*child, currentNode.first);
			}
			else
			{
				if (detail::shallReturn(postCallback, currentNode.first, NodeInfo{ currentNode.second, static_cast<int>(std::size(stack) -1) }))
					return;
				stack.pop();
			}
		}
		assert(std::empty(stack));
	}
}

#endif
