//          Copyright Dominic Koepke 2019 - 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "test_helper.hpp"

#include "Simple-Graph/searcher.hpp"
#include "Simple-Graph/traverse_algorithm.hpp"
#include "Simple-Graph/policies/node_factory/basic_factory.hpp"
#include "Simple-Graph/policies/visit_tracking/map.hpp"

using namespace sl::graph;

using test_vertex = int;
using test_node = node<test_vertex>;

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

static_assert(concepts::open_list_policy_for<single_element_open_list<test_node>, test_node>);

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

static_assert(concepts::open_list_policy_for<empty_open_list<test_node>, test_node>);

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

TEST_CASE("make_searcher creates correct searcher object for basic node types.", "[searcher]")
{
	using searcher = decltype(make_searcher<single_element_algorithm_def>(std::declval<IntRangeGraph>(), {}));

	STATIC_REQUIRE(std::same_as<searcher::vertex_type, int>);
	STATIC_REQUIRE(std::same_as<searcher::node_type, node<int>>);
}

TEST_CASE("make_searcher creates correct searcher object for weighted node types.", "[searcher]")
{
	// I actually have no idea, why gcc below 11.2 has an issue with the general approach.
#if ((__GNUC__ > 0 && __GNUC__ < 11) || \
	(__GNUC__ == 11 && __GNUC_MINOR__ < 2))
	using searcher = detail::searcher_type<
			single_element_weighted_algorithm_def<graph_vertex_type<IntRangeWeightedGraph>, graph_weight_type<IntRangeWeightedGraph>>,
			IntRangeWeightedGraph
	>;
#else
	using searcher = decltype(make_searcher<single_element_weighted_algorithm_def>(std::declval<const IntRangeWeightedGraph&>(), 0));
#endif

	STATIC_REQUIRE(std::same_as<searcher::vertex_type, int>);
	STATIC_REQUIRE(std::same_as<searcher::node_type, weighted_node<int, int>>);
}

TEST_CASE("searcher traverses graph and keeps algorithm state.", "[searcher]")
{
	const IntRangeGraph graph{{3, 9}};
	Searcher searcher = make_searcher<single_element_algorithm_def>(graph, 5);

	REQUIRE(!searcher.has_finished());
	REQUIRE(searcher.next()->vertex == 5);
	REQUIRE(searcher.next()->vertex == 6);
	REQUIRE(searcher.next()->vertex == 7);
	REQUIRE(searcher.next()->vertex == 8);
	REQUIRE(searcher.has_finished());
	REQUIRE(!searcher.next());
}

template <class TVertex>
using empty_algorithm_def = traverse_algorithm<
	node<TVertex>,
	policies::node_factory::basic_factory<node<TVertex>>,
	empty_open_list<node<TVertex>>,
	policies::visit_tracking::Map<TVertex>
>;

TEST_CASE("searcher respects open list beahviour.", "[searcher]")
{
	const IntRangeGraph graph{{3, 9}};
	Searcher searcher = make_searcher<empty_algorithm_def>(graph, 5);

	REQUIRE(searcher.has_finished());
	REQUIRE(!searcher.next());
}
