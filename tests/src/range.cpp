//          Copyright Dominic Koepke 2019 - 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "test_helper.hpp"

#include "Simple-Graph/range.hpp"
#include "Simple-Graph/traverse_algorithm.hpp"
#include "Simple-Graph/policies/node_factory/basic_factory.hpp"
#include "Simple-Graph/policies/visit_tracking/map.hpp"

using namespace sl::graph;

using test_vertex = int;
using test_node = node<test_vertex>;

template <class TVertex>
using single_element_algorithm_def = traverse_algorithm<
	node<TVertex>,
	policies::node_factory::basic_factory<node<TVertex>>,
	single_element_open_list<node<TVertex>>,
	policies::visit_tracking::Map<TVertex>
>;

template <class TVertex, class TWeight>
using single_element_weighted_algorithm_def = traverse_algorithm<
	weighted_node<TVertex, TWeight>,
	policies::node_factory::basic_factory<weighted_node<TVertex, TWeight>>,
	single_element_open_list<weighted_node<TVertex, TWeight>>,
	policies::visit_tracking::Map<TVertex>
>;

template <class TVertex>
using empty_algorithm_def = traverse_algorithm<
	node<TVertex>,
	policies::node_factory::basic_factory<node<TVertex>>,
	empty_open_list<node<TVertex>>,
	policies::visit_tracking::Map<TVertex>
>;

TEST_CASE("make_range creates correct range object for basic node types.", "[range]")
{
	using searcher = decltype(make_range<single_element_algorithm_def>(std::declval<IntRangeGraph>(), {}));

	STATIC_REQUIRE(std::same_as<searcher::vertex_type, int>);
	STATIC_REQUIRE(std::same_as<searcher::node_type, node<int>>);
	STATIC_REQUIRE(std::same_as<searcher::graph_type, IntRangeGraph>);
}

TEST_CASE("make_range creates correct range object for weighted node types.", "[range]")
{
	// I actually have no idea, why gcc below 11.2 has an issue with the general approach.
#if ((__GNUC__ > 0 && __GNUC__ < 11) || \
	(__GNUC__ == 11 && __GNUC_MINOR__ < 2))
	using searcher = Range<
			single_element_weighted_algorithm_def<graph_vertex_type<IntRangeWeightedGraph>, graph_weight_type<IntRangeWeightedGraph>>,
			IntRangeWeightedGraph
	>;
#else
	using searcher = decltype(make_range<single_element_weighted_algorithm_def>(std::declval<IntRangeWeightedGraph>(), {}));
#endif

	STATIC_REQUIRE(std::same_as<searcher::vertex_type, int>);
	STATIC_REQUIRE(std::same_as<searcher::node_type, weighted_node<int, int>>);
	STATIC_REQUIRE(std::same_as<searcher::graph_type, IntRangeWeightedGraph>);
}

TEST_CASE("range iterator returns visited nodes.", "[range]")
{
	const IntRangeGraph graph{{3, 9}};
	Range range = make_range<single_element_algorithm_def>(graph, 5);

	auto iter = range.begin();

	REQUIRE(iter != range.end());
	REQUIRE((*iter).vertex == 5);
	REQUIRE((*++iter).vertex == 6);
	REQUIRE((*++iter).vertex == 7);
	REQUIRE((*++iter).vertex == 8);
	REQUIRE(++iter == range.end());
}

TEST_CASE("range correctly propagates searcher state.", "[range]")
{
	const IntRangeGraph graph{{3, 9}};
	Range range = make_range<empty_algorithm_def>(graph, 5);

	REQUIRE(range.begin() == range.end());
}
