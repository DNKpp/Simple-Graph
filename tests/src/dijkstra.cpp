//          Copyright Dominic Koepke 2019 - 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

#include "Simple-Graph/dijkstra.hpp"

#include "helper.hpp"

using namespace sl::graph;

constexpr std::array<std::array<int, 3>, 4> default_grid
{
	{
		{ 1, 1, 1 },
		{ 1, 1, 1 },
		{ 1, 1, 1 },
		{ 1, 1, 1 }
	}
};

using state_map = std::map<vertex, dijkstra::state_t<int>, vertex_less>;

TEST_CASE("dijkstra should visit all nodes if not interrupted.", "[dijkstra]")
{
	const std::vector<vertex> expectedVertices
	{
		{ 0, 0 },
		{ 0, 1 },
		{ 0, 2 },
		{ 0, 3 },
		{ 1, 0 },
		{ 1, 1 },
		{ 1, 2 },
		{ 1, 3 },
		{ 2, 0 },
		{ 2, 1 },
		{ 2, 2 },
		{ 2, 3 },
	};

	std::vector<vertex> visitedVertices{};

	dijkstra::traverse
	(
		vertex{ 0, 0 },
		grid_4way_neighbor_searcher{ .grid = &default_grid },
		[](const auto&...) { return 1; },
		[&](const auto& v) { visitedVertices.emplace_back(v.vertex); },
		true_constant{},
		state_map{}
	);

	REQUIRE_THAT(visitedVertices, Catch::Matchers::UnorderedEquals(expectedVertices));
}

TEST_CASE("dijkstra should calculate all weights.", "[dijkstra]")
{
	constexpr std::array<std::array<int, 3>, 4> grid
	{
		{
			{ 1, 2, 1 },
			{ 1, 1, 1 },
			{ 1, 4, 1 },
			{ 1, 2, 1 }
		}
	};

	const std::map<vertex, int, vertex_less> expectedCosts
	{
		{ { 0, 0 }, 2 },
		{ { 0, 1 }, 1 },
		{ { 0, 2 }, 2 },
		{ { 0, 3 }, 3 },
		{ { 1, 0 }, 2 },
		{ { 1, 1 }, 0 },
		{ { 1, 2 }, 4 },
		{ { 1, 3 }, 5 },
		{ { 2, 0 }, 2 },
		{ { 2, 1 }, 1 },
		{ { 2, 2 }, 2 },
		{ { 2, 3 }, 3 },
	};

	dijkstra::traverse
	(
		vertex{ 1, 1 },
		grid_4way_neighbor_searcher{ .grid = &grid },
		grid_weight_extractor{ .grid = &grid },
		[&](const auto& node)
		{
			REQUIRE(expectedCosts.at(node.vertex) == node.weight_sum);
		},
		true_constant{},
		state_map{}
	);
}

TEST_CASE("dijkstra should exit early, if callback returns true.", "[dijkstra]")
{
	constexpr vertex goal{ 1, 2 };

	std::optional<vertex> lastVisited{};
	dijkstra::traverse
	(
		vertex{ 1, 1 },
		grid_4way_neighbor_searcher{ .grid = &default_grid },
		grid_weight_extractor{ .grid = &default_grid },
		[&](const auto& node)
		{
			lastVisited = goal;
			return node.vertex == goal;
		},
		true_constant{},
		state_map{}
	);

	REQUIRE(lastVisited == goal);
}

TEST_CASE("dijkstra should never visit vertices, for which predicate returns false.", "[dijkstra]")
{
	constexpr vertex skip{ 1, 2 };

	dijkstra::traverse
	(
		vertex{ 1, 1 },
		grid_4way_neighbor_searcher{ .grid = &default_grid },
		grid_weight_extractor{ .grid = &default_grid },
		[&](const auto& node) { REQUIRE(node.vertex != skip); },
		[&](const auto& predecessor, const vertex& current) { return current != skip; },
		state_map{}
	);
}
