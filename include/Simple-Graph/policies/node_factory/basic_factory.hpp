//          Copyright Dominic Koepke 2019 - 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SIMPLE_GRAPH_POLICIES_NODE_FACTORY_BASIC_FACTORY_HPP
#define SIMPLE_GRAPH_POLICIES_NODE_FACTORY_BASIC_FACTORY_HPP

#pragma once

#include "Simple-Graph/graph.hpp"
#include "Simple-Graph/node.hpp"

#include <cassert>

namespace sl::graph::policies::node_factory
{
	template <class T>
	struct basic_factory;

	template <concepts::vertex TVertex>
	struct basic_factory<node<TVertex>>
	{
	public:
		using node_type = node<TVertex>;

		[[nodiscard]]
		static constexpr node_type make_init_node(
			[[maybe_unused]] const concepts::graph auto& graph,
			const TVertex& vertex
		)
		{
			return {vertex, std::nullopt};
		}

		[[nodiscard]]
		static constexpr node_type make_successor_node(
			[[maybe_unused]] const concepts::graph auto& graph,
			const node_type& predecessor,
			const TVertex& vertex
		)
		{
			return {vertex, predecessor.vertex};
		}
	};

	template <concepts::vertex TVertex, concepts::weight TWeight>
	struct basic_factory<weighted_node<TVertex, TWeight>>
	{
	public:
		using node_type = weighted_node<TVertex, TWeight>;

		[[nodiscard]]
		static constexpr node_type make_init_node(
			[[maybe_unused]] const concepts::weighted_graph auto& graph,
			const TVertex& vertex
		)
		{
			return {vertex, {}, std::nullopt};
		}

		[[nodiscard]]
		static constexpr node_type make_successor_node(
			const concepts::weighted_graph auto& graph,
			const node_type& predecessor,
			const TVertex& vertex
		)
		{
			const TWeight relWeight = graph.weight(predecessor.vertex, vertex);
			assert(TWeight{} <= relWeight && "Weights between nodes must be positive.");

			return {
				vertex,
				predecessor.accumulatedWeight + relWeight,
				predecessor.vertex
			};
		}
	};
}

#endif
