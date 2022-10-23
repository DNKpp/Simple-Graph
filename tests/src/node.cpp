//          Copyright Dominic Koepke 2019 - 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <string>

#include "Simple-Graph/node.hpp"

using namespace sl::graph;

using node_test_type = node<std::string>;
using weighted_node_test_type = weighted_node<std::string, int>;

TEMPLATE_TEST_CASE(
	"node concept is satisfied.",
	"[concept]",
	node_test_type,
	weighted_node_test_type
)
{
	STATIC_REQUIRE(concepts::node<TestType>);
}

TEMPLATE_TEST_CASE(
	"weighted_node concept is satisfied.",
	"[concept]",
	weighted_node_test_type
)
{
	STATIC_REQUIRE(concepts::weighted_node<TestType>);
}

TEST_CASE("node_vertex_type yields expected type.", "[trait]")
{
	STATIC_REQUIRE((std::same_as<std::string, node_vertex_type<node_test_type>>));
	STATIC_REQUIRE((std::same_as<std::string, node_vertex_type<weighted_node_test_type>>));
}

TEST_CASE("node_weight_type yields expected type.", "[trait]")
{
	STATIC_REQUIRE((std::same_as<int, node_weight_type<weighted_node_test_type>>));
}

TEST_CASE("node_after for weighted_nodes yields expected results.", "[node]")
{
	const auto [lhsAccWeight, rhsAccWeight, expectedResult] = GENERATE(
		(table<int, int, bool>)({
			{0, 0, false},
			{0, 1, false},
			{1, 1, false},
			{2, 1, true}
			})
	);

	const weighted_node_test_type lhsNode{{}, lhsAccWeight};
	const weighted_node_test_type rhsNode{{}, rhsAccWeight};

	REQUIRE(node_after<weighted_node_test_type>{}(lhsNode, rhsNode) == expectedResult);
}
