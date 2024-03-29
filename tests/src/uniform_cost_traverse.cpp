//           Copyright Dominic Koepke 2019 - 2022.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

#include "Simple-Graph/breadth_first_search.hpp"
#include "Simple-Graph/depth_first_search.hpp"
#include "Simple-Graph/generic_traverse.hpp"

#include "helper.hpp"

using namespace sl::graph;

TEST_CASE("uniform_cost_traverse should visit all vertices of a given linear graph (order doesn't matter).", "[traverse]")
{
	using node_t = weighted_node<int, int>;

	constexpr int begin{ 3 };
	constexpr int end{ 9 };

	std::vector<int> visitedVertices{};

	detail::uniform_cost_traverse<node_t>
	(
		detail::make_weighted_node_factory<int>(constant_t<1>{}),
		{ .vertex = 5 },
		linear_graph_neighbor_searcher{ .begin = &begin, .end = &end },
		[&](const auto& v) { visitedVertices.emplace_back(v.vertex); },
		true_constant_t{},
		std::map<int, bool>{},
		std::stack<node_t>{}
	);

	REQUIRE_THAT(visitedVertices, Catch::Matchers::UnorderedEquals(std::vector{ 5, 6, 7, 8, 4, 3 }));
}

TEST_CASE("uniform_cost_traverse should visit all vertices of a given grid.", "[traverse]")
{
	using node_t = weighted_node<vertex, int>;

	constexpr grid2d<int, 3, 4> grid{};

	int invokeCounter = 0;

	detail::uniform_cost_traverse<node_t>
	(
		detail::make_weighted_node_factory<vertex>(constant_t<1>{}),
		{ .vertex = { 1, 1 } },
		grid_4way_neighbor_searcher{ &grid },
		[&invokeCounter](const auto&) { ++invokeCounter; },
		true_constant_t{},
		state_map_2d{},
		std::stack<node_t>{}
	);

	REQUIRE(invokeCounter == 3 * 4);
}

TEST_CASE("uniform_cost_traverse should skip vertices for which predicate returns false (order doesn't matter).", "[traverse]")
{
	using node_t = weighted_node<int, int>;

	constexpr int begin{ 3 };
	constexpr int end{ 9 };

	std::vector<int> visitedVertices{};

	detail::uniform_cost_traverse<node_t>
	(
		detail::make_weighted_node_factory<int>(constant_t<1>{}),
		{ .vertex = 5 },
		linear_graph_neighbor_searcher{ .begin = &begin, .end = &end },
		[&](const auto& v) { visitedVertices.emplace_back(v.vertex); },
		[](const auto& predecessor, const int& v) { return v != 7; },
		std::map<int, bool>{},
		std::stack<node_t>{}
	);

	REQUIRE_THAT(visitedVertices, Catch::Matchers::UnorderedEquals(std::vector{ 5, 6, 4, 3 }));
}

TEST_CASE("uniform_cost_traverse should exit early, if callback returns true.", "[traverse]")
{
	using node_t = weighted_node<int, int>;

	constexpr int begin{ 3 };
	constexpr int end{ 9 };
	constexpr int goal{ 7 };

	std::optional<int> lastVisited{};
	detail::uniform_cost_traverse<node_t>
	(
		detail::make_weighted_node_factory<int>(constant_t<1>{}),
		{ .vertex = 5 },
		linear_graph_neighbor_searcher{ .begin = &begin, .end = &end },
		[&](const auto& node)
		{
			lastVisited = node.vertex;
			return node.vertex == goal;
		},
		true_constant_t{},
		std::map<int, bool>{},
		std::stack<node_t>{}
	);

	REQUIRE(lastVisited == goal);
}

TEST_CASE("dfs should correctly expose its typedefs.", "[dfs]")
{
	constexpr grid2d<int, 3, 4> grid{};

	const dfs::search_params searcher
	{
		.begin = vertex{ 0, 1 },
		.neighborSearcher = grid_4way_neighbor_searcher{ &grid },
		.callback = empty_invokable_t{},
		.vertexPredicate = true_constant_t{},
		.stateMap = state_map_2d{},
		.openList = dfs::default_open_list_t<vertex>{}
	};
	using searcher_t = decltype(searcher);

	REQUIRE(std::same_as<searcher_t::vertex_t, vertex>);
	REQUIRE(std::same_as<searcher_t::weight_t, int>);
	REQUIRE(std::same_as<searcher_t::node_t, weighted_node<vertex, int>>);

	REQUIRE(std::same_as<searcher_t::neighbor_searcher_t, grid_4way_neighbor_searcher<grid2d<int, 3, 4>>>);
	REQUIRE(std::same_as<searcher_t::callback_t, empty_invokable_t>);
	REQUIRE(std::same_as<searcher_t::vertex_predicate_t, true_constant_t>);
	REQUIRE(std::same_as<searcher_t::state_map_t, state_map_2d>);
	REQUIRE(std::same_as<searcher_t::open_list_t, dfs::default_open_list_t<vertex>>);
}

TEST_CASE("Test dfs traverse compiling with as much default params as possible.", "[dfs]")
{
	constexpr int begin{ 3 };
	constexpr int end{ 9 };

	traverse
	(
		dfs::search_params
		{
			.begin = 5,
			.neighborSearcher = linear_graph_neighbor_searcher{ .begin = &begin, .end = &end }
		}
	);
}

TEST_CASE("dfs should visit all vertices in a specific order.", "[dfs][traverse]")
{
	constexpr grid2d<int, 3, 4> grid{};

	// provided neighbor search_params prefers up > left > down > right
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

	dfs::search_params searcher
	{
		.begin = vertex{ 0, 1 },
		.neighborSearcher = grid_4way_neighbor_searcher{ &grid },
		.callback = [&](const auto& v) { depths.emplace_back(v.vertex, v.weight_sum); },
		.vertexPredicate = true_constant_t{},
		.stateMap = state_map_2d{}
	};
	traverse(std::move(searcher));

	REQUIRE_THAT(depths, Catch::Matchers::UnorderedEquals(expected_depths));
}

TEST_CASE("dfs find_path should return reversed path as vector if exists.", "[dfs][find_path]")
{
	using node_t = weighted_node<vertex, int>;

	constexpr grid2d<int, 3, 4> grid
	{
		{
			{ 1, 2, 1 },
			{ 1, 1, 1 },
			{ 1, 4, 1 },
			{ 1, 2, 1 }
		}
	};

	// provided neighbor search_params prefers up > left > down > right
	const auto [destination, expectedPath] = GENERATE
	(
		table<vertex,
		std::optional<std::vector<vertex>>>({
			{ { 2, 3 }, { { { 2, 3 }, { 1, 3 }, { 0, 3 }, { 0, 2 }, { 0, 1 }, { 1, 1 } } } },
			{ { 3, 3 }, std::nullopt } // out of grid bounds
			})
	);

	const dfs::search_params searcher
	{
		.begin = vertex{ 1, 1 },
		.neighborSearcher = grid_4way_neighbor_searcher{ .grid = &grid },
		.callback = vertex_destination_t{ destination },
		.stateMap = std::map<vertex, dfs::state_t, vertex_less>{}
	};

	const auto path = find_path(searcher, std::map<vertex, std::optional<vertex>, vertex_less>{});

	REQUIRE(path == expectedPath);
}

TEST_CASE("bfs should correctly expose its typedefs.", "[bfs]")
{
	constexpr grid2d<int, 3, 4> grid{};

	const bfs::search_params searcher
	{
		.begin = vertex{ 0, 1 },
		.neighborSearcher = grid_4way_neighbor_searcher{ &grid },
		.callback = empty_invokable_t{},
		.vertexPredicate = true_constant_t{},
		.stateMap = state_map_2d{},
		.openList = bfs::default_open_list_t<vertex>{}
	};
	using searcher_t = decltype(searcher);

	REQUIRE(std::same_as<searcher_t::vertex_t, vertex>);
	REQUIRE(std::same_as<searcher_t::weight_t, int>);
	REQUIRE(std::same_as<searcher_t::node_t, weighted_node<vertex, int>>);

	REQUIRE(std::same_as<searcher_t::neighbor_searcher_t, grid_4way_neighbor_searcher<grid2d<int, 3, 4>>>);
	REQUIRE(std::same_as<searcher_t::callback_t, empty_invokable_t>);
	REQUIRE(std::same_as<searcher_t::vertex_predicate_t, true_constant_t>);
	REQUIRE(std::same_as<searcher_t::state_map_t, state_map_2d>);
	REQUIRE(std::same_as<searcher_t::open_list_t, bfs::default_open_list_t<vertex>>);
}

TEST_CASE("Test bfs traverse compiling with as much default params as possible.", "[bfs]")
{
	constexpr int begin{ 3 };
	constexpr int end{ 9 };

	traverse
	(
		bfs::search_params
		{
			.begin = 5,
			.neighborSearcher = linear_graph_neighbor_searcher{ .begin = &begin, .end = &end }
		}
	);
}

TEST_CASE("bfs should visit all vertices in a specific order.", "[bfs][traverse]")
{
	constexpr grid2d<int, 3, 4> grid{};

	// provided neighbor search_params prefers up > left > down > right
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

	bfs::search_params searcher
	{
		.begin = vertex{ 0, 1 },
		.neighborSearcher = grid_4way_neighbor_searcher{ &grid },
		.callback = [&](const auto& v) { depths.emplace_back(v.vertex, v.weight_sum); },
		.vertexPredicate = true_constant_t{},
		.stateMap = state_map_2d{}
	};
	traverse(std::move(searcher));

	REQUIRE_THAT(depths, Catch::Matchers::UnorderedEquals(expected_depths));
}

TEST_CASE("bfs find_path should return reversed path as vector if exists.", "[bfs][find_path]")
{
	using node_t = weighted_node<vertex, int>;

	constexpr grid2d<int, 3, 4> grid
	{
		{
			{ 1, 2, 1 },
			{ 1, 1, 1 },
			{ 1, 4, 1 },
			{ 1, 2, 1 }
		}
	};

	// provided neighbor search_params prefers up > left > down > right
	const auto [destination, expectedPath] = GENERATE
	(
		table<vertex,
		std::optional<std::vector<vertex>>>({
			{ { 2, 3 }, { { { 2, 3 }, { 2, 2 }, { 2, 1 }, { 1, 1 } } } },
			{ { 3, 3 }, std::nullopt } // out of grid bounds
			})
	);

	const bfs::search_params searcher
	{
		.begin = vertex{ 1, 1 },
		.neighborSearcher = grid_4way_neighbor_searcher{ .grid = &grid },
		.callback = vertex_destination_t{ destination },
		.stateMap = std::map<vertex, bfs::state_t, vertex_less>{}
	};

	const auto path = find_path(searcher, std::map<vertex, std::optional<vertex>, vertex_less>{});

	REQUIRE(path == expectedPath);
}
