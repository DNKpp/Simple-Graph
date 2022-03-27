//           Copyright Dominic Koepke 2022 - 2022.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)

#ifndef SIMPLE_GRAPH_DEPTH_FIRST_SEARCH_HPP
#define SIMPLE_GRAPH_DEPTH_FIRST_SEARCH_HPP

#pragma once

#include "generic_traverse.hpp"
#include "stack_helper.hpp"
#include "utility.hpp"

#include <map>

namespace sl::graph::dfs
{
	template <vertex_descriptor TVertex>
	using node_t = weighted_node<TVertex, int>;

	using state_t = bool;

	template <vertex_descriptor TVertex>
	using default_open_list_t = std::stack<node_t<TVertex>>;

	template <
		vertex_descriptor TVertex,
		neighbor_searcher_for<TVertex> TNeighborSearcher,
		node_callback<node_t<TVertex>> TCallback = empty_invokable_t,
		vertex_predicate_for<node_t<TVertex>> TVertexPredicate = true_constant_t,
		state_map_for<TVertex, state_t> TStateMap = std::map<TVertex, state_t>,
		open_list_for<node_t<TVertex>> TOpenList = default_open_list_t<TVertex>>
	struct Searcher
	{
		using vertex_t = std::remove_cvref_t<TVertex>;
		using weight_t = int;
		using node_t = weighted_node<vertex_t, weight_t>;

		using neighbor_searcher_t = TNeighborSearcher;
		using callback_t = TCallback;
		using vertex_predicate_t = TVertexPredicate;
		using state_map_t = std::remove_cvref_t<TStateMap>;
		using open_list_t = std::remove_cvref_t<TOpenList>;

		TVertex begin{};
		TNeighborSearcher neighborSearcher{};

		TCallback callback{};
		TVertexPredicate vertexPredicate{};
		TStateMap stateMap{};
		TOpenList openList{};
	};
}

namespace sl::graph
{
	template <class... TArgs>
	void traverse(dfs::Searcher<TArgs...> searcher)
	{
		using searcher_t = bfs::Searcher<TArgs...>;
		using vertex_t = typename searcher_t::vertex_t;
		using node_t = typename searcher_t::node_t;

		detail::uniform_cost_traverse<node_t>
		(
			detail::make_weighted_node_factory<vertex_t>(constant_t<1>{}),
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
