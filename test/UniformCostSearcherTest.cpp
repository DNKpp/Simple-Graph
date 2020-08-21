//          Copyright Dominic Koepke 2019 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "Simple-Graph/UniformCostSearcher.hpp"

#include "catch.hpp"
#include "TestUtility.hpp"

TEST_CASE("traverse table uniform-cost-searcher", "[UCS]")
{
	constexpr Vector size{ 10, 10 };
	std::array<std::array<int, 10>, 10> table{};
	table.fill({ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 });

	using UcsNodeInfo_t = sl::graph::UcsNodeInfo_t<Vector, int>;
	sl::graph::DefaultUcsStateMap_t<Vector, int> stateMap;
	SECTION("check early return through callback")
	{
		int count = 0;
		sl::graph::traverseUniformCostSearcher(
									Vector{ 0, 0 },
									Vector{ 9, 9 },
									PropertyMap{ table },
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
		sl::graph::traverseUniformCostSearcher(
									Vector{ 0, 0 },
									Vector{ 9, 9 },
									PropertyMap{ table },
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
					[](const auto& row) { return std::all_of(begin(row), end(row), [](const auto& val) { return val == 2; }); })
				);
		REQUIRE(std::size(stateMap) == size.x * size.y);
		REQUIRE(std::all_of(begin(stateMap), end(stateMap), [](const auto& pair){ return pair.second.state == sl::graph::NodeState::closed; }));
		REQUIRE(std::none_of(begin(stateMap), end(stateMap), [](const auto& pair){ return pair.second.state == sl::graph::NodeState::open; }));
	}
}
