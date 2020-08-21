//          Copyright Dominic Koepke 2019 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "Simple-Graph/AStar.hpp"

#include "catch.hpp"
#include "TestUtility.hpp"

TEST_CASE("traverse table A* without obstacles", "[AStar]")
{
	using Node_t = sl::graph::AStarNodeInfo_t<Vector, int>;

	constexpr Vector size{ 10, 10 };
	std::array<std::array<int, 10>, 10> table;
	table.fill({ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 });

	std::map<Vector, Node_t> stateMap;
	Vector start{ 0, 0 };
	Vector destination{ 7, 8 };
	sl::graph::traverseAStar(start, destination, PropertyMap{ table }, NeighbourSearcher{ table, true }, stateMap);

	auto startItr = stateMap.find(start);
	REQUIRE(startItr != std::end(stateMap));
	REQUIRE(!startItr->second.parent);
	REQUIRE(startItr->second.heuristic == 15);
	REQUIRE(startItr->second.weightSum == 0);

	auto destItr = stateMap.find(destination);
	REQUIRE(destItr != std::end(stateMap));
	REQUIRE(destItr->second.parent);
	REQUIRE(destItr->second.heuristic == 0);
	REQUIRE(destItr->second.weightSum == 15);
}

TEST_CASE("traverse table A* with obstacles but reachable", "[AStar]")
{
	using Node_t = sl::graph::AStarNodeInfo_t<Vector, int>;

	constexpr Vector size{ 10, 10 };
	std::array<std::array<int, 10>, 10> table
	{
		{
			{ 1, 1, -1, 1, 1, 1, 1, 1, 1, 1 },
			{ 1, -1, 1, 1, 1, 1, 1, 1, 1, 1 },
			{ 1, -1, 1, 1, 1, 1, 1, 1, 1, 1 },
			{ 1, 1, -1, 1, 1, 1, 1, 1, 1, 1 },
			{ 1, 1, 1, -1, 1, 1, 1, 1, 1, 1 },
			{ 1, 1, 1, 1, -1, 1, 1, 1, 1, 1 },
			{ 1, 1, 1, 1, 1, -1, 1, 1, 1, 1 },
			{ 1, -1, -1, 1, 1, -1, 1, 1, 1, 1 },
			{ 1, -1, 1, 1, 1, 1, -1, 1, 1, 1 },
			{ 1, -1, 1, 1, 1, 1, 1, 1, 1, 1 }
		}
	};

	std::map<Vector, Node_t> stateMap;
	Vector start{ 9, 0 };
	Vector destination{ 1, 0 };
	sl::graph::traverseAStar(start, destination, PropertyMap{ table }, NeighbourSearcher{ table, true }, stateMap);

	auto startItr = stateMap.find(start);
	REQUIRE(startItr != std::end(stateMap));
	REQUIRE(!startItr->second.parent);
	REQUIRE(startItr->second.heuristic == 8);
	REQUIRE(startItr->second.weightSum == 0);

	auto destItr = stateMap.find(destination);
	REQUIRE(destItr != std::end(stateMap));
	REQUIRE(destItr->second.parent);
	REQUIRE(destItr->second.heuristic == 0);
	REQUIRE(destItr->second.weightSum == 28);
}
