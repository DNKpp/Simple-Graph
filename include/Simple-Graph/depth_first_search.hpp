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
		state_map_for<TVertex, bool> TStateMap = std::map<TVertex, bool>>
		requires std::ranges::input_range<std::invoke_result_t<TNeighborSearcher, TVertex>>
	void traverse_dfs(TVertex begin, TNeighborSearcher neighborSearcher, TPreOrderFunc preOrderFunc = {}, TStateMap stateMap = {})
	{
		std::stack<weighted_node<TVertex, int>> openNodes{};
		openNodes.emplace(std::nullopt, begin, 0);

		while (!std::empty(openNodes))
		{
			auto node = openNodes.top();
			openNodes.pop();

			if (detail::shall_interrupt(preOrderFunc, node))
				return;

			for
			(
				const TVertex& current
				: std::invoke(neighborSearcher, node.vertex)
				| std::views::filter([&](const TVertex& v) { return v != node.predecessor; })
				| std::views::filter([&stateMap](const TVertex& v) { return !std::exchange(stateMap[v], true); })
			)
			{
				openNodes.emplace(node.vertex, current, node.weight_sum + 1);
			}
		}
	}
}

#endif
