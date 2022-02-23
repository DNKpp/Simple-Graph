//          Copyright Dominic Koepke 2019 - 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "Simple-Graph/utility.hpp"

using namespace sl::graph;

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
