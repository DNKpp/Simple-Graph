//          Copyright Dominic Koepke 2019 - 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SIMPLE_GRAPH_NODE_HPP
#define SIMPLE_GRAPH_NODE_HPP

#pragma once

#include "Simple-Graph/graph.hpp"

#include <optional>

namespace sl::graph
{
	template <class T>
	struct node_traits
		: detail::try_extract_weight_type<T>
	{
		using vertex_type = typename T::vertex_type;
	};

	template <class T>
	struct node_after;
}

namespace sl::graph::concepts
{
	template <class T>
	concept node = requires { typename node_traits<T>::vertex_type; }
					&& vertex<typename node_traits<T>::vertex_type>
					&& requires(const T& n)
					{
						{ n.vertex } -> std::convertible_to<typename node_traits<T>::vertex_type>;
						static_cast<bool>(n.predecessor);
						{ *n.predecessor } -> std::convertible_to<typename node_traits<T>::vertex_type>;
					};

	template <class T>
	concept weighted_node = node<T>
							&& requires { typename node_traits<T>::weight_type; }
							&& weight<typename node_traits<T>::weight_type>
							&& requires(const T& n)
							{
								{ n.accumulatedWeight } -> std::convertible_to<typename node_traits<T>::weight_type>;
							}
							&& std::predicate<node_after<T>, T, T>;
}

namespace sl::graph
{
	template <concepts::node T>
	using node_vertex_type = typename node_traits<T>::vertex_type;

	template <concepts::weighted_node T>
	using node_weight_type = typename node_traits<T>::weight_type;

	template <concepts::vertex TVertex>
	struct node
	{
		using vertex_type = TVertex;

		TVertex vertex{};
		std::optional<TVertex> predecessor{};

		[[nodiscard]]
		bool operator ==(const node&) const = default;
	};

	template <concepts::vertex TVertex, concepts::weight TWeight>
	struct weighted_node
		: node<TVertex>
	{
		using weight_type = TWeight;

		TWeight accumulatedWeight{};

		[[nodiscard]]
		bool operator ==(const weighted_node&) const = default;
	};

	template <concepts::vertex TVertex, concepts::weight TWeight>
	struct node_after<weighted_node<TVertex, TWeight>>
	{
		[[nodiscard]]
		constexpr bool operator ()(const weighted_node<TVertex, TWeight>& lhs, const weighted_node<TVertex, TWeight>& rhs) const
		{
			return lhs.accumulatedWeight > rhs.accumulatedWeight;
		}
	};
}

#endif
