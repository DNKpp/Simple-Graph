//          Copyright Dominic Koepke 2019 - 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

#include "Simple-Graph/astar.hpp"
#include "Simple-Graph/dijkstra.hpp"

#include "helper.hpp"

#include <set>

using namespace sl::graph;

constexpr grid2d<int, 3, 4> default_grid
{
	{
		{ 1, 1, 1 },
		{ 1, 1, 1 },
		{ 1, 1, 1 },
		{ 1, 1, 1 }
	}
};

using state_map_t = std::map<vertex, dijkstra::state_t<int>, vertex_less>;
template <class TNode>
using open_list_t = std::priority_queue<TNode, std::vector<TNode>, std::greater<>>;

TEST_CASE("dynamic_cost_traverse should visit all nodes if not interrupted.", "[traverse]")
{
	using node_t = weighted_node<vertex, int>;

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

	detail::dynamic_cost_traverse<node_t>
	(
		detail::make_weighted_node_factory<vertex>(constant_t<1>{}),
		node_t{ {}, { 0, 0 }, {} },
		grid_4way_neighbor_searcher{ .grid = &default_grid },
		[&](const auto& v) { visitedVertices.emplace_back(v.vertex); },
		true_constant_t{},
		state_map_t{},
		open_list_t<node_t>{}
	);

	REQUIRE_THAT(visitedVertices, Catch::Matchers::UnorderedEquals(expectedVertices));
}

TEST_CASE("dynamic_cost_traverse should prefer cheaper routes over already known, but not yet finalized, routes.", "[traverse]")
{
	using node_t = weighted_node<vertex, int>;

	constexpr grid2d<int, 3, 4> grid
	{
		{
			{ 99, 99, 99 },
			{ 99, 1, 1 },
			{ 99, 99, 3 },
			{ 99, 99, 99 }
		}
	};

	std::set<vertex, vertex_less> visitedVertices{};

	const auto weightCalc = [&](const vertex& predecessor, const vertex& current)
	{
		// we use this to force a better route to 2/2, thus that vertex will be added twice to the open list
		if (predecessor == vertex{ 2, 1 } && current == vertex{ 2, 2 })
			return 1;
		return grid_weight_extractor{ .grid = &grid }(predecessor, current);
	};

	detail::dynamic_cost_traverse<node_t>
	(
		detail::make_weighted_node_factory<vertex>(weightCalc),
		node_t{ {}, { 1, 1 }, {} },
		grid_8way_neighbor_searcher{ .grid = &grid },
		[&](const auto& node)
		{
			REQUIRE(visitedVertices.emplace(node.vertex).second);
		},
		true_constant_t{},
		state_map_t{},
		open_list_t<node_t>{}
	);
}

TEST_CASE("dynamic_cost_traverse should accumulate weights.", "[traverse]")
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

	detail::dynamic_cost_traverse<node_t>
	(
		detail::make_weighted_node_factory<vertex>(grid_weight_extractor{ &grid }),
		node_t{ {}, { 1, 1 }, {} },
		grid_4way_neighbor_searcher{ .grid = &grid },
		[&](const auto& node)
		{
			REQUIRE(expectedCosts.at(node.vertex) == node.weight_sum);
		},
		true_constant_t{},
		state_map_t{},
		open_list_t<node_t>{}
	);
}

TEST_CASE("dynamic_cost_traverse should exit early, if callback returns true.", "[traverse]")
{
	using node_t = weighted_node<vertex, int>;
	constexpr vertex goal{ 1, 2 };

	std::optional<vertex> lastVisited{};
	detail::dynamic_cost_traverse<node_t>
	(
		detail::make_weighted_node_factory<vertex>(grid_weight_extractor{ &default_grid }),
		node_t{ {}, { 1, 1 }, {} },
		grid_4way_neighbor_searcher{ .grid = &default_grid },
		[&](const auto& node)
		{
			lastVisited = node.vertex;
			return node.vertex == goal;
		},
		true_constant_t{},
		state_map_t{},
		open_list_t<node_t>{}
	);

	REQUIRE(lastVisited == goal);
}

TEST_CASE("dynamic_cost_traverse should never visit vertices, for which predicate returns false.", "[traverse]")
{
	using node_t = weighted_node<vertex, int>;
	constexpr vertex skip{ 1, 2 };

	detail::dynamic_cost_traverse<node_t>
	(
		detail::make_weighted_node_factory<vertex>(grid_weight_extractor{ &default_grid }),
		node_t{ {}, { 1, 1 }, {} },
		grid_4way_neighbor_searcher{ .grid = &default_grid },
		[&](const auto& node) { REQUIRE(node.vertex != skip); },
		[&](const auto& predecessor, const vertex& current) { return current != skip; },
		state_map_t{},
		open_list_t<node_t>{}
	);
}

TEST_CASE("dijkstra should correctly expose its typedefs.", "[dijkstra]")
{
	constexpr grid2d<int, 3, 4> grid{};

	const dijkstra::searcher searcher
	{
		.begin = vertex{ 0, 1 },
		.neighborSearcher = grid_4way_neighbor_searcher{ &grid },
		.weightCalculator = grid_weight_extractor{ &grid },
		.callback = empty_invokable_t{},
		.vertexPredicate = true_constant_t{},
		.stateMap = state_map_t{},
		.openList = dijkstra::default_open_list_t<vertex, int>{}
	};
	using searcher_t = decltype(searcher);

	REQUIRE(std::same_as<searcher_t::vertex_t, vertex>);
	REQUIRE(std::same_as<searcher_t::weight_t, int>);
	REQUIRE(std::same_as<searcher_t::node_t, weighted_node<vertex, int>>);

	REQUIRE(std::same_as<searcher_t::neighbor_searcher_t, grid_4way_neighbor_searcher<grid2d<int, 3, 4>>>);
	REQUIRE(std::same_as<searcher_t::weight_calculator_t, grid_weight_extractor<grid2d<int, 3, 4>>>);
	REQUIRE(std::same_as<searcher_t::callback_t, empty_invokable_t>);
	REQUIRE(std::same_as<searcher_t::vertex_predicate_t, true_constant_t>);
	REQUIRE(std::same_as<searcher_t::state_map_t, state_map_t>);
	REQUIRE(std::same_as<searcher_t::open_list_t, dijkstra::default_open_list_t<vertex, int>>);
}

TEST_CASE("Test dijkstra traverse compiling with as much default params as possible.", "[dijkstra]")
{
	constexpr int begin{ 3 };
	constexpr int end{ 9 };

	const dijkstra::searcher searcher
	{
		.begin = 5,
		.neighborSearcher = linear_graph_neighbor_searcher{ .begin = &begin, .end = &end },
		.weightCalculator = [](int predecessor, int current) { return current; }
	};

	traverse(searcher);
}

TEST_CASE("astar should correctly expose its typedefs.", "[astar]")
{
	constexpr grid2d<int, 3, 4> grid{};

	const astar::searcher searcher
	{
		.begin = vertex{ 0, 1 },
		.neighborSearcher = grid_4way_neighbor_searcher{ &grid },
		.weightCalculator = grid_weight_extractor{ &grid },
		.heuristic = constant_t<0>{},
		.callback = empty_invokable_t{},
		.vertexPredicate = true_constant_t{},
		.stateMap = state_map_t{},
		.openList = astar::default_open_list_t<vertex, int>{}
	};
	using searcher_t = decltype(searcher);

	REQUIRE(std::same_as<searcher_t::vertex_t, vertex>);
	REQUIRE(std::same_as<searcher_t::weight_t, int>);
	REQUIRE(std::same_as<searcher_t::node_t, astar::node_t<vertex, int>>);

	REQUIRE(std::same_as<searcher_t::neighbor_searcher_t, grid_4way_neighbor_searcher<grid2d<int, 3, 4>>>);
	REQUIRE(std::same_as<searcher_t::weight_calculator_t, grid_weight_extractor<grid2d<int, 3, 4>>>);
	REQUIRE(std::same_as<searcher_t::heuristic_t, constant_t<0>>);
	REQUIRE(std::same_as<searcher_t::callback_t, empty_invokable_t>);
	REQUIRE(std::same_as<searcher_t::vertex_predicate_t, true_constant_t>);
	REQUIRE(std::same_as<searcher_t::state_map_t, state_map_t>);
	REQUIRE(std::same_as<searcher_t::open_list_t, astar::default_open_list_t<vertex, int>>);
}

TEST_CASE("Test astar traverse compiling with as much default params as possible.", "[astar]")
{
	constexpr int begin{ 3 };
	constexpr int end{ 9 };

	const astar::searcher searcher
	{
		.begin = 5,
		.neighborSearcher = linear_graph_neighbor_searcher{ .begin = &begin, .end = &end },
		.weightCalculator = [](int predecessor, int current) { return current; },
		.heuristic = [](const int) { return 0; }
	};

	traverse(searcher);
}

TEST_CASE("astar should prefer vertices with less estimated weight.", "[astar]")
{
	constexpr int begin{ 3 };
	constexpr int end{ 9 };

	const std::set expectedVertices{ 5, 6, 7, 8 };

	const astar::searcher searcher
	{
		.begin = 5,
		.neighborSearcher = linear_graph_neighbor_searcher{ .begin = &begin, .end = &end },
		.weightCalculator = constant_t<1>{},
		.heuristic = [](const int v) { return 8 - v; },
		.callback = [&](const auto& node)
		{
			REQUIRE(expectedVertices.count(node.vertex) == 1);
			return node.vertex == 8;
		}
	};

	traverse(searcher);
}
