//          Copyright Dominic Koepke 2019 - 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SIMPLE_GRAPH_POLICIES_VISIT_TRACKING_COMMON_HPP
#define SIMPLE_GRAPH_POLICIES_VISIT_TRACKING_COMMON_HPP

#pragma once

#include "Simple-Graph/common.hpp"

namespace sl::graph::concepts
{
	template <class T, class TVertex>
	concept visit_track_policy_for = vertex<TVertex>
									&& requires(T& policy, const TVertex& v)
									{
										{ policy.set_visited(v) } -> std::convertible_to<bool>;
										{ policy.is_visited(v) } -> std::convertible_to<bool>;
									};
}

#endif
