//          Copyright Dominic Koepke 2019 - 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SIMPLE_GRAPH_POLICIES_OPEN_LIST_COMMON_HPP
#define SIMPLE_GRAPH_POLICIES_OPEN_LIST_COMMON_HPP

#pragma once

#include "Simple-Graph/node.hpp"

namespace sl::graph::concepts
{
	template <class T, class TNode>
	concept open_list_policy_for = node<TNode>
									&& requires(T& policy, const TNode& n)
									{
										policy.enqueue(n);
										{ policy.take_next() } -> std::convertible_to<TNode>;
									}
									&& requires(const T& policy)
									{
										{ policy.has_pending() } -> std::convertible_to<bool>;
									};
}

#endif
