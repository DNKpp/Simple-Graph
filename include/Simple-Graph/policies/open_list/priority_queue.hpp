//          Copyright Dominic Koepke 2019 - 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SIMPLE_GRAPH_POLICIES_OPEN_LIST_PRIORITY_QUEUE_HPP
#define SIMPLE_GRAPH_POLICIES_OPEN_LIST_PRIORITY_QUEUE_HPP

#pragma once

#include "Simple-Graph/policies/open_list/common.hpp"

#include <queue>

namespace sl::graph::policies::open_list
{
	template <concepts::weighted_node TNode, std::predicate<TNode, TNode> TCompare = node_after<TNode>>
	class PriorityQueue
	{
	public:
		void enqueue(TNode node)
		{
			m_Queue.emplace(std::move(node));
		}

		[[nodiscard]]
		TNode take_next()
		{
			TNode next = m_Queue.top();
			m_Queue.pop();

			return next;
		}

		[[nodiscard]]
		bool has_pending() const
		{
			return !m_Queue.empty();
		}

	private:
		std::priority_queue<TNode, std::vector<TNode>, TCompare> m_Queue{};
	};
}

#endif
