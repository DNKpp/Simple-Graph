//          Copyright Dominic Koepke 2019 - 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SIMPLE_GRAPH_DIJKSTRA_HPP
#define SIMPLE_GRAPH_DIJKSTRA_HPP

#pragma once

#include "utility.hpp"

#include <cassert>
#include <map>
#include <queue>
#include <ranges>
#include <tuple>

template <class... TArgs>
struct sl::graph::detail::take_next_func_t<std::priority_queue<TArgs...>>
{
	constexpr auto operator ()(std::priority_queue<TArgs...>& container) const
	{
		auto el{ std::move(container.top()) };
		container.pop();
		return el;
	}
};

namespace sl::graph::dijkstra
{
	template <class TWeightCalculator, class TVertex>
	using weight_type_of_t = std::remove_cvref_t<std::invoke_result_t<TWeightCalculator, const TVertex&, const TVertex&>>;

	template <class TWeight>
	using state_t = std::tuple<visit_state, TWeight>;

	template <class TVertex, class TWeight>
	using default_state_map_t = std::map<TVertex, state_t<TWeight>>;

	template <
		vertex_descriptor TVertex,
		std::invocable<TVertex> TNeighborSearcher,
		std::invocable<TVertex, TVertex> TWeightCalculator,
		std::invocable<weighted_node<TVertex, weight_type_of_t<TWeightCalculator, TVertex>>> TCallback = empty_invokable,
		std::predicate<weighted_node<TVertex, weight_type_of_t<TWeightCalculator, TVertex>>, TVertex> TVertexPredicate = true_constant,
		state_map_for<TVertex, state_t<weight_type_of_t<TWeightCalculator, TVertex>>> TStateMap
		= default_state_map_t<TVertex, state_t<weight_type_of_t<TWeightCalculator, TVertex>>>>
		requires std::ranges::input_range<std::invoke_result_t<TNeighborSearcher, TVertex>>
	void traverse
	(
		TVertex begin,
		TNeighborSearcher neighborSearcher,
		TWeightCalculator weightCalculator,
		TCallback callback = {},
		TVertexPredicate vertexPredicate = {},
		TStateMap stateMap = {}
	)
	{
		using vertex_t = TVertex;
		using weight_t = weight_type_of_t<TWeightCalculator, TVertex>;
		using node_t = weighted_node<vertex_t, weight_t>;
		using state_t = state_t<weight_t>;

		stateMap[begin] = { visit_state::discovered, weight_t{} };
		std::priority_queue<node_t, std::vector<node_t>, std::greater<node_t>> openList{};
		openList.emplace(std::nullopt, std::move(begin), weight_t{});

		while (!std::empty(openList))
		{
			node_t predecessor{ detail::take_next(openList) };
			if (visit_state::visited == std::exchange(std::get<0>(stateMap[predecessor.vertex]), visit_state::visited))
				continue;

			if (detail::shall_interrupt(callback, predecessor))
				return;

			for
			(
				auto&& neighbors{ std::invoke(neighborSearcher, predecessor.vertex) };
				const vertex_t& cur_vertex
				: neighbors
				| std::views::filter([&](const vertex_t& v) { return v != predecessor.predecessor; })
			)
			{
				auto&& [cur_state, cur_weight] = stateMap[cur_vertex];
				if (cur_state == visit_state::visited || !std::invoke(vertexPredicate, predecessor, cur_vertex))
					continue;

				weight_t rel_weight{ std::invoke(weightCalculator, predecessor.vertex, cur_vertex) };
				assert(weight_t{} <= rel_weight && "relative weight between nodes must be greater or equal zero.");
				node_t current{ predecessor.vertex, cur_vertex, predecessor.weight_sum + rel_weight };

				switch (cur_state)
				{
				case visit_state::none:
					cur_state = visit_state::discovered;
					cur_weight = current.weight_sum;
					openList.emplace(std::move(current));
					break;

				case visit_state::discovered:
					if (current.weight_sum < cur_weight)
					{
						cur_weight = current.weight_sum;
						openList.emplace(std::move(current));
					}
					break;

				default:
					assert(false && "Should never be reached.");
					break;
				}
			}
		}
	}
}

#endif
