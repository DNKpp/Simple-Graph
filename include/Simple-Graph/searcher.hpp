//          Copyright Dominic Koepke 2019 - 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SIMPLE_GRAPH_SEARCHER_HPP
#define SIMPLE_GRAPH_SEARCHER_HPP

#pragma once

#include "Simple-Graph/policies/node_factory/common.hpp"
#include "Simple-Graph/policies/open_list/common.hpp"
#include "Simple-Graph/policies/visit_tracking/common.hpp"

namespace sl::graph
{
	template <
		class TGraph,
		class TNode,
		concepts::node_factory_policy_for<TNode, TGraph> TNodeFactoryPolicy,
		concepts::open_list_policy_for<TNode> TOpenListPolicy,
		concepts::visit_track_policy_for<node_vertex_type<TNode>> TVisitTrackPolicy
	>
		requires (concepts::weighted_graph<TGraph> || !concepts::weighted_node<TNode>)
	class Searcher final
		: private TNodeFactoryPolicy,
		private TOpenListPolicy,
		private TVisitTrackPolicy
	{
	private:
		using TNodeFactoryPolicy::make_init_node;
		using TNodeFactoryPolicy::make_successor_node;
		using TOpenListPolicy::enqueue;
		using TOpenListPolicy::take_next;
		using TOpenListPolicy::has_pending;
		using TVisitTrackPolicy::is_visited;
		using TVisitTrackPolicy::set_visited;

	public:
		using node_type = TNode;
		using vertex_type = node_vertex_type<node_type>;

		[[nodiscard]]
		constexpr Searcher(const TGraph& graph, const vertex_type& beginVertex)
			: m_Graph{std::addressof(graph)}
		{
			enqueue(make_init_node(*m_Graph, beginVertex));
		}

		[[nodiscard]]
		constexpr std::optional<node_type> next()
		{
			if (std::optional current_opt = find_next())
			{
				for (const vertex_type& vertex : m_Graph->neighbors(current_opt->vertex))
				{
					if (!is_visited(vertex))
					{
						enqueue(make_successor_node(*m_Graph, *current_opt, vertex));
					}
				}

				return current_opt;
			}

			return std::nullopt;
		}

		[[nodiscard]]
		constexpr bool has_finished() const
		{
			return !has_pending();
		}

	private:
		const TGraph* m_Graph;

		[[nodiscard]]
		constexpr std::optional<node_type> find_next()
		{
			while (has_pending())
			{
				if (node_type next = take_next(); !set_visited(next.vertex))
				{
					return next;
				}
			}

			return std::nullopt;
		}
	};
}

namespace sl::graph::detail
{
	template <class TAlgorithm, class TGraph>
	using searcher_type = Searcher<
		TGraph,
		typename TAlgorithm::node_type,
		typename TAlgorithm::node_factory_policy,
		typename TAlgorithm::open_list_policy,
		typename TAlgorithm::visit_track_policy
	>;
}

namespace sl::graph
{
	template <template <concepts::vertex> class TAlgorithmDef, concepts::graph TGraph>
	[[nodiscard]]
	constexpr detail::searcher_type<TAlgorithmDef<graph_vertex_type<TGraph>>, TGraph> make_searcher(
		const TGraph& graph,
		const graph_vertex_type<TGraph>& begin
	)
	{
		return {graph, begin};
	}

	template <template <concepts::vertex, concepts::weight> class TAlgorithmDef, concepts::weighted_graph TGraph>
	[[nodiscard]]
	constexpr detail::searcher_type<TAlgorithmDef<graph_vertex_type<TGraph>, graph_weight_type<TGraph>>, TGraph> make_searcher(
		const TGraph& graph,
		const graph_vertex_type<TGraph>& begin
	)
	{
		return {graph, begin};
	}
}

#endif
