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

template <class T, class TContainer>
struct sl::graph::detail::take_next_func_t<std::stack<T, TContainer>>
{
	constexpr T operator ()(std::stack<T, TContainer>& container) const
	{
		auto el{ std::move(container.top()) };
		container.pop();
		return el;
	}
};

namespace sl::graph::dfs
{
	template <vertex_descriptor TVertex>
	using node_t = weighted_node<TVertex, int>;

	template <
		vertex_descriptor TVertex,
		neighbor_searcher_for<TVertex> TNeighborSearcher,
		std::invocable<node_t<TVertex>> TPreOrderFunc = empty_invokable_t,
		std::predicate<node_t<TVertex>, TVertex> TVertexPredicate = true_constant_t,
		state_map_for<TVertex, bool> TStateMap = std::map<TVertex, bool>>
	void traverse
	(
		TVertex begin,
		TNeighborSearcher neighborSearcher,
		TPreOrderFunc callback = {},
		TVertexPredicate vertexPredicate = {},
		TStateMap stateMap = {}
	)
	{
		detail::uniform_cost_traverse<weighted_node<TVertex, int>>
		(
			std::move(begin),
			std::move(neighborSearcher),
			std::move(callback),
			std::move(vertexPredicate),
			std::move(stateMap),
			std::stack<weighted_node<TVertex, int>>{}
		);
	}
}

#endif
