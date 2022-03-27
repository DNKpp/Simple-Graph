//           Copyright Dominic Koepke 2022 - 2022.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)

#include <Simple-Graph/astar.hpp>

#include <array>
#include <iostream>
#include <string>
#include <vector>

/*
 * In this example I'll show the minimal setup performing the astar algorithm on a given 2d grid. Most of the following code
 * is simply necessary to actually have something to run the algorithm on, thus setting up some classes and an actual grid.
 * As the astar is the "heaviest" algorithm, it has the requirements on the setup, thus if one picks a dijkstra or even a
 * depth-first-search, some of the requirements will not be necessary and therefore the setup smaller.
 */

// simple 2d grid; the actual values will be used as cost to step onto
template <int VHeight, int VWidth>
using grid2d = std::array<std::array<int, VWidth>, VHeight>;

// a simple vector 2d type. The library expects the vertex types to be at least equality-comparable.
// As we are going to use the default state map, which is an std::map, our type has to be also
// less-comparable.
struct vector2d
{
	int x{};
	int y{};

	constexpr auto operator<=>(const vector2d& other) const = default;
};

// simple helper function for printing our vector2d objects.
std::ostream& operator <<(std::ostream& out, const vector2d& vec)
{
	out << vec.x << "/" << vec.y;
	return out;
}

// convenience alias, which we can use in our following definitions
using astar_node_t = sl::graph::astar::node_t<vector2d, int>;

// this searcher simply returns all 4 way adjacent vertices to the current vertex, when they are within the grid.
template <int VHeight, int VWidth>
struct neighbor_searcher_2d
{
	constexpr std::vector<vector2d> operator()(const vector2d& current) const
	{
		std::vector<vector2d> neighbors{};
		if (0 < current.x)
			neighbors.emplace_back(current.x - 1, current.y);
		if (0 < current.y)
			neighbors.emplace_back(current.x, current.y - 1);
		if (current.x + 1 < VWidth)
			neighbors.emplace_back(current.x + 1, current.y);
		if (current.y + 1 < VHeight)
			neighbors.emplace_back(current.x, current.y + 1);
		return neighbors;
	}
};

// simple calculate of the manhattan distance between two vectors; equally to the mathematical notation: |a.x-b.x| + |a.y-b.y|
struct manhattan_dist
{
	vector2d destination{};

	int operator()(const vector2d& current) const
	{
		return std::abs(destination.x - current.x)
				+ std::abs(destination.y - current.y);
	}
};

// accesses the grid and reads the value from the provided index (the current vertex)
template <class TGrid>
struct weight_reader_grid2d
{
	const TGrid& grid;

	constexpr int operator()(const vector2d& predecessor, const vector2d& current) const noexcept
	{
		// as we are in this example only interested in the entry of the grid, we do not take into account the vertex we came from.
		return grid[current.y][current.x];
	}
};

// helper type, which simply prints the given node on the console
struct node_printer
{
	std::string prefix{};

	void operator()(const astar_node_t& node) const
	{
		std::cout << prefix << "vertex: " << node.vertex;
		if (node.predecessor)
			std::cout << " predecessor: " << *node.predecessor;
		std::cout << " weight_sum: " << node.weight_sum << " estimated_cost: " << node.weight_estimated << "\n";
	}
};

// This functions simply executes an astar with a constant heuristic of 0. This is actually equivalent to dijkstra.
// As the callback does not return anything, the algorithm will run until all reachable vertices are visited.
template <int VHeight, int VWidth>
void astar_traversal(const grid2d<VHeight, VWidth>& grid, const vector2d& start)
{
	traverse
	(
		sl::graph::astar::searcher
		{
			.begin = start,
			.neighborSearcher = neighbor_searcher_2d<VHeight, VWidth>{},
			.weightCalculator = weight_reader_grid2d{ .grid = grid },
			.heuristic = sl::graph::constant_t<0>{},
			.callback = node_printer{ "astar-traversal" }
		}
	);
}

// This functions takes a start and an end vertex in fills the astar searcher accordingly.
// As the searcher itself is not interested in the actual endpoint, the heuristic and the callback have to take into account
// the end vertex theirselves.
// As the callback returns a bool, the algorithm runs until true is returned.
template <int VHeight, int VWidth>
void astar_pathfinder(const grid2d<VHeight, VWidth>& grid, const vector2d& start, const vector2d& end)
{
	traverse
	(
		sl::graph::astar::searcher
		{
			.begin = start,
			.neighborSearcher = neighbor_searcher_2d<VHeight, VWidth>{},
			.weightCalculator = weight_reader_grid2d{ .grid = grid },
			.heuristic = manhattan_dist{ end },
			.callback = [end](const astar_node_t& node)
			{
				node_printer{ "astar-pathfinder" }(node);

				return node.vertex == end;
			}
		}
	);
}

int main()
{
	constexpr grid2d<5, 7> grid
	{
		{
			{ 99, 99, 99, 99, 99, 99, 99 },
			{ 99, 5, 1, 1, 1, 1, 99 },
			{ 99, 1, 5, 5, 5, 1, 99 },
			{ 99, 1, 1, 1, 1, 5, 99 },
			{ 99, 99, 99, 99, 99, 99, 99 }
		}
	};

	astar_traversal(grid, { 1, 1 });
	astar_pathfinder(grid, { 1, 1 }, { 5, 3 });
}
