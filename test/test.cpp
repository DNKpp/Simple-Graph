//          Copyright Dominic Koepke 2019 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include "Simple-Graph/algorithm.hpp"

#include <array>
#include <bitset>
#include <optional>

struct Vector
{
    int x = 0;
    int y = 0;

	friend bool operator ==(const Vector& _lhs, const Vector& _rhs)
	{
		return _lhs.x == _rhs.x && _lhs.y == _rhs.y;
	}
};

template <int WIDTH, int HEIGHT>
auto make_table_graph()
{
    std::array<std::array<int, WIDTH>, HEIGHT> table;
    int value = 0;
    for (auto& row : table)
        std::generate(std::begin(row), std::end(row), [&value]() { return value++; });
    return table;
}

template <int WIDTH, int HEIGHT>
class TableVisitationTracker
{
public:
    decltype(auto) operator [](const Vector& _at) const
    {
        return m_Check[_at.y * WIDTH + _at.x];
    }
    
    decltype(auto) operator [](const Vector& _at)
    {
        return m_Check[_at.y * WIDTH + _at.x];
    }
    
private:
    std::bitset<WIDTH * WIDTH> m_Check;
};

TEST_CASE("traverse table breadth-first-search", "[bfs]")
{
    constexpr Vector size{ 10, 10 };
	
	auto table = make_table_graph<size.x, size.y>();

    auto bfsNeighbourSearcher = [&table](const auto& _node, auto _callback)
    {
        for (int i = 0; i < 4; ++i)
        {
            auto cur = _node.vertex;
            switch (i)
            {
            case 0: ++cur.x; break;
            case 1: --cur.x; break;
            case 2: ++cur.y; break;
            case 3: --cur.y; break;
            }
            
            if (0 <= cur.y && cur.y < std::size(table) &&
                0 <= cur.x && cur.x < std::size(table[cur.y]))
            {
                _callback(cur);
            }
        }
    };

    constexpr std::array<int, size.x * size.y> check
    {
         0, 1,10, 2,11,20, 3,12,21,30,
    	 4,13,22,31,40, 5,14,23,32,41,
    	50, 6,15,24,33,42,51,60, 7,16,
    	25,34,43,52,61,70, 8,17,26,35,
    	44,53,62,71,80, 9,18,27,36,45,
    	54,63,72,81,90,19,28,37,46,55,
    	64,73,82,91,29,38,47,56,65,74,
    	83,92,39,48,57,66,75,84,93,49,
    	58,67,76,85,94,59,68,77,86,95,
    	69,78,87,96,79,88,97,89,98,99
    };

	// check early return
	sl::graph::traverse_bfs(Vector{ 0, 0 }, bfsNeighbourSearcher, TableVisitationTracker<size.x, size.y>{},
        [count = 0](const auto& _node) mutable
		{
			REQUIRE(count == 0);
			++count;
			return true;
		}
    );
	
	sl::graph::traverse_bfs(Vector{ 0, 0 }, bfsNeighbourSearcher, TableVisitationTracker<size.x, size.y>{},
        [&table, itr = std::begin(check)](const auto& _node) mutable
		{
			REQUIRE(*(itr++) == table[_node.vertex.y][_node.vertex.x]);
		}
    );
}
