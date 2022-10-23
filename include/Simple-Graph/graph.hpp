//          Copyright Dominic Koepke 2019 - 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SIMPLE_GRAPH_GRAPH_HPP
#define SIMPLE_GRAPH_GRAPH_HPP

#pragma once

#include "Simple-Graph/common.hpp"

namespace sl::graph
{
	template <class T>
	struct graph_traits
		: detail::try_extract_weight_type<T>
	{
		using vertex_type = typename T::vertex_type;
	};
}

namespace sl::graph::concepts
{
	template <class T, class TVertex>
	concept neighbor_map_policy_for = std::same_as<T, std::remove_cvref_t<T>>
									&& vertex<TVertex>
									&& requires(const T& policy, const TVertex& v)
									{
										{ policy.neighbors(v) } -> std::ranges::input_range;
										requires std::convertible_to<
											std::ranges::range_value_t<decltype(policy.neighbors(v))>,
											TVertex
										>;
									};

	template <class T, class TVertex, class TWeight>
	concept weight_map_policy_for = std::same_as<T, std::remove_cvref_t<T>>
									&& vertex<TVertex>
									&& weight<TWeight>
									&& requires(const T& policy, const TVertex& v)
									{
										{ policy.weight(v, v) } -> std::convertible_to<TWeight>;
									};

	template <class T>
	concept graph = std::same_as<T, std::remove_cvref_t<T>>
					&& requires { typename graph_traits<T>::vertex_type; }
					&& neighbor_map_policy_for<T, typename graph_traits<T>::vertex_type>;

	template <class T>
	concept weighted_graph = graph<T>
							&& requires { typename graph_traits<T>::weight_type; }
							&& weight_map_policy_for<T, typename graph_traits<T>::vertex_type, typename graph_traits<T>::weight_type>;
}

namespace sl::graph
{
	template <concepts::graph T>
	using graph_vertex_type = typename graph_traits<T>::vertex_type;

	template <concepts::weighted_graph T>
	using graph_weight_type = typename graph_traits<T>::weight_type;

	template <
		concepts::vertex TVertex,
		concepts::neighbor_map_policy_for<TVertex> TNeighborMapPolicy
	>
	struct graph final
		: public TNeighborMapPolicy
	{
		using vertex_type = TVertex;
	};

	template <
		concepts::vertex TVertex,
		concepts::weight TWeight,
		concepts::neighbor_map_policy_for<TVertex> TNeighborMapPolicy,
		concepts::weight_map_policy_for<TVertex, TWeight> TWeightMapPolicy
	>
	struct weighted_graph final
		: public TNeighborMapPolicy,
		public TWeightMapPolicy
	{
		using vertex_type = TVertex;
		using weight_type = TWeight;
	};
}

#endif
