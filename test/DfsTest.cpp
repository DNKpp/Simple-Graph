//          Copyright Dominic Koepke 2019 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "Simple-Graph/DFS.hpp"

#include "catch.hpp"

#include <array>
#include <compare>
#include <map>
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
	std::optional<Vector> operator ()(const Vector& vertex, const TNode& node, TCallback callback) const
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
				if (callback(cur))
					return cur;
			}
		}
		return std::nullopt;
	}

private:
	const TTable* m_Table = nullptr;
};

template <class TTable>
NeighbourSearcher(const TTable&) -> NeighbourSearcher<TTable>;

TEST_CASE("traverse table depth-first-search", "[dfs]")
{
	constexpr Vector size{ 10, 10 };
	std::array<std::array<int, 10>, 10> table{};

	using DfsNodeInfo_t = sl::graph::DfsNodeInfo_t<Vector>;
	sl::graph::DefaultDfsStateMap_t<Vector> stateMap;
	SECTION("check early return through preCallback")
	{
		int count = 0;
		sl::graph::traverseDfsIterative(
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
	}

	SECTION("check early return through postCallback")
	{
		int count = 0;
		traverseDfsIterative(
							Vector{ 0, 0 },
							NeighbourSearcher{ table },
							stateMap,
							sl::graph::EmptyCallback{},
							[&count](const auto& vertex, const auto& nodeInfo)
							{
								++count;
								return true;
							}
							);
		REQUIRE(count == 1);
	}

	SECTION("check node visitation count via preCallback")
	{
		sl::graph::traverseDfsIterative(
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
	}

	SECTION("check node visitation count via postCallback")
	{
		sl::graph::traverseDfsIterative(
										Vector{ 0, 0 },
										NeighbourSearcher{ table },
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
	}
}
