//          Copyright Dominic Koepke 2019 - 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SIMPLE_GRAPH_TESTS_TEST_HELPER_HPP
#define SIMPLE_GRAPH_TESTS_TEST_HELPER_HPP

#include "Simple-Graph/graph.hpp"
#include "Simple-Graph/policies/open_list/common.hpp"

#include <vector>

class IntRangeNeighborMap
{
public:
	IntRangeNeighborMap(const int begin, const int end)
		: m_Begin{begin},
		m_End{end}
	{}

	[[nodiscard]]
	std::vector<int> neighbors(const int v) const
	{
		std::vector<int> neighbors{};
		if (m_Begin < v)
		{
			neighbors.emplace_back(v - 1);
		}
		if (v + 1 < m_End)
		{
			neighbors.emplace_back(v + 1);
		}
		return neighbors;
	}

private:
	int m_Begin{};
	int m_End{};
};

class IntWeightMap
{
public:
	[[nodiscard]]
	int weight(const int from, const int to) const
	{
		return std::abs(to - from);
	}
};

using IntRangeGraph = sl::graph::graph<int, IntRangeNeighborMap>;
using IntRangeWeightedGraph = sl::graph::weighted_graph<int, int, IntRangeNeighborMap, IntWeightMap>;

template <class T>
struct single_element_open_list
{
	std::optional<T> element{};

	void enqueue(T n)
	{
		element = std::move(n);
	}

	[[nodiscard]]
	T take_next()
	{
		auto n = std::move(*element);
		element.reset();

		return n;
	}

	[[nodiscard]]
	bool has_pending() const
	{
		return element.has_value();
	}
};

static_assert(sl::graph::concepts::open_list_policy_for<single_element_open_list<sl::graph::node<int>>, sl::graph::node<int>>);

template <class T>
struct empty_open_list
{
	static void enqueue(T n)
	{ }

	[[nodiscard]]
	static T take_next()
	{
		assert(false && "Should never be called.");
		return {};
	}

	[[nodiscard]]
	static bool has_pending()
	{
		return false;
	}
};

static_assert(sl::graph::concepts::open_list_policy_for<empty_open_list<sl::graph::node<int>>, sl::graph::node<int>>);

#endif
