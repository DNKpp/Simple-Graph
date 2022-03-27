//           Copyright Dominic Koepke 2022 - 2022.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)

#ifndef SIMPLE_GRAPH_GENERIC_TRAVERSE_HPP
#define SIMPLE_GRAPH_GENERIC_TRAVERSE_HPP

#pragma once

#include "utility.hpp"

#include <cassert>
#include <concepts>
#include <ranges>

namespace sl::graph::detail
{
	template <class T, class TNode>
	concept node_factory_for = std::invocable<T, TNode, node_vertex_t<TNode>>
								&& std::convertible_to<std::invoke_result_t<T, TNode, node_vertex_t<TNode>>, TNode>;

	template <class TNode, class TVertex = node_vertex_t<TNode>, class TWeight = node_weight_t<TNode>>
	void uniform_cost_traverse
	(
		node_factory_for<TNode> auto&& nodeFactory,
		TNode begin,
		neighbor_searcher_for<TVertex> auto&& neighborSearcher,
		node_callback<TNode> auto&& callback,
		vertex_predicate_for<TNode> auto&& vertexPredicate,
		state_map_for<node_vertex_t<TNode>, bool> auto&& stateMap,
		open_list_for<TNode> auto&& openList
	)
	{
		assert(detail::empty_helper(openList));

		stateMap[begin.vertex] = true;
		emplace(openList, std::move(begin));

		while (!detail::empty_helper(openList))
		{
			TNode predecessor{ take_next(openList) };

			if (detail::shall_interrupt(callback, predecessor))
				return;

			for
			(
				auto&& neighbors = std::invoke(neighborSearcher, predecessor.vertex);
				const TVertex& cur_vertex
				: neighbors
				| std::views::filter([&stateMap](const TVertex& v) { return !std::exchange(stateMap[v], true); })
				| std::views::filter([&](const TVertex& v) { return std::invoke(vertexPredicate, predecessor, v); })
			)
			{
				emplace(openList, std::invoke(nodeFactory, predecessor, cur_vertex));
			}
		}
	}

	template <weight TWeight>
	using dynamic_cost_state_t = std::tuple<visit_state, TWeight>;

	template <class TNode, class TVertex = node_vertex_t<TNode>, class TWeight = node_weight_t<TNode>>
	void dynamic_cost_traverse
	(
		node_factory_for<TNode> auto&& nodeFactory,
		TNode begin,
		neighbor_searcher_for<TVertex> auto&& neighborSearcher,
		node_callback<TNode> auto&& callback,
		vertex_predicate_for<TNode> auto&& vertexPredicate,
		state_map_for<TVertex, dynamic_cost_state_t<TWeight>> auto&& stateMap,
		open_list_for<TNode> auto&& openList
	)
	{
		using state_t = dynamic_cost_state_t<TWeight>;

		assert(detail::empty_helper(openList));

		stateMap[begin.vertex] = { visit_state::discovered, TWeight{} };
		emplace(openList, std::move(begin));

		while (!detail::empty_helper(openList))
		{
			TNode predecessor{ take_next(openList) };
			if (visit_state::visited == std::exchange(std::get<0>(stateMap[predecessor.vertex]), visit_state::visited))
				continue;

			if (detail::shall_interrupt(callback, predecessor))
				return;

			for (const TVertex& cur_vertex : std::invoke(neighborSearcher, predecessor.vertex))
			{
				auto&& [cur_state, cur_weight] = stateMap[cur_vertex];
				if (cur_state == visit_state::visited || !std::invoke(vertexPredicate, predecessor, cur_vertex))
					continue;

				TNode current{ std::invoke(nodeFactory, predecessor, cur_vertex) };
				switch (cur_state)
				{
				case visit_state::none:
					cur_state = visit_state::discovered;
					cur_weight = static_cast<TWeight>(current);
					emplace(openList, std::move(current));
					break;

				case visit_state::discovered:
					if (current.weight_sum < cur_weight)
					{
						cur_weight = static_cast<TWeight>(current);
						emplace(openList, std::move(current));
					}
					break;
				}
			}
		}
	}
}

#endif
