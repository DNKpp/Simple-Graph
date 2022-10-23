//          Copyright Dominic Koepke 2019 - 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <ranges>
#include <string>

#include "Simple-Graph/policies/node_factory/basic_factory.hpp"
#include "Simple-Graph/policies/node_factory/common.hpp"

using namespace sl::graph;

using test_node = node<std::string>;
using test_weighted_node = weighted_node<std::string, int>;

struct dummy_neighbor_map
{
	[[nodiscard]]
	static std::ranges::empty_view<std::string> neighbors([[maybe_unused]] const std::string& v)
	{
		return {};
	}
};

struct dummy_weight_map
{
	int weight_value{};

	[[nodiscard]]
	int weight([[maybe_unused]] const std::string& from, [[maybe_unused]] const std::string& to) const
	{
		return weight_value;
	}
};

using test_graph = graph<std::string, dummy_neighbor_map>;
using test_weighted_graph = weighted_graph<std::string, int, dummy_neighbor_map, dummy_weight_map>;

TEMPLATE_TEST_CASE(
	"basic_factory satisfies node_factory_policy_for concept.",
	"[concept][node_factory]",
	test_node,
	test_weighted_node
)
{
	STATIC_REQUIRE(concepts::node_factory_policy_for<policies::node_factory::basic_factory<TestType>, TestType, test_weighted_graph>);
}

TEST_CASE("basic_factory creates init node instances.", "[node_factory]")
{
	const std::string& vertex = GENERATE(as<std::string>{}, "1", "12", "123");

	policies::node_factory::basic_factory<test_node> factory{};

	const node initNode = factory.make_init_node(test_graph{}, vertex);  // NOLINT(readability-static-accessed-through-instance)

	REQUIRE(initNode.vertex == vertex);
	REQUIRE(initNode.predecessor == std::nullopt);
}

TEST_CASE("basic_factory creates successor node instances.", "[node_factory]")
{
	const std::string& predecessorVertex = GENERATE(as<std::string>{}, "1", "133", "1337");
	const std::string& successorVertex = GENERATE(as<std::string>{}, "4", "2", "42");
	const test_node predecessor{predecessorVertex};

	policies::node_factory::basic_factory<test_node> factory{};

	const node successor = factory.make_successor_node(	// NOLINT(readability-static-accessed-through-instance)
		test_graph{},
		predecessor,
		successorVertex
	);

	REQUIRE(successor.vertex == successorVertex);
	REQUIRE(successor.predecessor == predecessorVertex);
}

TEST_CASE("basic_factory creates init weighted_node instances.", "[node_factory]")
{
	const std::string& vertex = GENERATE(as<std::string>{}, "1", "12", "123");

	constexpr test_weighted_graph graph{{}, dummy_weight_map{1337}};
	policies::node_factory::basic_factory<test_weighted_node> factory{};

	const weighted_node initNode = factory.make_init_node(graph, vertex);  // NOLINT(readability-static-accessed-through-instance)

	REQUIRE(initNode.vertex == vertex);
	REQUIRE(initNode.accumulatedWeight == 0);
	REQUIRE(initNode.predecessor == std::nullopt);
}

TEST_CASE("basic_factory creates successor weighted_node instances.", "[node_factory]")
{
	const std::string& predecessorVertex = GENERATE(as<std::string>{}, "1", "133", "1337");
	const int predecessorWeight = GENERATE(42, 1337);
	const std::string& successorVertex = GENERATE(as<std::string>{}, "4", "2", "42");
	const int relWeight = GENERATE(0, 1, 42);

	const test_weighted_graph graph{{}, dummy_weight_map{relWeight}};
	const test_weighted_node predecessor{{predecessorVertex}, predecessorWeight};

	policies::node_factory::basic_factory<test_weighted_node> factory{};

	const weighted_node successor = factory.make_successor_node(	// NOLINT(readability-static-accessed-through-instance)
		graph,
		predecessor,
		successorVertex
	);

	REQUIRE(successor.vertex == successorVertex);
	REQUIRE(successor.predecessor == predecessorVertex);
	REQUIRE(successor.accumulatedWeight == predecessorWeight + relWeight);
}
