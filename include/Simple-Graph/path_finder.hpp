//           Copyright Dominic Koepke 2022 - 2022.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)

#ifndef SIMPLE_GRAPH_PATH_FINDER_HPP
#define SIMPLE_GRAPH_PATH_FINDER_HPP

#include "utility.hpp"

#include <concepts>
#include <vector>
#include <map>
#include <optional>

namespace sl::graph
{
	template <class T, class TVertex>
	concept predecessor_map_for = requires(T& container, TVertex v)
	{
		{ container[v] } -> std::assignable_from<std::optional<TVertex>>;
		static_cast<bool>(container[v]);
	};
}

namespace sl::graph::detail
{
	template <
		class TNode,
		std::predicate<TNode> TDestinationPredicate,
		std::output_iterator<node_vertex_t<TNode>> TOutItr,
		predecessor_map_for<node_vertex_t<TNode>> TPredecessorMap>
	struct path_finder_t
	{
		TDestinationPredicate destinationPredicate{};
		TOutItr outItr{};
		TPredecessorMap predecessors{};
		bool success{ false };

		[[nodiscard]]
		bool operator ()(const TNode& node)
			requires requires
			{
				node.vertex;
				node.predecessor;
				{ std::invoke(destinationPredicate, node) } -> std::convertible_to<bool>;
				predecessors[node.vertex] = node.predecessor;
			}
		{
			predecessors[node.vertex] = node.predecessor;

			if (std::invoke(destinationPredicate, node))
			{
				success = true;
				*outItr = node.vertex;
				++outItr;
				for
				(
					auto predecessor = predecessors[node.vertex];
					predecessor;
					predecessor = predecessors[*predecessor], ++outItr
				)
				{
					*outItr = *predecessor;
				}
				return true;
			}
			return false;
		}
	};

	template <class TNode>
	[[nodiscard]]
	std::optional<std::vector<node_vertex_t<TNode>>> extract_path
	(
		auto callback,
		predecessor_map_for<node_vertex_t<TNode>> auto predecessorMap,
		auto find_path_callback
	)
	{
		std::vector<node_vertex_t<TNode>> path{};
		auto pathFinder = make_path_finder<TNode>(std::ref(callback), std::back_inserter(path), std::move(predecessorMap));

		std::invoke(find_path_callback, std::ref(pathFinder));

		if (pathFinder.success)
		{
			return { std::move(path) };
		}
		return std::nullopt;
	}
}

namespace sl::graph
{
	template <
		class TNode,
		std::predicate<TNode> TDestinationPredicate,
		std::output_iterator<node_vertex_t<TNode>> TOutItr,
		predecessor_map_for<node_vertex_t<TNode>> TPredecessorMap = std::map<node_vertex_t<TNode>, std::optional<node_vertex_t<TNode>>>>
	[[nodiscard]]
	constexpr auto make_path_finder
	(
		TDestinationPredicate destinationPredicate,
		TOutItr outItr,
		TPredecessorMap predecessors = {}
	)
	{
		return detail::path_finder_t<TNode, TDestinationPredicate, TOutItr, TPredecessorMap>
		{
			std::move(destinationPredicate),
			std::move(outItr),
			std::move(predecessors)
		};
	}
}

#endif
