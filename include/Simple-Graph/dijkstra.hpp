//          Copyright Dominic Koepke 2019 - 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SIMPLE_GRAPH_DIJKSTRA_HPP
#define SIMPLE_GRAPH_DIJKSTRA_HPP

#pragma once

#include "generic_traverse.hpp"
#include "queue_helper.hpp"
#include "utility.hpp"

#include <cassert>
#include <map>
#include <ranges>

namespace sl::graph::dijkstra
{
	template <vertex_descriptor TVertex, weight TWeight>
	using node_t = weighted_node<TVertex, TWeight>;

	template <weight TWeight>
	using state_t = detail::dynamic_cost_state_t<TWeight>;

	template <
		vertex_descriptor TVertex,
		std::invocable<TVertex> TNeighborSearcher,
		std::invocable<TVertex, TVertex> TWeightCalculator,
		std::invocable<node_t<TVertex, detail::weight_type_of_t<TWeightCalculator, TVertex>>> TCallback = empty_invokable,
		std::predicate<node_t<TVertex, detail::weight_type_of_t<TWeightCalculator, TVertex>>, TVertex> TVertexPredicate
		= true_constant,
		state_map_for<TVertex, state_t<detail::weight_type_of_t<TWeightCalculator, TVertex>>> TStateMap
		= std::map<TVertex, state_t<detail::weight_type_of_t<TWeightCalculator, TVertex>>>>
		requires std::ranges::input_range<std::invoke_result_t<TNeighborSearcher, TVertex>>
	void traverse
	(
		TVertex begin,
		TNeighborSearcher&& neighborSearcher,
		TWeightCalculator&& weightCalculator,
		TCallback&& callback = {},
		TVertexPredicate&& vertexPredicate = {},
		TStateMap stateMap = {}
	)
	{
		using vertex_t = TVertex;
		using weight_t = detail::weight_type_of_t<TWeightCalculator, TVertex>;
		using node_t = node_t<vertex_t, weight_t>;
		using state_t = state_t<weight_t>;

		detail::dynamic_cost_traverse<node_t>
		(
			[&](const node_t& predecessor, const vertex_t& cur_vertex) -> node_t
			{
				weight_t rel_weight{ std::invoke(weightCalculator, predecessor.vertex, cur_vertex) };
				assert(weight_t{} <= rel_weight && "relative weight between nodes must be greater or equal zero.");

				return
				{
					.predecessor = predecessor.vertex,
					.vertex = cur_vertex,
					.weight_sum = predecessor.weight_sum + rel_weight
				};
			},
			node_t{ .vertex = std::move(begin), .weight_sum = {} },
			std::forward<TNeighborSearcher>(neighborSearcher),
			std::forward<TCallback>(callback),
			std::forward<TVertexPredicate>(vertexPredicate),
			std::move(stateMap),
			std::priority_queue<node_t, std::vector<node_t>, std::greater<>>{}
		);
	}
}

#endif
