//          Copyright Dominic Koepke 2019 - 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "Simple-Graph/policies/visit_tracking/common.hpp"
#include "Simple-Graph/policies/visit_tracking/map.hpp"

#include <string>

using namespace sl::graph;

using test_vertex = std::string;

TEMPLATE_TEST_CASE(
	"Given types satisfy visit_track_policy_for concept.",
	"[visit_tracking]",
	policies::visit_tracking::Map<test_vertex>
)
{
	STATIC_REQUIRE(concepts::visit_track_policy_for<TestType, test_vertex>);
}

TEST_CASE("Map policy tracks visitation state of vertices.", "[visit_tracking]")
{
	const test_vertex vertex{"1337"};

	policies::visit_tracking::Map<test_vertex> map{};

	REQUIRE(!map.is_visited(vertex));
	REQUIRE(!map.set_visited(vertex));
	REQUIRE(map.is_visited(vertex));
}
