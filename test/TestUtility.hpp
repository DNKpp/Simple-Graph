//          Copyright Dominic Koepke 2019 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SL_GRAPH_TEST_TEST_UTILITY_HPP
#define SL_GRAPH_TEST_TEST_UTILITY_HPP

#pragma once

#include <array>
#include <compare>
#include <numeric>
#include <optional>

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
	NeighbourSearcher(const TTable& table, bool filterLessEqualZero) :
		m_Table{ &table },
		m_FilterLessEqualZero{ filterLessEqualZero }
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
				(!m_FilterLessEqualZero || 0 < (*m_Table)[cur.y][cur.x])
			)
			{
				callback(cur);
			}
		}
	}

private:
	const TTable* m_Table = nullptr;
	bool m_FilterLessEqualZero = false;
};

template <class TTable>
NeighbourSearcher(const TTable&) -> NeighbourSearcher<TTable>;

template <class TTable>
class IterativeNeighbourSearcher
{
public:
	IterativeNeighbourSearcher(const TTable& table) :
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
				0 <= cur.x && cur.x < std::size((*m_Table)[cur.y])
			)
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
IterativeNeighbourSearcher(const TTable&) -> IterativeNeighbourSearcher<TTable>;

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

#endif
