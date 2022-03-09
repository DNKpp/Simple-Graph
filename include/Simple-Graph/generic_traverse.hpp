//          Copyright Dominic Koepke 2019 - 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SIMPLE_GRAPH_GENERIC_TRAVERSE_HPP
#define SIMPLE_GRAPH_GENERIC_TRAVERSE_HPP

#pragma once

#include "utility.hpp"

#include <cassert>
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
		std::invocable<node_vertex_t<TNode>> TNeighborSearcher,
		std::invocable<TNode> TCallback,
		std::predicate<TNode, node_vertex_t<TNode>> TVertexPredicate,
		state_map_for<node_vertex_t<TNode>, bool> TStateMap,
		open_list_for<TNode> TOpenList>
		requires std::ranges::input_range<std::invoke_result_t<TNeighborSearcher, node_vertex_t<TNode>>>
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
				| std::views::filter([&](const vertex_t& v) { return v != predecessor.predecessor; })
				| std::views::filter([&stateMap](const vertex_t& v) { return !std::exchange(stateMap[v], true); })
				| std::views::filter([&](const vertex_t& v) { return std::invoke(vertexPredicate, predecessor, v); })
			)
			{
				openList.emplace(predecessor.vertex, current, predecessor.weight_sum + 1);
			}
		}
	}
}

#endif
