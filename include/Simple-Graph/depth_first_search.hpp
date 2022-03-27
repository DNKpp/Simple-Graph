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
	/** \addtogroup dfs
	* @{
	*/

	/**
	 * \brief The node type of the algorithm.
	 */
	template <vertex_descriptor TVertex>
	using node_t = weighted_node<TVertex, int>;

	/**
	 * \brief The state type of the algorithm.
	 */
	using state_t = bool;

	/**
	 * \brief The default open list type of the algorithm.
	 */
	template <vertex_descriptor TVertex>
	using default_open_list_t = std::stack<node_t<TVertex>>;

	/**
	 * \brief The searcher of the algorithm.
	 * \tparam TVertex The vertex type.
	 * \tparam TNeighborSearcher The neighbor searcher type.
	 * \tparam TCallback The callback type.
	 * \tparam TVertexPredicate The vertex predicate type.
	 * \tparam TStateMap The state map type.
	 * \tparam TOpenList The open list type.
	 */
	template <
		vertex_descriptor TVertex,
		neighbor_searcher_for<TVertex> TNeighborSearcher,
		node_callback<node_t<TVertex>> TCallback = empty_invokable_t,
		vertex_predicate_for<node_t<TVertex>> TVertexPredicate = true_constant_t,
		state_map_for<TVertex, state_t> TStateMap = std::map<TVertex, state_t>,
		open_list_for<node_t<TVertex>> TOpenList = default_open_list_t<TVertex>>
	struct searcher
	{
		/**
		 * \brief Alias for the actual value type of the vertex.
		 */
		using vertex_t = std::remove_cvref_t<TVertex>;

		/**
		 * \brief Alias for the actual weight type of the weight.
		 */
		using weight_t = int;

		/**
		 * \brief Alias for the node type.
		 */
		using node_t = weighted_node<vertex_t, weight_t>;

		/**
		 * \brief Alias for the neighbor searcher type.
		 */
		using neighbor_searcher_t = TNeighborSearcher;

		/**
		 * \brief Alias for the callback type.
		 */
		using callback_t = TCallback;

		/**
		 * \brief Alias for the vertex predicate type.
		 */
		using vertex_predicate_t = TVertexPredicate;

		/**
		 * \brief Alias for the state map type.
		 */
		using state_map_t = std::remove_cvref_t<TStateMap>;

		/**
		 * \brief Alias for the open list type.
		 */
		using open_list_t = std::remove_cvref_t<TOpenList>;

		/**
		 * \brief The vertex to start the traversal.
		 */
		TVertex begin{};

		/**
		 * \brief The neighbor searcher to be used. Must be an invokable type, which returns a range of vertices.
		 * \details The signature of the invocation operator should match the following:
		 * \code
		 * range<vertex_t> operator()(const vertex_t& current)
		 * \endcode
		 * where ``range<vertex_t>`` must satisfy the requirements of a ``std::ranges::input_range`` type.
		 */
		TNeighborSearcher neighborSearcher{};

		/**
		 * \brief The callback object to be invoked for each finalized node.
		 * \remark If not set, an empty_invokable_t object is used.
		 * \details The signature of the invocation operator should match the following:
		 * \code
		 * void operator()(const node_t& current)
		 * \endcode
		 *
		 * Alternatively the invoke operator may return a boolean-convertible type, where true indicates that the algorithm shall
		 * be aborted.
		 */
		TCallback callback{};

		/**
		 * \brief The predicate, which will be invoked for each discovered vertex.
		 * \remark If not set, an true_constant_t object is used.
		 * \details The signature of the invocation operator should match the following:
		 * \code
		 * bool operator()(const node_t& predecessor, const vertex_t& current)
		 * \endcode
		 * The predicates determines whether a vertex shall be further investigated, where false means it will be skipped.
		 */
		TVertexPredicate vertexPredicate{};

		/**
		 * \brief The state map prototype object.
		 * \note If not explicitly set, a ``std::map```is used, thus the vertex type must be less-comparable.
		 * \see state_map_for concept for the actual type requirements.
		 */
		TStateMap stateMap{};

		/**
		 * \brief The open list prototype object.
		 * \note If not explicitly set, a ``std::stack`` is used.
		 * \see open_list_for concept for the actual type requirements.
		 * \attention As the used open list has a huge impact on the actual behaviour of the algorithm, the type of the provided
		 * open list should model the behaviour of a stack-like container. Otherwise this result in undefined behaviour.
		 */
		TOpenList openList{};
	};

	/** @}*/
}

namespace sl::graph
{
	/**
	 * \brief Overload for dfs::searcher.
	 * \tparam TArgs Template arguments for the searcher.
	 * \param searcher The searcher object.
	 * \ingroup dfs
	 */
	template <class... TArgs>
	void traverse(dfs::searcher<TArgs...> searcher)
	{
		using searcher_t = bfs::searcher<TArgs...>;
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
