//          Copyright Dominic Koepke 2019 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "Simple-Graph/DepthFirstSearch.hpp"

#include "catch.hpp"

#include "TestUtility.hpp"

#include <vector>

class VisitationTracker
{
public:
	VisitationTracker(const Vector& size) :
		m_Size{ size },
		m_Data(size.x * size.y, false)
	{
	}
	
	decltype(auto) operator [](const Vector& at) const noexcept
	{
		return m_Data[at.y * m_Size.x + at.x];
	}

	decltype(auto) operator [](const Vector& at) noexcept
	{
		return m_Data[at.y * m_Size.x + at.x];
	}

	Vector m_Size;
	std::vector<bool> m_Data;
};

TEST_CASE("traverse table depth-first-search", "[dfs]")
{
	constexpr Vector size{ 10, 10 };
	std::array<std::array<int, 10>, 10> table{};

	using DfsNodeInfo_t = sl::graph::DfsNodeInfo_t<Vector>;
	sl::graph::DfsNodeInfoStateMap_t<Vector> stateMap;
	SECTION("check early return through preCallback")
	{
		int count = 0;
		sl::graph::traverseDepthFirstSearchIterative(
													Vector{ 0, 0 },
													IterativeNeighbourSearcher{ table },
													stateMap,
													[&count](const auto& vertex, const auto& nodeInfo)
													{
														++count;
														return true;
													}
													);
		REQUIRE(count == 1);
		REQUIRE(std::size(stateMap) == 1);
		REQUIRE(std::none_of(begin(stateMap), end(stateMap), [](const auto& pair){ return pair.second.state == sl::graph::NodeState::closed; }));
		REQUIRE(std::all_of(begin(stateMap), end(stateMap), [](const auto& pair){ return pair.second.state == sl::graph::NodeState::open; }));
	}

	SECTION("check early return through postCallback")
	{
		int count = 0;
		traverseDepthFirstSearchIterative(
										Vector{ 0, 0 },
										IterativeNeighbourSearcher{ table },
										stateMap,
										sl::graph::EmptyCallback{},
										[&count](const auto& vertex, const auto& nodeInfo)
										{
											++count;
											return true;
										}
										);
		REQUIRE(count == 1);
		REQUIRE(std::size(stateMap) == size.x * size.y);
		REQUIRE(std::count_if(begin(stateMap), end(stateMap), [](const auto& pair){ return pair.second.state == sl::graph::NodeState::closed; }) == 1);
		REQUIRE(std::count_if(begin(stateMap), end(stateMap), [](const auto& pair){ return pair.second.state == sl::graph::NodeState::open; }) == size.x * size.y - 1);
	}

	SECTION("check node visitation count via preCallback")
	{
		sl::graph::traverseDepthFirstSearchIterative(
													Vector{ 0, 0 },
													IterativeNeighbourSearcher{ table },
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

	SECTION("check node visitation count via postCallback")
	{
		traverseDepthFirstSearchIterative(
										Vector{ 0, 0 },
										IterativeNeighbourSearcher{ table },
										stateMap,
										sl::graph::EmptyCallback{},
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

	SECTION("check node visitation with boolean count via preCallback")
	{
		sl::graph::DefaultDfsStateMap_t<Vector> booleanStateMap;
		sl::graph::traverseDepthFirstSearchIterative(
													Vector{ 0, 0 },
													IterativeNeighbourSearcher{ table },
													booleanStateMap,
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
		REQUIRE(std::size(booleanStateMap) == size.x * size.y);
		REQUIRE(std::all_of(begin(booleanStateMap), end(booleanStateMap), [](const auto& pair) { return pair.second; }));
	}

	SECTION("check node visitation with boolean count via preCallback and custom st::vector<bool> visitation tracker")
	{
		VisitationTracker visitationTracker{ size };
		sl::graph::traverseDepthFirstSearchIterative(
													Vector{ 0, 0 },
													IterativeNeighbourSearcher{ table },
													visitationTracker,
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
		REQUIRE(std::size(visitationTracker.m_Data) == size.x * size.y);
		REQUIRE(std::all_of(begin(visitationTracker.m_Data), end(visitationTracker.m_Data), [](const auto& val) { return val; }));
	}
}
