//          Copyright Dominic Koepke 2019 - 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SIMPLE_GRAPH_DEPTH_FIRST_SEARCH_HPP
#define SIMPLE_GRAPH_DEPTH_FIRST_SEARCH_HPP

#pragma once

#include "utility.hpp"

#include <map>

#include <ranges>
#include <stack>

namespace sl::graph
{
	template <
		vertex_descriptor TVertex,
		std::invocable<TVertex> TNeighborSearcher,
		std::invocable<weighted_node<TVertex, int>> TPreOrderFunc = empty_invokable,
		std::predicate<weighted_node<TVertex, int>> TNodePredicate = true_constant,
		state_map_for<TVertex, bool> TStateMap = std::map<TVertex, bool>>
		requires std::ranges::input_range<std::invoke_result_t<TNeighborSearcher, TVertex>>
	void traverse_dfs
	(
		TVertex begin,
		TNeighborSearcher neighborSearcher,
		TPreOrderFunc preOrderFunc = {},
		TNodePredicate nodePredicate = {},
		TStateMap stateMap = {}
	)
	{
		std::stack<weighted_node<TVertex, int>> openNodes{};
		openNodes.emplace(std::nullopt, std::move(begin), 0);

		while (!std::empty(openNodes))
		{
			auto node{ std::move(openNodes.top()) };
			openNodes.pop();

			if (detail::shall_interrupt(preOrderFunc, node))
				return;

			for
			(
				const TVertex& current
				: std::invoke(neighborSearcher, node.vertex)
				| std::views::filter([&](const TVertex& v) { return v != node.predecessor; })
				| std::views::filter([&stateMap](const TVertex& v) { return !std::exchange(stateMap[v], true); })
				| std::views::filter(nodePredicate)
			)
			{
				openNodes.emplace(node.vertex, current, node.weight_sum + 1);
			}
		}
	}
}

#endif
