//          Copyright Dominic Koepke 2019 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "Simple-Graph/Astar.hpp"

#include "catch.hpp"

#include <array>
#include <compare>
#include <map>
#include <numeric>
#include <optional>

struct Vector
{
	int x = 0;
	int y = 0;

	[[nodiscard]] constexpr auto operator <=>(const Vector& other) const noexcept = default;
};

template <class TTable>
class NeighbourSearcher
{
public:
	NeighbourSearcher(const TTable& table) :
		m_Table{ &table }
	{
	}

	template <class TNode, class TCallback>
	void operator ()(const Vector& vertex, const TNode& node, TCallback callback) const
	{
		assert(m_Table);

		for (int i = 0; i < 4; ++i)
		{
			auto cur = vertex;
			switch (i)
			{
			case 0: ++cur.x;
				break;
			case 1: --cur.x;
				break;
			case 2: ++cur.y;
				break;
			case 3: --cur.y;
				break;
			}

			if (0 <= cur.y && cur.y < std::size(*m_Table) &&
				0 <= cur.x && cur.x < std::size((*m_Table)[cur.y]) &&
				0 < (*m_Table)[cur.y][cur.x])
			{
				callback(cur);
			}
		}
	}

private:
	const TTable* m_Table = nullptr;
};

template <class TTable>
NeighbourSearcher(const TTable&) -> NeighbourSearcher<TTable>;

template <class TTable>
class PropertyMap
{
public:
	using VertexType = Vector;
	using WeightType = int;

	PropertyMap(const TTable& table) :
		m_Table{ &table }
	{
	}

	int heuristic(const Vector& vertex, const Vector& destination) const
	{
		return std::abs(vertex.x - destination.x) + std::abs(vertex.y - destination.y);
	}

	int edgeWeight(const Vector& from, const Vector& to) const
	{
		return 0;
	}

	int nodeWeight(const Vector& vertex) const
	{
		return (*m_Table)[vertex.y][vertex.x];
	}

private:
	const TTable* m_Table = nullptr;
};

template <class TTable>
PropertyMap(const TTable&) -> PropertyMap<TTable>;

TEST_CASE("traverse table A* without obstacles", "[AStar]")
{
	using Node_t = sl::graph::AStarNodeInfo_t<Vector, int>;

	constexpr Vector size{ 10, 10 };
	std::array<std::array<int, 10>, 10> table;
	table.fill({ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 });

	std::map<Vector, Node_t> stateMap;
	Vector start{ 0, 0 };
	Vector destination{ 7, 8 };
	sl::graph::traverseAStar(start, destination, PropertyMap{ table }, NeighbourSearcher{ table }, stateMap);

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
	sl::graph::traverseAStar(start, destination, PropertyMap{ table }, NeighbourSearcher{ table }, stateMap);

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
