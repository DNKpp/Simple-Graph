//           Copyright Dominic Koepke 2019 - 2022.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)

#ifndef SIMPLE_GRAPH_BREADTH_FIRST_SEARCH_HPP
#define SIMPLE_GRAPH_BREADTH_FIRST_SEARCH_HPP

#pragma once

#include "generic_traverse.hpp"
#include "path_finder.hpp"
#include "queue_helper.hpp"
#include "utility.hpp"

#include <map>

namespace sl::graph::bfs
{
	/** \addtogroup bfs
	* @{
	* \brief Provides the search_params and other related things the breadth first search algorithm.
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
	using default_open_list_t = std::queue<node_t<TVertex>>;

	/**
	 * \brief The search_params of the algorithm.
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
	struct search_params
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
		 * \note If not explicitly set, a ``std::queue`` is used.
		 * \see open_list_for concept for the actual type requirements.
		 * \attention As the used open list has a huge impact on the actual behaviour of the algorithm, the type of the provided
		 * open list should model the behaviour of a queue-like container. Otherwise this results in undefined behaviour.
		 */
		TOpenList openList{};
	};

	/** @}*/
}

namespace sl::graph
{
	/**
	 * \brief Overload for bfs::search_params.
	 * \tparam TArgs Template arguments for the search_params.
	 * \param params The search_params object.
	 * \ingroup bfs
	 */
	template <class... TArgs>
	void traverse(bfs::search_params<TArgs...> params)
	{
		using params_t = bfs::search_params<TArgs...>;
		using vertex_t = typename params_t::vertex_t;
		using node_t = typename params_t::node_t;

		detail::uniform_cost_traverse<node_t>
		(
			detail::make_weighted_node_factory<vertex_t>(constant_t<1>{}),
			{ .vertex = std::move(params.begin) },
			std::ref(params.neighborSearcher),
			std::ref(params.callback),
			std::ref(params.vertexPredicate),
			std::move(params.stateMap),
			std::move(params.openList)
		);
	}

	/**
	 * \brief Executes the algorithm and returns a vector, where each element is a vertex on the existing path.
	 * \tparam TArgs Template arguments for the search_params.
	 * \tparam TVertex The used vertex type. Do not change!
	 * \param params The search_params object.
	 * \param predecessorMap The provided predecessor map, where each predecessor will be stored in between.
	 * \return Returns the path as vector if exists. Otherwise ``std::nullopt`` is returned.
	 * \details This function actually forwards the ``searcher_params`` to the appropriate traverse function, but
	 * wraps the callback into a ``path_finder_t``. Unlike to the usual ``search_params`` constraints, the callback
	 * must return a boolean convertible type.
	 * \note The path will be ordered from destination to begin, thus reversed.
	 * \attention If the predecessorMap contains any predefined graph state, the behaviour is this function is undefined.
	 * \ingroup bfs
	 */
	template <class... TArgs, vertex_descriptor TVertex = typename bfs::search_params<TArgs...>::vertex_t>
	[[nodiscard]]
	std::optional<std::vector<TVertex>> find_path
	(
		bfs::search_params<TArgs...> params,
		predecessor_map_for<TVertex> auto predecessorMap = std::map<TVertex, std::optional<TVertex>>{}
	)
	{
		using params_t = bfs::search_params<TArgs...>;
		using vertex_t = typename params_t::vertex_t;
		using node_t = typename params_t::node_t;

		return detail::extract_path<node_t>
		(
			std::ref(params.callback),
			std::move(predecessorMap),
			[&](auto path_extractor)
			{
				detail::uniform_cost_traverse<node_t>
				(
					detail::make_weighted_node_factory<vertex_t>(constant_t<1>{}),
					{ .vertex = std::move(params.begin) },
					std::ref(params.neighborSearcher),
					std::ref(path_extractor),
					std::ref(params.vertexPredicate),
					std::move(params.stateMap),
					std::move(params.openList)
				);
			}
		);
	}
}

#endif
