//          Copyright Dominic Koepke 2019 - 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <catch2/catch_template_test_macros.hpp>

#include <array>
#include <ranges>
#include <string>
#include <vector>

#include "Simple-Graph/graph.hpp"

using namespace sl::graph;

template <class TVertex>
struct templated_neighbor_map
{
	static std::array<TVertex, 0> neighbors([[maybe_unused]] const TVertex&)
	{
		return {};
	}
};

using valid_neighbor_map = templated_neighbor_map<std::string>;
using undecayed_neighbor_map = valid_neighbor_map&;

struct no_neighbors_neighbor_map
{};

struct no_input_range_neighbor_map
{
	static std::optional<int> neighbors([[maybe_unused]] const std::string)
	{
		return {};
	}
};

struct wrong_vertex_neighbor_map
{
	static std::vector<int> neighbors([[maybe_unused]] const std::string)
	{
		return {};
	}
};

template <class TVertex, class TWeight>
struct templated_weight_map
{
	static TWeight weight([[maybe_unused]] const TVertex&, [[maybe_unused]] const TVertex&)
	{
		return {};
	}
};

using valid_weight_map = templated_weight_map<std::string, int>;
using undecayed_weight_map = valid_weight_map&;

struct wrong_params_weight_map
{
	static int weight([[maybe_unused]] const std::string&, [[maybe_unused]] const int&)
	{
		return {};
	}

	static int weight([[maybe_unused]] const std::string&)
	{
		return {};
	}
};

using wrong_return_weight_map = templated_weight_map<std::string, std::string>;

TEMPLATE_TEST_CASE(
	"neighbor_map_policy_for concept is not satisfied.",
	"[concept]",
	undecayed_neighbor_map,
	no_neighbors_neighbor_map,
	no_input_range_neighbor_map,
	wrong_vertex_neighbor_map,
	(templated_neighbor_map<int>)
)
{
	STATIC_REQUIRE(!concepts::neighbor_map_policy_for<TestType, std::string>);
}

TEMPLATE_TEST_CASE(
	"neighbor_map_policy_for concept is satisfied.",
	"[concept]",
	valid_neighbor_map
)
{
	STATIC_REQUIRE(concepts::neighbor_map_policy_for<TestType, std::string>);
}

TEMPLATE_TEST_CASE(
	"weight_map_policy_for concept is not satisfied.",
	"[concept]",
	undecayed_weight_map,
	wrong_params_weight_map,
	wrong_return_weight_map,
	(templated_weight_map<int, int>)
)
{
	STATIC_REQUIRE(!concepts::weight_map_policy_for<TestType, std::string, int>);
}

TEMPLATE_TEST_CASE(
	"weight_map_policy_for concept is satisfied.",
	"[concept]",
	valid_weight_map
)
{
	STATIC_REQUIRE(concepts::weight_map_policy_for<TestType, std::string, int>);
}

using graph_test_type = graph<std::string, templated_neighbor_map<std::string>>;
using weighted_graph_test_type = weighted_graph<
		std::string,
		int,
		templated_neighbor_map<std::string>,
		templated_weight_map<std::string, int>
	>;

TEMPLATE_TEST_CASE(
	"graph concept is satisfied.",
	"[concept]",
	graph_test_type,
	weighted_graph_test_type
)
{
	STATIC_REQUIRE(concepts::graph<TestType>);
}

TEMPLATE_TEST_CASE(
	"weighted_graph concept is satisfied.",
	"[concept]",
	weighted_graph_test_type
)
{
	STATIC_REQUIRE(concepts::weighted_graph<TestType>);
}

TEST_CASE("graph_vertex_type yields expected type.", "[trait]")
{
	STATIC_REQUIRE((std::same_as<std::string, graph_vertex_type<graph_test_type>>));
	STATIC_REQUIRE((std::same_as<std::string, graph_vertex_type<weighted_graph_test_type>>));
}

TEST_CASE("graph_weight_type yields expected type.", "[trait]")
{
	STATIC_REQUIRE((std::same_as<int, graph_weight_type<weighted_graph_test_type>>));
}
