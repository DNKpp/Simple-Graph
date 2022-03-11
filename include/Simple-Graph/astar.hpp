//           Copyright Dominic Koepke 2022 - 2022.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)

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

	template <vertex_descriptor TVertex, class TWeightCalculator, class THeuristic>
	[[nodiscard]]
	constexpr astar_node_factory_t<TVertex, TWeightCalculator, THeuristic> make_astar_node_factory
	(
		TWeightCalculator&& weightCalc,
		THeuristic&& heuristic
	)
	{
		return { .weightCalculator = std::forward<TWeightCalculator>(weightCalc), .heuristic = std::forward<THeuristic>(heuristic) };
	}
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

	template <class TNode>
	using default_open_list_t = std::priority_queue<TNode, std::vector<TNode>, std::greater<>>;

	template <class T, class TVertex, class TWeightCalculator>
	concept compatible_heuristic_for = vertex_descriptor<TVertex>
										&& weight_calculator_for<TWeightCalculator, TVertex>
										&& std::invocable<T, TVertex>
										&& std::convertible_to<
											std::invoke_result_t<T, TVertex>, detail::weight_type_of_t<TWeightCalculator, TVertex>
										>;

	template <
		vertex_descriptor TVertex,
		neighbor_searcher_for<TVertex> TNeighborSearcher,
		weight_calculator_for<TVertex> TWeightCalculator,
		compatible_heuristic_for<TVertex, TWeightCalculator> THeuristic,
		node_callback<node_t<TVertex, detail::weight_type_of_t<TWeightCalculator, TVertex>>> TCallback = empty_invokable,
		vertex_predicate_for<node_t<TVertex, detail::weight_type_of_t<TWeightCalculator, TVertex>>> TVertexPredicate = true_constant,
		state_map_for<TVertex, state_t<detail::weight_type_of_t<TWeightCalculator, TVertex>>> TStateMap
		= std::map<TVertex, state_t<detail::weight_type_of_t<TWeightCalculator, TVertex>>>,
		open_list_for<node_t<TVertex, detail::weight_type_of_t<TWeightCalculator, TVertex>>> TOpenList
		= default_open_list_t<node_t<TVertex, detail::weight_type_of_t<TWeightCalculator, TVertex>>>>
	struct Searcher
	{
		using vertex_t = TVertex;
		using weight_t = detail::weight_type_of_t<TWeightCalculator, TVertex>;
		using node_t = detail::astar_node<vertex_t, weight_t>;
		using open_list_t = TOpenList;

		TVertex begin;
		TNeighborSearcher neighborSearcher;
		TWeightCalculator weightCalculator;
		THeuristic heuristic;

		TCallback callback{};
		TVertexPredicate vertexPredicate{};
		TStateMap stateMap{};
	};
}

namespace sl::graph
{
	template <class... TArgs>
	void traverse(astar::Searcher<TArgs...> searcher)
	{
		using searcher_t = astar::Searcher<TArgs...>;
		using vertex_t = typename searcher_t::vertex_t;
		using weight_t = typename searcher_t::weight_t;
		using node_t = typename searcher_t::node_t;
		using open_list_t = typename searcher_t::open_list_t;

		detail::dynamic_cost_traverse<node_t>
		(
			detail::make_astar_node_factory<vertex_t>(searcher.weightCalculator, searcher.heuristic),
			node_t{ .vertex = std::move(searcher.begin) },
			searcher.neighborSearcher,
			searcher.callback,
			searcher.vertexPredicate,
			std::move(searcher.stateMap),
			open_list_t{}
		);
	}
}

#endif
