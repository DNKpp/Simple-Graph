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
	template <class>
	struct take_next_t;

	template <class T, class TNode>
	concept open_list_for = requires(const T& container) { std::empty(container); }
							&& requires(T& container)
							{
								container.emplace(std::declval<TNode>());
								{ take_next_t<std::remove_cvref_t<T>>{}(container) } -> std::convertible_to<TNode>;
							};

	template <
		class TNode,
		std::invocable<const node_vertex_t<TNode>&> TNeighborSearcher,
		std::invocable<const TNode&> TCallback,
		std::predicate<const TNode&> TNodePredicate,
		open_list_for<TNode> TOpenList,
		state_map_for<node_vertex_t<TNode>, bool> TStateMap>
		requires std::ranges::input_range<std::invoke_result_t<TNeighborSearcher, const node_vertex_t<TNode>&>>
	void uniform_cost_traverse
	(
		node_vertex_t<TNode> begin,
		TNeighborSearcher neighborSearcher,
		TCallback callback,
		TNodePredicate nodePredicate,
		TOpenList openList,
		TStateMap stateMap
	)
	{
		using vertex_t = node_vertex_t<TNode>;

		assert(std::empty(openList));

		stateMap[begin] = true;
		openList.emplace(std::nullopt, std::move(begin), 0);

		while (!std::empty(openList))
		{
			auto node{ take_next_t<TOpenList>{}(openList) };

			if (detail::shall_interrupt(callback, node))
				return;

			for
			(
				auto&& neighbors = std::invoke(neighborSearcher, node.vertex);
				const vertex_t& current
				: neighbors
				| std::views::filter([&](const vertex_t& v) { return v != node.predecessor; })
				| std::views::filter([&stateMap](const vertex_t& v) { return !std::exchange(stateMap[v], true); })
			)
			{
				TNode currentNode{ node.vertex, current, node.weight_sum + 1 };
				if (std::invoke(nodePredicate, currentNode))
				{
					openList.emplace(std::move(currentNode));
				}
			}
		}
	}
}

#endif
