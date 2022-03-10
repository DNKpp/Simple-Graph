//          Copyright Dominic Koepke 2019 - 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SIMPLE_GRAPH_ASTAR_HPP
#define SIMPLE_GRAPH_ASTAR_HPP

#pragma once

#include "generic_traverse.hpp"
#include "queue_helper.hpp"
#include "utility.hpp"

#include <map>

namespace sl::graph::detail
{
	template <vertex_descriptor TVertex, weight TWeight>
	struct astar_node
	{
		using vertex_type = TVertex;
		using weight_type = TWeight;

		std::optional<TVertex> predecessor{};
		TVertex vertex{};
		TWeight weight_sum{};
		TWeight weight_estimated{};

		[[nodiscard]]
		constexpr bool operator ==(const astar_node& other) const noexcept = default;

		[[nodiscard]]
		constexpr std::compare_three_way_result_t<weight_type> operator <=>(const astar_node& other) const noexcept
		{
			return static_cast<TWeight>(*this) <=> static_cast<TWeight>(other);
		}

		[[nodiscard]]
		explicit constexpr operator TWeight() const noexcept
		{
			return weight_sum + weight_estimated;
		}
	};

	template <vertex_descriptor TVertex, std::invocable<TVertex, TVertex> TWeightCalculator, std::invocable<TVertex> THeuristic>
	struct astar_node_factory_t
	{
		using vertex_t = TVertex;
		using weight_t = weight_type_of_t<TWeightCalculator, TVertex>;
		using node_t = astar_node<vertex_t, weight_t>;

		TWeightCalculator weightCalculator{};
		THeuristic heuristic{};

		constexpr node_t operator ()(const node_t& predecessor, const TVertex& cur_vertex)
		{
			weight_t rel_weight{ std::invoke(weightCalculator, predecessor.vertex, cur_vertex) };
			assert(weight_t{} <= rel_weight && "relative weight between nodes must be greater or equal zero.");

			weight_t estimated_weight{ std::invoke(heuristic, cur_vertex) };
			assert(weight_t{} <= estimated_weight && "estimated weight must be greater or equal zero.");

			return
			{
				.predecessor = predecessor.vertex,
				.vertex = cur_vertex,
				.weight_sum = predecessor.weight_sum + rel_weight,
				.weight_estimated = estimated_weight
			};
		}
	};
}

template <class TVertex, class TWeight>
struct sl::graph::node_traits<sl::graph::detail::astar_node<TVertex, TWeight>>
{
	using node_t = detail::astar_node<TVertex, TWeight>;
	using vertex_type = typename node_t::vertex_type;
	using weight_type = typename node_t::weight_type;
};

namespace sl::graph::astar
{
	template <vertex_descriptor TVertex, weight TWeight>
	using node_t = detail::astar_node<TVertex, TWeight>;

	template <weight TWeight>
	using state_t = detail::dynamic_cost_state_t<TWeight>;

	template <
		vertex_descriptor TVertex,
		std::invocable<TVertex> TNeighborSearcher,
		std::invocable<TVertex, TVertex> TWeightCalculator,
		std::invocable<TVertex> THeuristic,
		std::invocable<node_t<TVertex, detail::weight_type_of_t<TWeightCalculator, TVertex>>> TCallback = empty_invokable,
		std::predicate<node_t<TVertex, detail::weight_type_of_t<TWeightCalculator, TVertex>>, TVertex> TVertexPredicate
		= true_constant,
		state_map_for<TVertex, state_t<detail::weight_type_of_t<TWeightCalculator, TVertex>>> TStateMap
		= std::map<TVertex, state_t<detail::weight_type_of_t<TWeightCalculator, TVertex>>>>
		requires std::ranges::input_range<std::invoke_result_t<TNeighborSearcher, TVertex>>
				&& std::convertible_to<std::invoke_result_t<THeuristic, TVertex>, detail::weight_type_of_t<TWeightCalculator, TVertex>>
	void traverse
	(
		TVertex begin,
		TNeighborSearcher&& neighborSearcher,
		TWeightCalculator&& weightCalculator,
		THeuristic&& heuristic,
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
			detail::astar_node_factory_t<vertex_t, TWeightCalculator, THeuristic>
			{
				.weightCalculator = std::forward<TWeightCalculator>(weightCalculator),
				.heuristic = std::forward<THeuristic>(heuristic)
			},
			node_t{ .vertex = std::move(begin) },
			std::forward<TNeighborSearcher>(neighborSearcher),
			std::forward<TCallback>(callback),
			std::forward<TVertexPredicate>(vertexPredicate),
			std::move(stateMap),
			std::priority_queue<node_t, std::vector<node_t>, std::greater<>>{}
		);
	}
}

#endif
