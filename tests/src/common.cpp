//          Copyright Dominic Koepke 2019 - 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <catch2/catch_template_test_macros.hpp>

#include <string>

#include "Simple-Graph/common.hpp"

using namespace sl::graph;

struct non_equality_comparable
{
	bool operator ==(const non_equality_comparable&) const = delete;
	auto operator <=>(const non_equality_comparable&) const = default;
};

struct non_copyable
{
	non_copyable(const non_copyable&) = delete;
	non_copyable& operator =(const non_copyable&) = delete;

	auto operator <=>(const non_copyable&) const = default;
};

struct non_totally_ordered
{
	bool operator ==(const non_totally_ordered&) const = default;
};

struct non_mutable_plus_weight
{
	auto operator <=>(const non_mutable_plus_weight&) const = default;

	non_mutable_plus_weight operator +([[maybe_unused]] const non_mutable_plus_weight&) const
	{
		return *this;
	}
};

struct non_immutable_plus_weight
{
	auto operator <=>(const non_immutable_plus_weight&) const = default;

	non_immutable_plus_weight& operator +=([[maybe_unused]] const non_immutable_plus_weight&)
	{
		return *this;
	}
};

struct valid_weight
{
	auto operator <=>(const valid_weight&) const = default;

	valid_weight& operator +=([[maybe_unused]] const valid_weight&)
	{
		return *this;
	}

	valid_weight operator +([[maybe_unused]] const valid_weight&) const
	{
		return *this;
	}
};

TEMPLATE_TEST_CASE(
	"vertex concept is satisfied.",
	"[concept]",
	int,
	std::string
)
{
	STATIC_REQUIRE(concepts::vertex<TestType>);
}

TEMPLATE_TEST_CASE(
	"vertex concept is not satisfied.",
	"[concept]",
	const int,	// not decayed
	int&,		// not decayed
	non_equality_comparable,
	non_copyable
)
{
	STATIC_REQUIRE(!concepts::vertex<TestType>);
}

TEMPLATE_TEST_CASE(
	"weight concept is satisfied.",
	"[concept]",
	int,
	float,
	valid_weight
)
{
	STATIC_REQUIRE(concepts::weight<TestType>);
}

TEMPLATE_TEST_CASE(
	"vertex concept is not satisfied.",
	"[concept]",
	const int,	// not decayed
	int&,		// not decayed
	non_equality_comparable,
	non_copyable,
	non_mutable_plus_weight,
	non_immutable_plus_weight
)
{
	STATIC_REQUIRE(!concepts::weight<TestType>);
}
