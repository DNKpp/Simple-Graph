//          Copyright Dominic Koepke 2019 - 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

#include "Simple-Graph/breadth_first_search.hpp"
#include "Simple-Graph/depth_first_search.hpp"
#include "Simple-Graph/generic_traverse.hpp"

#include "helper.hpp"

using namespace sl::graph;

TEST_CASE("uniform_cost_traverse should visit all vertices of a given linear graph (order doesn't matter).", "[traverse]")
{
	constexpr int begin{ 3 };
	constexpr int end{ 9 };

	std::vector<int> visitedVertices{};

	detail::uniform_cost_traverse<weighted_node<int, int>>
	(
		5,
		linear_graph_neighbor_searcher{ .begin = &begin, .end = &end },
		[&](const auto& v) { visitedVertices.emplace_back(v.vertex); },
		true_constant_t{},
		std::map<int, bool>{},
		std::stack<weighted_node<int, int>>{}
	);

	REQUIRE_THAT(visitedVertices, Catch::Matchers::UnorderedEquals(std::vector{ 5, 6, 7, 8, 4, 3 }));
}

TEST_CASE("uniform_cost_traverse should visit all vertices of a given grid.", "[traverse]")
{
	constexpr grid2d<int, 3, 4> grid{};

	int invokeCounter = 0;

	detail::uniform_cost_traverse<weighted_node<vertex, int>>
	(
		vertex{ 1, 1 },
		grid_4way_neighbor_searcher{ &grid },
		[&invokeCounter](const auto&) { ++invokeCounter; },
		true_constant_t{},
		state_map_2d{},
		std::stack<weighted_node<vertex, int>>{}
	);

	REQUIRE(invokeCounter == 3 * 4);
}

TEST_CASE("uniform_cost_traverse should skip vertices for which predicate returns false (order doesn't matter).", "[traverse]")
{
	constexpr int begin{ 3 };
	constexpr int end{ 9 };

	std::vector<int> visitedVertices{};

	detail::uniform_cost_traverse<weighted_node<int, int>>
	(
		5,
		linear_graph_neighbor_searcher{ .begin = &begin, .end = &end },
		[&](const auto& v) { visitedVertices.emplace_back(v.vertex); },
		[](const auto& predecessor, const int& v) { return v != 7; },
		std::map<int, bool>{},
		std::stack<weighted_node<int, int>>{}
	);

	REQUIRE_THAT(visitedVertices, Catch::Matchers::UnorderedEquals(std::vector{ 5, 6, 4, 3 }));
}

TEST_CASE("uniform_cost_traverse should exit early, if callback returns true.", "[traverse]")
{
	constexpr int begin{ 3 };
	constexpr int end{ 9 };
	constexpr int goal{ 7 };

	std::optional<int> lastVisited{};
	detail::uniform_cost_traverse<weighted_node<int, int>>
	(
		5,
		linear_graph_neighbor_searcher{ .begin = &begin, .end = &end },
		[&](const auto& node)
		{
			lastVisited = node.vertex;
			return node.vertex == goal;
		},
		true_constant_t{},
		std::map<int, bool>{},
		std::stack<weighted_node<int, int>>{}
	);

	REQUIRE(lastVisited == goal);
}

TEST_CASE("traverse_dfs should visit all vertices in a specific order.", "[traverse]")
{
	constexpr grid2d<int, 3, 4> grid{};

	// provided neighbor searcher prefers up > left > down > right
	const std::vector<std::tuple<vertex, int>> expected_depths{
		{ { 0, 0 }, 1 },
		{ { 1, 0 }, 2 },
		{ { 2, 0 }, 3 },

		{ { 0, 1 }, 0 },
		{ { 1, 1 }, 1 },
		{ { 2, 1 }, 4 },

		{ { 0, 2 }, 1 },
		{ { 1, 2 }, 6 },
		{ { 2, 2 }, 5 },

		{ { 0, 3 }, 8 },
		{ { 1, 3 }, 7 },
		{ { 2, 3 }, 6 }
	};

	std::vector<std::tuple<vertex, int>> depths{};

	dfs::traverse
	(
		vertex{ 0, 1 },
		grid_4way_neighbor_searcher{ &grid },
		[&](const auto& v) { depths.emplace_back(v.vertex, v.weight_sum); },
		true_constant_t{},
		state_map_2d{}
	);

	REQUIRE_THAT(depths, Catch::Matchers::UnorderedEquals(expected_depths));
}

TEST_CASE("traverse_bfs should visit all vertices in a specific order.", "[traverse]")
{
	constexpr grid2d<int, 3, 4> grid{};

	// provided neighbor searcher prefers up > left > down > right
	const std::vector<std::tuple<vertex, int>> expected_depths{
		{ { 0, 0 }, 1 },
		{ { 1, 0 }, 2 },
		{ { 2, 0 }, 3 },

		{ { 0, 1 }, 0 },
		{ { 1, 1 }, 1 },
		{ { 2, 1 }, 2 },

		{ { 0, 2 }, 1 },
		{ { 1, 2 }, 2 },
		{ { 2, 2 }, 3 },

		{ { 0, 3 }, 2 },
		{ { 1, 3 }, 3 },
		{ { 2, 3 }, 4 }
	};

	std::vector<std::tuple<vertex, int>> depths{};

	bfs::traverse
	(
		vertex{ 0, 1 },
		grid_4way_neighbor_searcher{ &grid },
		[&](const auto& v) { depths.emplace_back(v.vertex, v.weight_sum); },
		true_constant_t{},
		state_map_2d{}
	);

	REQUIRE_THAT(depths, Catch::Matchers::UnorderedEquals(expected_depths));
}
