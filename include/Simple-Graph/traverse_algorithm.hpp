//          Copyright Dominic Koepke 2019 - 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SIMPLE_GRAPH_TRAVERSE_ALGORITHM_HPP
#define SIMPLE_GRAPH_TRAVERSE_ALGORITHM_HPP

#pragma once

#include "Simple-Graph/node.hpp"
#include "Simple-Graph/policies/open_list/common.hpp"
#include "Simple-Graph/policies/visit_tracking/common.hpp"

namespace sl::graph
{
	template <
		class TNode,
		class TNodeFactoryPolicy,
		concepts::open_list_policy_for<TNode> TOpenListPolicy,
		concepts::visit_track_policy_for<node_vertex_type<TNode>> TVisitTrackPolicy
	>
	struct traverse_algorithm
	{
		using node_type = TNode;
		using node_factory_policy = TNodeFactoryPolicy;
		using open_list_policy = TOpenListPolicy;
		using visit_track_policy = TVisitTrackPolicy;
	};
}

#endif
