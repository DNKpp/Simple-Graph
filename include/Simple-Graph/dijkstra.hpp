#ifndef SIMPLE_GRAPH_DIJKSTRA_HPP
#define SIMPLE_GRAPH_DIJKSTRA_HPP

#pragma once

#include "generic_traverse.hpp"
#include "queue_helper.hpp"
#include "utility.hpp"

#include <cassert>
#include <functional>
#include <map>
#include <ranges>

namespace sl::graph::detail
{
	template <vertex_descriptor TVertex, std::invocable<TVertex, TVertex> TWeightCalculator>
	struct weighted_node_factory_t
	{
		using vertex_t = TVertex;
		using weight_t = weight_type_of_t<TWeightCalculator, TVertex>;
		using node_t = weighted_node<vertex_t, weight_t>;

		TWeightCalculator weightCalculator{};

		constexpr node_t operator ()(const node_t& predecessor, const TVertex& cur_vertex)
		{
			weight_t rel_weight{ std::invoke(weightCalculator, predecessor.vertex, cur_vertex) };
			assert(weight_t{} <= rel_weight && "relative weight between nodes must be greater or equal zero.");

			return
			{
				.predecessor = predecessor.vertex,
				.vertex = cur_vertex,
				.weight_sum = predecessor.weight_sum + rel_weight
			};
		}
	};

	template <vertex_descriptor TVertex, class TWeightCalculator>
	[[nodiscard]]
	constexpr weighted_node_factory_t<TVertex, TWeightCalculator> make_weighted_node_factory(TWeightCalculator&& weightCalc)
	{
		return { .weightCalculator = std::forward<TWeightCalculator>(weightCalc) };
	}
}

namespace sl::graph::dijkstra
{
	template <vertex_descriptor TVertex, weight TWeight>
	using node_t = weighted_node<TVertex, TWeight>;

	template <weight TWeight>
	using state_t = detail::dynamic_cost_state_t<TWeight>;

	template <class TNode>
	using default_open_list_t = std::priority_queue<TNode, std::vector<TNode>, std::greater<>>;

	template <
		vertex_descriptor TVertex,
		std::invocable<TVertex> TNeighborSearcher,
		std::invocable<TVertex, TVertex> TWeightCalculator,
		std::invocable<node_t<TVertex, detail::weight_type_of_t<TWeightCalculator, TVertex>>> TCallback = empty_invokable,
		std::predicate<node_t<TVertex, detail::weight_type_of_t<TWeightCalculator, TVertex>>, TVertex> TVertexPredicate
		= true_constant,
		state_map_for<TVertex, state_t<detail::weight_type_of_t<TWeightCalculator, TVertex>>> TStateMap
		= std::map<TVertex, state_t<detail::weight_type_of_t<TWeightCalculator, TVertex>>>,
		class TOpenList = default_open_list_t<node_t<TVertex, detail::weight_type_of_t<TWeightCalculator, TVertex>>>>
		requires std::ranges::input_range<std::invoke_result_t<TNeighborSearcher, TVertex>>
	struct Searcher
	{
		using vertex_t = TVertex;
		using weight_t = detail::weight_type_of_t<TWeightCalculator, TVertex>;
		using node_t = weighted_node<vertex_t, weight_t>;
		using open_list_t = TOpenList;

		TVertex begin;
		TNeighborSearcher neighborSearcher;
		TWeightCalculator weightCalculator;

		TCallback callback{};
		TVertexPredicate vertexPredicate{};
		TStateMap stateMap{};
	};
}

namespace sl::graph
{
	template <class... TArgs>
	void traverse(dijkstra::Searcher<TArgs...> searcher)
	{
		using searcher_t = dijkstra::Searcher<TArgs...>;
		using vertex_t = typename searcher_t::vertex_t;
		using weight_t = typename searcher_t::weight_t;
		using node_t = typename searcher_t::node_t;
		using open_list_t = typename searcher_t::open_list_t;

		detail::dynamic_cost_traverse<node_t>
		(
			detail::make_weighted_node_factory<vertex_t>(searcher.weightCalculator),
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
