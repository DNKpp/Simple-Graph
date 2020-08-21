//          Copyright Dominic Koepke 2019 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "Simple-Graph/BreadthFirstSearch.hpp"

#include "catch.hpp"
#include "TestUtility.hpp"

TEST_CASE("traverse table breadth-first-search", "[bfs]")
{
	constexpr Vector size{ 10, 10 };
	std::array<std::array<int, 10>, 10> table{};

	using DfsNodeInfo_t = sl::graph::BfsNodeInfo_t<Vector>;
	sl::graph::DefaultBfsStateMap_t<Vector> stateMap;
	SECTION("check early return through callback")
	{
		int count = 0;
		sl::graph::traverseBreadthFirstSearch(
											Vector{ 0, 0 },
											NeighbourSearcher{ table, false },
											stateMap,
											[&count](const auto& vertex, const auto& nodeInfo)
											{
												++count;
												return true;
											}
											);
		REQUIRE(count == 1);
		REQUIRE(std::size(stateMap) == 1);
		REQUIRE(std::count_if(begin(stateMap), end(stateMap), [](const auto& pair){ return pair.second.state == sl::graph::NodeState::closed; }) == 1);
		REQUIRE(std::none_of(begin(stateMap), end(stateMap), [](const auto& pair){ return pair.second.state == sl::graph::NodeState::open; }));
	}

	SECTION("check node visitation count via callback")
	{
		sl::graph::traverseBreadthFirstSearch(
											Vector{ 0, 0 },
											NeighbourSearcher{ table, false },
											stateMap,
											[&table](const auto& vertex, const auto& nodeInfo)
											{
												++table[vertex.y][vertex.x];
											}
											);
		REQUIRE(
				std::all_of(
					begin(table),
					end(table),
					[](const auto& row) { return std::all_of(begin(row), end(row), [](const auto& val) { return val == 1; }); })
				);
		REQUIRE(std::size(stateMap) == size.x * size.y);
		REQUIRE(std::all_of(begin(stateMap), end(stateMap), [](const auto& pair){ return pair.second.state == sl::graph::NodeState::closed; }));
		REQUIRE(std::none_of(begin(stateMap), end(stateMap), [](const auto& pair){ return pair.second.state == sl::graph::NodeState::open; }));
	}
}
