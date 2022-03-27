//           Copyright Dominic Koepke 2022 - 2022.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)

#ifndef SIMPLE_GRAPH_DIJKSTRA_HPP
#define SIMPLE_GRAPH_DIJKSTRA_HPP

#pragma once

#include "generic_traverse.hpp"
#include "queue_helper.hpp"
#include "utility.hpp"

#include <cassert>
#include <functional>
#include <map>

namespace sl::graph::dijkstra
{
	template <vertex_descriptor TVertex, weight TWeight>
	using node_t = weighted_node<TVertex, TWeight>;

	template <weight TWeight>
	using state_t = detail::dynamic_cost_state_t<TWeight>;

	template <vertex_descriptor TVertex, weight TWeight>
	using default_open_list_t = std::priority_queue<node_t<TVertex, TWeight>, std::vector<node_t<TVertex, TWeight>>, std::greater<>>;

	template <
		vertex_descriptor TVertex,
		neighbor_searcher_for<TVertex> TNeighborSearcher,
		weight_calculator_for<TVertex> TWeightCalculator,
		node_callback<node_t<TVertex, detail::weight_type_of_t<TWeightCalculator, TVertex>>> TCallback = empty_invokable_t,
		vertex_predicate_for<node_t<TVertex, detail::weight_type_of_t<TWeightCalculator, TVertex>>> TVertexPredicate = true_constant_t,
		state_map_for<TVertex, state_t<detail::weight_type_of_t<TWeightCalculator, TVertex>>> TStateMap
		= std::map<TVertex, state_t<detail::weight_type_of_t<TWeightCalculator, TVertex>>>,
		open_list_for<node_t<TVertex, detail::weight_type_of_t<TWeightCalculator, TVertex>>> TOpenList
		= default_open_list_t<TVertex, detail::weight_type_of_t<TWeightCalculator, TVertex>>>
	struct Searcher
	{
		using vertex_t = std::remove_cvref_t<TVertex>;
		using weight_t = std::remove_cvref_t<detail::weight_type_of_t<TWeightCalculator, TVertex>>;
		using node_t = weighted_node<vertex_t, weight_t>;

		using neighbor_searcher_t = TNeighborSearcher;
		using callback_t = TCallback;
		using vertex_predicate_t = TVertexPredicate;
		using state_map_t = std::remove_cvref_t<TStateMap>;
		using open_list_t = std::remove_cvref_t<TOpenList>;

		TVertex begin;
		TNeighborSearcher neighborSearcher;
		TWeightCalculator weightCalculator;

		TCallback callback{};
		TVertexPredicate vertexPredicate{};
		TStateMap stateMap{};
		TOpenList openList{};
	};
}

namespace sl::graph
{
	template <class... TArgs>
	void traverse(dijkstra::Searcher<TArgs...> searcher)
	{
		using searcher_t = dijkstra::Searcher<TArgs...>;
		using vertex_t = typename searcher_t::vertex_t;
		using node_t = typename searcher_t::node_t;

		detail::dynamic_cost_traverse<node_t>
		(
			detail::make_weighted_node_factory<vertex_t>(std::ref(searcher.weightCalculator)),
			{ .vertex = std::move(searcher.begin) },
			std::ref(searcher.neighborSearcher),
			std::ref(searcher.callback),
			std::ref(searcher.vertexPredicate),
			std::move(searcher.stateMap),
			std::move(searcher.openList)
		);
	}
}

#endif
