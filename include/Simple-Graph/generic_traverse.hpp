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
	concept open_list_for = requires(const T& container) { std::empty(container); }
							&& requires(T& container)
							{
								container.emplace(std::declval<TNode>());
								{ take_next_func_t<std::remove_cvref_t<T>>{}(container) } -> std::convertible_to<TNode>;
							};

	template <
		class TNode,
		neighbor_searcher_for<node_vertex_t<TNode>> TNeighborSearcher,
		node_callback<TNode> TCallback,
		vertex_predicate_for<TNode> TVertexPredicate,
		state_map_for<node_vertex_t<TNode>, bool> TStateMap,
		open_list_for<TNode> TOpenList>
	void uniform_cost_traverse
	(
		node_vertex_t<TNode> begin,
		TNeighborSearcher neighborSearcher,
		TCallback callback,
		TVertexPredicate vertexPredicate,
		TStateMap stateMap,
		TOpenList openList
	)
	{
		using vertex_t = node_vertex_t<TNode>;

		assert(std::empty(openList));

		stateMap[begin] = true;
		openList.emplace(std::nullopt, std::move(begin), 0);

		while (!std::empty(openList))
		{
			TNode predecessor{ take_next_func_t<TOpenList>{}(openList) };

			if (detail::shall_interrupt(callback, predecessor))
				return;

			for
			(
				auto&& neighbors = std::invoke(neighborSearcher, predecessor.vertex);
				const vertex_t& current
				: neighbors
				| std::views::filter([&stateMap](const vertex_t& v) { return !std::exchange(stateMap[v], true); })
				| std::views::filter([&](const vertex_t& v) { return std::invoke(vertexPredicate, predecessor, v); })
			)
			{
				openList.emplace(predecessor.vertex, current, predecessor.weight_sum + 1);
			}
		}
	}

	template <weight TWeight>
	using dynamic_cost_state_t = std::tuple<visit_state, TWeight>;

	template <class T, class TNode>
	concept node_factory_for = std::invocable<T, TNode, node_vertex_t<TNode>>
								&& std::convertible_to<std::invoke_result_t<T, TNode, node_vertex_t<TNode>>, TNode>;

	template <
		class TNode,
		class TVertex = node_vertex_t<TNode>,
		class TWeight = node_weight_t<TNode>>
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
		using node_t = TNode;
		using vertex_t = TVertex;
		using weight_t = TWeight;
		using state_t = dynamic_cost_state_t<weight_t>;

		assert(std::empty(openList));

		stateMap[begin.vertex] = { visit_state::discovered, weight_t{} };
		openList.emplace(std::move(begin));

		while (!std::empty(openList))
		{
			node_t predecessor{ detail::take_next(openList) };
			if (visit_state::visited == std::exchange(std::get<0>(stateMap[predecessor.vertex]), visit_state::visited))
				continue;

			if (detail::shall_interrupt(callback, predecessor))
				return;

			for (const vertex_t& cur_vertex : std::invoke(neighborSearcher, predecessor.vertex))
			{
				auto&& [cur_state, cur_weight] = stateMap[cur_vertex];
				if (cur_state == visit_state::visited || !std::invoke(vertexPredicate, predecessor, cur_vertex))
					continue;

				node_t current{ std::invoke(nodeFactory, predecessor, cur_vertex) };
				switch (cur_state)
				{
				case visit_state::none:
					cur_state = visit_state::discovered;
					cur_weight = static_cast<weight_t>(current);
					openList.emplace(std::move(current));
					break;

				case visit_state::discovered:
					if (current.weight_sum < cur_weight)
					{
						cur_weight = static_cast<weight_t>(current);
						openList.emplace(std::move(current));
					}
					break;
				}
			}
		}
	}
}

#endif
