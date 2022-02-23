//          Copyright Dominic Koepke 2019 - 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SIMPLE_GRAPH_DEPTH_FIRST_SEARCH_HPP
#define SIMPLE_GRAPH_DEPTH_FIRST_SEARCH_HPP

#pragma once

#include "generic_traverse.hpp"
#include "utility.hpp"

#include <map>
#include <stack>

namespace sl::graph
{
	template <class T, class TContainer>
	struct detail::take_next_t<std::stack<T, TContainer>>
	{
		constexpr T operator ()(std::stack<T, TContainer>& container) const
		{
			auto el{ std::move(container.top()) };
			container.pop();
			return el;
		}
	};

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
		TPreOrderFunc callback = {},
		TNodePredicate nodePredicate = {},
		TStateMap stateMap = {}
	)
	{
		detail::uniform_cost_traverse<weighted_node<TVertex, int>>
		(
			std::move(begin),
			std::move(neighborSearcher),
			std::move(callback),
			std::move(nodePredicate),
			std::stack<weighted_node<TVertex, int>>{},
			std::move(stateMap)
		);
	}
}

#endif
