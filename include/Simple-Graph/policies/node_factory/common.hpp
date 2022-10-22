//          Copyright Dominic Koepke 2019 - 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SIMPLE_GRAPH_POLICIES_NODE_FACTORY_COMMON_HPP
#define SIMPLE_GRAPH_POLICIES_NODE_FACTORY_COMMON_HPP

#pragma once

#include "Simple-Graph/node.hpp"

#include <concepts>

namespace sl::graph::concepts
{
	template <class T, class TNode, class TGraph>
	concept node_factory_policy_for = concepts::node<TNode>
									&& concepts::graph<TGraph>
									&& requires(T policy, const TNode& n, const TGraph& g)
									{
										{ policy.make_init_node(g, n.vertex) } -> std::convertible_to<TNode>;
										{ policy.make_successor_node(g, n, n.vertex) } -> std::convertible_to<TNode>;
									};
}

#endif
