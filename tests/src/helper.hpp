//          Copyright Dominic Koepke 2019 - 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SIMPLE_GRAPH_TESTS_HELPER_HPP
#define SIMPLE_GRAPH_TESTS_HELPER_HPP

#pragma once

#include "Simple-Vector/Vector.hpp"

#include <array>
#include <map>
#include <ranges>
#include <vector>

template <class T, int VWidth, int VHeight>
using grid2d = std::array<std::array<T, VWidth>, VHeight>;
using vertex = sl::vec::Vector<int, 2>;

struct vertex_less
{
	constexpr bool operator ()(const vertex& lhs, const vertex& rhs) const noexcept
	{
		return std::ranges::lexicographical_compare(lhs, rhs);
	}
};

template <class TGrid>
struct within_grid_t
{
	const TGrid* grid{};

	constexpr bool operator ()(const vertex& v) const noexcept
	{
		return 0 <= v.y() && v.y() < std::ssize(*grid)
				&& 0 <= v.x() && v.x() < std::ssize((*grid)[v.y()]);
	}
};

using state_map_2d = std::map<vertex, bool, vertex_less>;

// prefers up > left > down > right
template <class TGrid>
struct grid_4way_neighbor_searcher
{
	const TGrid* grid{};

	std::vector<vertex> operator ()(const vertex& v) const
	{
		constexpr auto directions = std::to_array<vertex>({ { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } });

		std::vector<vertex> neighbors{};
		std::ranges::copy
		(
			directions
			| std::views::transform([&](const vertex& o) { return v + o; })
			| std::views::filter(within_grid_t{ .grid = grid }),
			std::back_inserter(neighbors)
		);
		return neighbors;
	}
};

template <class TGrid>
struct grid_8way_neighbor_searcher
{
	const TGrid* grid{};

	std::vector<vertex> operator ()(const vertex& v) const
	{
		constexpr auto directions = std::to_array<vertex>({ { 1, 1 }, { 1, -1 }, { -1, 1 }, { -1, -1 } });

		std::vector<vertex> neighbors{ grid_4way_neighbor_searcher{ .grid = grid }(v) };
		std::ranges::copy
		(
			directions
			| std::views::transform([&](const vertex& o) { return v + o; })
			| std::views::filter(within_grid_t{ .grid = grid }),
			std::back_inserter(neighbors)
		);
		return neighbors;
	}
};

template <class TGrid>
struct grid_weight_extractor
{
	const TGrid* grid{};

	auto operator ()(const vertex&, const vertex& current) const
	{
		return (*grid)[current.y()][current.x()];
	}
};

struct linear_graph_neighbor_searcher
{
	const int* begin{};
	const int* end{};

	std::vector<int> operator ()(const int& v) const
	{
		std::vector<int> neighbors{};
		if (*begin < v)
			neighbors.emplace_back(v - 1);
		if (v + 1 < *end)
			neighbors.emplace_back(v + 1);
		return neighbors;
	}
};

#endif
