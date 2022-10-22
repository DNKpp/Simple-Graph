//          Copyright Dominic Koepke 2019 - 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SIMPLE_GRAPH_POLICIES_VISIT_TRACKING_MAP_HPP
#define SIMPLE_GRAPH_POLICIES_VISIT_TRACKING_MAP_HPP

#pragma once

#include "Simple-Graph/policies/visit_tracking/common.hpp"

#include <map>

namespace sl::graph::policies::visit_tracking
{
	template <concepts::vertex TVertex, std::predicate<TVertex, TVertex> TCompare = std::less<>>
	class Map
	{
	public:
		[[nodiscard]]
		bool is_visited(const TVertex& vertex)
		{
			return m_Map[vertex];
		}

		[[nodiscard]]
		bool set_visited(const TVertex& vertex)
		{
			return std::exchange(m_Map[vertex], true);
		}

	private:
		std::map<TVertex, bool, TCompare> m_Map{};
	};
}

#endif
