//          Copyright Dominic Koepke 2019 - 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "Simple-Graph/policies/open_list/common.hpp"
#include "Simple-Graph/policies/open_list/priority_queue.hpp"
#include "Simple-Graph/policies/open_list/queue.hpp"
#include "Simple-Graph/policies/open_list/stack.hpp"

#include <ranges>

using namespace sl::graph;

using test_node = weighted_node<std::string, int>;

template <class T>
T init_open_list(std::ranges::input_range auto&& nodes)
{
	T q{};
	for (const test_node& n : nodes)
	{
		q.enqueue(n);
	}

	return q;
}

TEMPLATE_TEST_CASE(
	"Given types satisfy open_list_policy_for concept.",
	"[open_list]",
	policies::open_list::Queue<test_node>,
	policies::open_list::PriorityQueue<test_node>
)
{
	STATIC_REQUIRE(concepts::open_list_policy_for<TestType, test_node>);
}

TEST_CASE("Queue policy stores nodes and returns them in the same order.", "[open_list]")
{
	const std::vector<test_node> nodes{
		{"1", 0, std::nullopt},
		{"2", 1, "1"},
		{"3", 1, "1"},
		{"4", 2, "3"}
	};

	auto queue = init_open_list<policies::open_list::Queue<test_node>>(nodes);

	for (const test_node& n : nodes)
	{
		REQUIRE(queue.has_pending());

		const test_node next = queue.take_next();

		REQUIRE(next == n);
	}

	REQUIRE(!queue.has_pending());
}

TEST_CASE("PriorityQueue policy stores nodes and returns them ordered by weight.", "[open_list]")
{
	const std::vector<test_node> nodes{
		{"1", 0, std::nullopt},
		{"2", 3, "1"},
		{"4", 5, "3"},
		{"3", 1, "1"}
	};
	const std::vector expectedNodes{
		nodes[0], nodes[3], nodes[1], nodes[2]
	};

	auto queue = init_open_list<policies::open_list::PriorityQueue<test_node>>(nodes);

	for (const test_node& n : expectedNodes)
	{
		REQUIRE(queue.has_pending());

		const test_node next = queue.take_next();

		REQUIRE(next == n);
	}

	REQUIRE(!queue.has_pending());
}

TEST_CASE("Stack policy stores nodes and returns them like a stack.", "[open_list]")
{
	const std::vector<test_node> nodes{
		{"1", 0, std::nullopt},
		{"2", 3, "1"},
		{"4", 5, "3"},
		{"3", 1, "1"}
	};
	const auto expectedNodes = nodes | std::views::reverse;

	auto queue = init_open_list<policies::open_list::Stack<test_node>>(nodes);

	for (const test_node& n : expectedNodes)
	{
		REQUIRE(queue.has_pending());

		const test_node next = queue.take_next();

		REQUIRE(next == n);
	}

	REQUIRE(!queue.has_pending());
}
