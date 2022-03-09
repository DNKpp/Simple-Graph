//          Copyright Dominic Koepke 2019 - 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SIMPLE_GRAPH_TESTS_HELPER_HPP
#define SIMPLE_GRAPH_TESTS_HELPER_HPP

#pragma once

#include "Simple-Vector/Vector.hpp"

#include <array>
#include <vector>
#include <ranges>
#include <map>

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

using state_map_2d = std::map<vertex, bool, vertex_less>;

// prefers up > left > down > right
template <class TGrid>
struct grid_4way_neighbor_searcher
{
	const TGrid* grid{};

	std::vector<vertex> operator ()(const vertex& v) const
	{
		std::vector<vertex> neighbors{};
		if (v.x() < std::ssize((*grid)[0]) - 1)
			neighbors.emplace_back(v + vertex{ 1, 0 });
		if (v.y() < std::ssize(*grid) - 1)
			neighbors.emplace_back(v + vertex{ 0, 1 });
		if (0 < v.x())
			neighbors.emplace_back(v - vertex{ 1, 0 });
		if (0 < v.y())
			neighbors.emplace_back(v - vertex{ 0, 1 });
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
