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

TEST_CASE("dynamic_cost_traverse should visit all nodes if not interrupted.", "[dijkstra]")
{
	using node_t = weighted_node<vertex, int>;
	constexpr auto weightCalc = [](const vertex&, const vertex&) { return 1; };

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
		detail::weighted_node_factory_t<vertex, decltype(weightCalc)>{ weightCalc },
		node_t{ {}, { 0, 0 }, {} },
		grid_4way_neighbor_searcher{ .grid = &default_grid },
		[&](const auto& v) { visitedVertices.emplace_back(v.vertex); },
		true_constant{},
		state_map_t{},
		open_list_t<node_t>{}
	);

	REQUIRE_THAT(visitedVertices, Catch::Matchers::UnorderedEquals(expectedVertices));
}

TEST_CASE("dynamic_cost_traverse should prefer cheaper routes over already known, but not yet finalized, routes.", "[dijkstra]")
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
		detail::weighted_node_factory_t<vertex, decltype(weightCalc)>{ weightCalc },
		node_t{ {}, { 1, 1 }, {} },
		grid_8way_neighbor_searcher{ .grid = &grid },
		[&](const auto& node)
		{
			REQUIRE(visitedVertices.emplace(node.vertex).second);
		},
		true_constant{},
		state_map_t{},
		open_list_t<node_t>{}
	);
}

TEST_CASE("dynamic_cost_traverse should accumulate weights.", "[dijkstra]")
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
		detail::weighted_node_factory_t<vertex, grid_weight_extractor<grid2d<int, 3, 4>>>{ { &grid } },
		node_t{ {}, { 1, 1 }, {} },
		grid_4way_neighbor_searcher{ .grid = &grid },
		[&](const auto& node)
		{
			REQUIRE(expectedCosts.at(node.vertex) == node.weight_sum);
		},
		true_constant{},
		state_map_t{},
		open_list_t<node_t>{}
	);
}

TEST_CASE("dynamic_cost_traverse should exit early, if callback returns true.", "[dijkstra]")
{
	using node_t = weighted_node<vertex, int>;
	constexpr vertex goal{ 1, 2 };

	std::optional<vertex> lastVisited{};
	detail::dynamic_cost_traverse<node_t>
	(
		detail::weighted_node_factory_t<vertex, grid_weight_extractor<grid2d<int, 3, 4>>>{ { &default_grid } },
		node_t{ {}, { 1, 1 }, {} },
		grid_4way_neighbor_searcher{ .grid = &default_grid },
		[&](const auto& node)
		{
			lastVisited = goal;
			return node.vertex == goal;
		},
		true_constant{},
		state_map_t{},
		open_list_t<node_t>{}
	);

	REQUIRE(lastVisited == goal);
}

TEST_CASE("dynamic_cost_traverse should never visit vertices, for which predicate returns false.", "[dijkstra]")
{
	using node_t = weighted_node<vertex, int>;
	constexpr vertex skip{ 1, 2 };

	detail::dynamic_cost_traverse<node_t>
	(
		detail::weighted_node_factory_t<vertex, grid_weight_extractor<grid2d<int, 3, 4>>>{ { &default_grid } },
		node_t{ {}, { 1, 1 }, {} },
		grid_4way_neighbor_searcher{ .grid = &default_grid },
		[&](const auto& node) { REQUIRE(node.vertex != skip); },
		[&](const auto& predecessor, const vertex& current) { return current != skip; },
		state_map_t{},
		open_list_t<node_t>{}
	);
}

TEST_CASE("Test dijkstra traverse compiling with as much default params as possible.", "[dijkstra]")
{
	constexpr int begin{ 3 };
	constexpr int end{ 9 };

	dijkstra::traverse
	(
		5,
		linear_graph_neighbor_searcher{ .begin = &begin, .end = &end },
		[](int predecessor, int current) { return current; }
	);
}

TEST_CASE("Test astar traverse compiling with as much default params as possible.", "[dijkstra]")
{
	constexpr int begin{ 3 };
	constexpr int end{ 9 };

	astar::traverse
	(
		5,
		linear_graph_neighbor_searcher{ .begin = &begin, .end = &end },
		[](int predecessor, int current) { return current; },
		[](const int) { return 0; }
	);
}

TEST_CASE("astar should prefer vertices with less estimated weight.", "[dijkstra]")
{
	constexpr int begin{ 3 };
	constexpr int end{ 9 };

	const std::set expectedVertices{ 5, 6, 7, 8 };

	astar::traverse
	(
		5,
		linear_graph_neighbor_searcher{ .begin = &begin, .end = &end },
		[](int predecessor, int current) { return 1; },
		[](const int v) { return 8 - v; },
		[&](const auto& node)
		{
			REQUIRE(expectedVertices.count(node.vertex) == 1);
			return node.vertex == 8;
		}
	);
}
