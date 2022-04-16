//           Copyright Dominic Koepke 2019 - 2022.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "Simple-Graph/path_finder.hpp"
#include "Simple-Graph/utility.hpp"

using namespace sl::graph;

namespace
{
	template <class T>
	using node_t = weighted_node<T, int>;
}

TEST_CASE("shall_interrupt should invoke its provided invokable.", "[utility]")
{
	int invokeCounter = 0;
	const auto func = [&] { ++invokeCounter; };

	const bool _ = detail::shall_interrupt(func);

	REQUIRE(invokeCounter == 1);
}

TEST_CASE("shall_interrupt should forward its passed params to the invokable.", "[utility]")
{
	const auto func = [&](int x, int y)
	{
		REQUIRE(x == 1);
		REQUIRE(y == 42);
	};

	const bool _ = detail::shall_interrupt(func, 1, 42);
}

TEST_CASE("shall_interrupt should return false when invokables with void return type are provided", "[utility]")
{
	constexpr auto func = []
	{
	};

	REQUIRE(!detail::shall_interrupt(func));
}

TEST_CASE("shall_interrupt should the invocation result of the provided invokable.", "[utility]")
{
	const bool result = GENERATE(false, true);

	const auto func = [&] { return result; };

	REQUIRE(detail::shall_interrupt(func) == result);
}

TEST_CASE("vertex_destination_t should use its compare invocable for comparison.", "[utility]")
{
	int invoke_counter = 0;
	auto compare = [&invoke_counter](const int& lhs, const int& rhs)
	{
		++invoke_counter;
		return lhs == rhs;
	};

	vertex_destination_t destination
	{
		.destination = 1337,
		.compare = compare
	};

	REQUIRE(!std::invoke(destination, node_t<int>{ .vertex = 42 }));
	REQUIRE(std::invoke(destination, node_t<int>{ .vertex = 1337 }));
	REQUIRE(invoke_counter == 2);
}

TEST_CASE("path_finder_t should invoke destinationPredicate.", "[utility]")
{
	int invoke_counter = 0;
	auto predicate = [&invoke_counter](const auto& node)
	{
		++invoke_counter;
		return false;
	};
	std::vector<int> dummy{};

	std::invoke(make_path_finder<node_t<int>>(predicate, std::back_inserter(dummy)), node_t<int>{ .vertex = 42 });
	REQUIRE(invoke_counter == 1);
}
