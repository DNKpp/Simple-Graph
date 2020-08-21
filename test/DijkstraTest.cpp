//          Copyright Dominic Koepke 2019 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "Simple-Graph/Dijkstra.hpp"

#include "catch.hpp"

#include <array>
#include <map>
#include <numeric>
#include <bitset>
#include <optional>
#include <compare>

struct Vector
{
    int x = 0;
    int y = 0;

	[[nodiscard]] constexpr auto operator <=>(const Vector& other) const noexcept = default;
};

template <std::size_t TWidth, std::size_t THeight>
constexpr auto makeTableGraph()
{
    std::array<std::array<int, TWidth>, THeight> table;
    int value = 0;
    for (auto& row : table)
    {
		std::iota(std::begin(row), std::end(row), value);
    	value += static_cast<int>(size(row));
    }
    return table;
}

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
	        case 0: ++cur.x; break;
	        case 1: --cur.x; break;
	        case 2: ++cur.y; break;
	        case 3: --cur.y; break;
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
		return 1;
	}

private:
	const TTable* m_Table = nullptr;
};
template <class TTable>
PropertyMap(const TTable&) -> PropertyMap<TTable>;

//TEST_CASE("traverse table A*", "[AStar]")
//{
//	using Node_t = sl::graph::AStarNodeState<Vector, int>;
//	
//	constexpr Vector size{ 10, 10 };
//    constexpr auto table = makeTableGraph<size.x, size.y>();
//	
//	//constexpr std::array<int, size.x * size.y> check
// //   {
// //        0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
// //   	19,18,17,16,15,14,13,12,11,10,
// //   	20,21,22,23,24,25,26,27,28,29,
// //   	39,38,37,36,35,34,33,32,31,30,
// //   	40,41,42,43,44,45,46,47,48,49,
// //   	59,58,57,56,55,54,53,52,51,50,
// //   	60,61,62,63,64,65,66,67,68,69,
// //   	79,78,77,76,75,74,73,72,71,70,
// //   	80,81,82,83,84,85,86,87,88,89,
// //   	99,98,97,96,95,94,93,92,91,90
// //   };
//
//	//std::map<Vector, Node_t> stateMap;
//	sl::graph::new_traverse_astar(Vector{ 0, 0 }, Vector{ 7, 8 }, PropertyMap{ table }, NeighbourSearcher{table}/*, stateMap*/);
//	//REQUIRE(stateMap.find(Vector{ 7, 8 })->second.parent);
//}
//template <int WIDTH, int HEIGHT>
//auto make_table_graph()
//{
//    std::array<std::array<int, WIDTH>, HEIGHT> table;
//    int value = 0;
//    for (auto& row : table)
//        std::generate(std::begin(row), std::end(row), [&value]() { return value++; });
//    return table;
//}
//
//template <int WIDTH, int HEIGHT>
//class TableVisitationTracker
//{
//public:
//    decltype(auto) operator [](const Vector& _at) const
//    {
//        return m_Check[_at.y * WIDTH + _at.x];
//    }
//    
//    decltype(auto) operator [](const Vector& _at)
//    {
//        return m_Check[_at.y * WIDTH + _at.x];
//    }
//    
//private:
//    std::bitset<WIDTH * WIDTH> m_Check;
//};
//
//TEST_CASE("traverse table depth-first-search", "[dfs]")
//{
//    constexpr Vector size{ 10, 10 };
//	
//	auto table = make_table_graph<size.x, size.y>();
//
//	auto dfsNeighbourSearcher = [&table](const auto& _node, auto _callback) -> std::optional<Vector>
//    {
//        for (int i = 0; i < 4; ++i)
//        {
//            auto cur = _node.vertex;
//            switch (i)
//            {
//            case 0: ++cur.x; break;
//            case 1: --cur.x; break;
//            case 2: ++cur.y; break;
//            case 3: --cur.y; break;
//            }
//            
//            if (0 <= cur.y && cur.y < std::size(table) &&
//                0 <= cur.x && cur.x < std::size(table[cur.y]))
//            {
//                if (_callback(cur))
//                    return cur;
//            }
//        }
//        return std::nullopt;
//    };
//
//    constexpr std::array<int, size.x * size.y> check
//    {
//         0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
//    	19,18,17,16,15,14,13,12,11,10,
//    	20,21,22,23,24,25,26,27,28,29,
//    	39,38,37,36,35,34,33,32,31,30,
//    	40,41,42,43,44,45,46,47,48,49,
//    	59,58,57,56,55,54,53,52,51,50,
//    	60,61,62,63,64,65,66,67,68,69,
//    	79,78,77,76,75,74,73,72,71,70,
//    	80,81,82,83,84,85,86,87,88,89,
//    	99,98,97,96,95,94,93,92,91,90
//    };
//
//	// check early returns
//	sl::graph::traverse_dfs(Vector{ 0, 0 }, dfsNeighbourSearcher, TableVisitationTracker<size.x, size.y>{},
//        [count = 0](const auto& _node) mutable
//		{
//			REQUIRE(count == 0);
//			++count;
//			return true;
//		}
//    );
//
//	sl::graph::traverse_dfs(Vector{ 0, 0 }, dfsNeighbourSearcher, TableVisitationTracker<size.x, size.y>{},
//		sl::graph::EmptyCallback{},
//        [count = 0](const auto& _node) mutable
//		{
//			REQUIRE(count == 0);
//			++count;
//			return true;
//		}
//    );
//	
//	sl::graph::traverse_dfs(Vector{ 0, 0 }, dfsNeighbourSearcher, TableVisitationTracker<size.x, size.y>{},
//        [&table, itr = std::begin(check)](const auto& _node) mutable
//		{
//			REQUIRE(*(itr++) == table[_node.vertex.y][_node.vertex.x]);
//		},
//        [&table, itr = std::rbegin(check)](const auto& _node) mutable
//		{
//			REQUIRE(*(itr++) == table[_node.vertex.y][_node.vertex.x]);
//		}
//    );
//}
//
//TEST_CASE("traverse table breadth-first-search", "[bfs]")
//{
//    constexpr Vector size{ 10, 10 };
//	
//	auto table = make_table_graph<size.x, size.y>();
//
//    auto bfsNeighbourSearcher = [&table](const auto& _node, auto _callback)
//    {
//        for (int i = 0; i < 4; ++i)
//        {
//            auto cur = _node.vertex;
//            switch (i)
//            {
//            case 0: ++cur.x; break;
//            case 1: --cur.x; break;
//            case 2: ++cur.y; break;
//            case 3: --cur.y; break;
//            }
//            
//            if (0 <= cur.y && cur.y < std::size(table) &&
//                0 <= cur.x && cur.x < std::size(table[cur.y]))
//            {
//                _callback(cur);
//            }
//        }
//    };
//
//    constexpr std::array<int, size.x * size.y> check
//    {
//         0, 1,10, 2,11,20, 3,12,21,30,
//    	 4,13,22,31,40, 5,14,23,32,41,
//    	50, 6,15,24,33,42,51,60, 7,16,
//    	25,34,43,52,61,70, 8,17,26,35,
//    	44,53,62,71,80, 9,18,27,36,45,
//    	54,63,72,81,90,19,28,37,46,55,
//    	64,73,82,91,29,38,47,56,65,74,
//    	83,92,39,48,57,66,75,84,93,49,
//    	58,67,76,85,94,59,68,77,86,95,
//    	69,78,87,96,79,88,97,89,98,99
//    };
//
//	// check early return
//	sl::graph::traverse_bfs(Vector{ 0, 0 }, bfsNeighbourSearcher, TableVisitationTracker<size.x, size.y>{},
//        [count = 0](const auto& _node) mutable
//		{
//			REQUIRE(count == 0);
//			++count;
//			return true;
//		}
//    );
//	
//	sl::graph::traverse_bfs(Vector{ 0, 0 }, bfsNeighbourSearcher, TableVisitationTracker<size.x, size.y>{},
//        [&table, itr = std::begin(check)](const auto& _node) mutable
//		{
//			REQUIRE(*(itr++) == table[_node.vertex.y][_node.vertex.x]);
//		}
//    );
//}
//
//TEST_CASE("traverse dijkstra-search", "[dijkstra]")
//{
//    constexpr Vector size{ 10, 10 };
//	
//	auto table = make_table_graph<size.x, size.y>();
//
//    auto bfsNeighbourSearcher = [&table](const auto& _node, auto _callback)
//    {
//        for (int i = 0; i < 4; ++i)
//        {
//            auto cur = _node.vertex;
//            switch (i)
//            {
//            case 0: ++cur.x; break;
//            case 1: --cur.x; break;
//            case 2: ++cur.y; break;
//            case 3: --cur.y; break;
//            }
//            
//            if (0 <= cur.y && cur.y < std::size(table) &&
//                0 <= cur.x && cur.x < std::size(table[cur.y]))
//            {
//                _callback(cur);
//            }
//        }
//    };
//
//    constexpr std::array<int, size.x * size.y> check
//    {
//         0, 1,10, 2,11,20, 3,12,21,30,
//    	 4,13,22,31,40, 5,14,23,32,41,
//    	50, 6,15,24,33,42,51,60, 7,16,
//    	25,34,43,52,61,70, 8,17,26,35,
//    	44,53,62,71,80, 9,18,27,36,45,
//    	54,63,72,81,90,19,28,37,46,55,
//    	64,73,82,91,29,38,47,56,65,74,
//    	83,92,39,48,57,66,75,84,93,49,
//    	58,67,76,85,94,59,68,77,86,95,
//    	69,78,87,96,79,88,97,89,98,99
//    };
//
//	// check early return
//	sl::graph::traverse_dijkstra(Vector{ 0, 0 }, bfsNeighbourSearcher, TableVisitationTracker<size.x, size.y>{},
//        sl::graph::ConstWeight<1>(), sl::graph::ConstWeight<0>(),
//        [count = 0](const auto&) mutable
//		{
//			REQUIRE(count == 0);
//			++count;
//			return true;
//		}
//    );
//	
//	sl::graph::traverse_dijkstra(Vector{ 0, 0 }, bfsNeighbourSearcher, TableVisitationTracker<size.x, size.y>{},
//        sl::graph::ConstWeight<1>(), sl::graph::ConstWeight<0>(),
//        [last_weight = int(0)](const auto& _node) mutable
//		{
//			REQUIRE(last_weight <= _node.weight_sum);
//			last_weight = std::max(last_weight, _node.weight_sum);
//		}
//    );
//}
//
//TEST_CASE("traverse astar-search", "[astar]")
//{
//    constexpr Vector size{ 10, 10 };
//	
//	auto table = make_table_graph<size.x, size.y>();
//
//    auto neighbourSearcher = [&table](const auto& _node, auto _callback)
//    {
//        for (int i = 0; i < 4; ++i)
//        {
//            auto cur = _node.vertex;
//            switch (i)
//            {
//            case 0: ++cur.x; break;
//            case 1: --cur.x; break;
//            case 2: ++cur.y; break;
//            case 3: --cur.y; break;
//            }
//            
//            if (0 <= cur.y && cur.y < std::size(table) &&
//                0 <= cur.x && cur.x < std::size(table[cur.y]))
//            {
//                _callback(cur);
//            }
//        }
//    };
//
//    constexpr std::array<int, size.x * size.y> check
//    {
//         0, 1,10, 2,11,20, 3,12,21,30,
//    	 4,13,22,31,40, 5,14,23,32,41,
//    	50, 6,15,24,33,42,51,60, 7,16,
//    	25,34,43,52,61,70, 8,17,26,35,
//    	44,53,62,71,80, 9,18,27,36,45,
//    	54,63,72,81,90,19,28,37,46,55,
//    	64,73,82,91,29,38,47,56,65,74,
//    	83,92,39,48,57,66,75,84,93,49,
//    	58,67,76,85,94,59,68,77,86,95,
//    	69,78,87,96,79,88,97,89,98,99
//    };
//
//	// traverse from top-left to bottom-right
//	sl::graph::traverse_astar(Vector{ 0, 0 }, Vector{ 9, 9 }, neighbourSearcher, TableVisitationTracker<size.x, size.y>{},
//        [](const Vector& _lhs, const Vector& _rhs) { return std::abs(_lhs.x - _rhs.x) + std::abs(_lhs.y - _rhs.y); },
//        sl::graph::ConstWeight<1>(), sl::graph::ConstWeight<0>(),
//        [last_weight = int(0), counter = int(0)](const auto& _node) mutable
//		{
//			last_weight = std::max(last_weight, _node.weight_sum);
//			//REQUIRE(++counter < 20);
//		}
//    );
//}