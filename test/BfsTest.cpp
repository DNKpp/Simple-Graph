//          Copyright Dominic Koepke 2019 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "Simple-Graph/BFS.hpp"

#include "catch.hpp"

#include <array>

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
				0 <= cur.x && cur.x < std::size((*m_Table)[cur.y]))
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

TEST_CASE("traverse table breadth-first-search", "[bfs]")
{
	constexpr Vector size{ 10, 10 };
	std::array<std::array<int, 10>, 10> table{};

	using DfsNodeInfo_t = sl::graph::BfsNodeInfo_t<Vector>;
	sl::graph::DefaultBfsStateMap_t<Vector> stateMap;
	SECTION("check early return through callback")
	{
		int count = 0;
		sl::graph::traverseBfs(
								Vector{ 0, 0 },
								NeighbourSearcher{ table },
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
		sl::graph::traverseBfs(
								Vector{ 0, 0 },
								NeighbourSearcher{ table },
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
