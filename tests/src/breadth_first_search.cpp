//          Copyright Dominic Koepke 2019 - 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

#include "Simple-Graph/breadth_first_search.hpp"

#include "Simple-Vector/Vector.hpp"

#include <array>
#include <vector>

using namespace sl::graph;

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
struct grid_4way_neighbor_searcher
{
	const TGrid* grid{};

	std::vector<vertex> operator ()(const vertex& v) const
	{
		std::vector<vertex> neighbors{};
		if (0 < v.x())
			neighbors.emplace_back(v - vertex{ 1, 0 });
		if (v.x() < std::ssize((*grid)[0]) - 1)
			neighbors.emplace_back(v + vertex{ 1, 0 });
		if (0 < v.y())
			neighbors.emplace_back(v - vertex{ 0, 1 });
		if (v.y() < std::ssize(*grid) - 1)
			neighbors.emplace_back(v + vertex{ 0, 1 });
		return neighbors;
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

TEST_CASE("traverse_bfs should visit all vertices of a given linear graph.", "[bfs]")
{
	constexpr int begin{ 3 };
	constexpr int end{ 9 };

	std::vector<int> visitedVertices{};

	traverse_bfs
	(
		5,
		linear_graph_neighbor_searcher{ .begin = &begin, .end = &end },
		[&](const auto& v) { visitedVertices.emplace_back(v.vertex); }
	);

	REQUIRE(visitedVertices == std::vector{ 5, 4, 6, 3, 7, 8 });
}

TEST_CASE("traverse_bfs should skip vertices for which predicate returns false.", "[bfs]")
{
	constexpr int begin{ 3 };
	constexpr int end{ 9 };

	std::vector<int> visitedVertices{};

	traverse_bfs
	(
		5,
		linear_graph_neighbor_searcher{ .begin = &begin, .end = &end },
		[&](const auto& v) { visitedVertices.emplace_back(v.vertex); },
		[](const auto& node) { return node.vertex != 6; }
	);

	REQUIRE(visitedVertices == std::vector{ 5, 4, 3 });
}

TEST_CASE("traverse_bfs should visit all vertices of a given grid.", "[bfs]")
{
	constexpr grid2d<int, 3, 4> grid{};

	int invokeCounter = 0;

	traverse_bfs
	(
		vertex{ 1, 1 },
		grid_4way_neighbor_searcher{ &grid },
		[&invokeCounter](const auto&) { ++invokeCounter; },
		true_constant{},
		std::map<vertex, bool, vertex_less>{}
	);

	REQUIRE(invokeCounter == 3 * 4);
}
