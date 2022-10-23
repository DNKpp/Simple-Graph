//          Copyright Dominic Koepke 2019 - 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SIMPLE_GRAPH_POLICIES_OPEN_LIST_QUEUE_HPP
#define SIMPLE_GRAPH_POLICIES_OPEN_LIST_QUEUE_HPP

#pragma once

#include "Simple-Graph/policies/open_list/common.hpp"

#include <queue>

namespace sl::graph::policies::open_list
{
	template <concepts::node TNode>
	class Queue
	{
	public:
		void enqueue(TNode node)
		{
			m_Queue.emplace(std::move(node));
		}

		[[nodiscard]]
		TNode take_next()
		{
			TNode next = m_Queue.front();
			m_Queue.pop();

			return next;
		}

		[[nodiscard]]
		bool has_pending() const
		{
			return !m_Queue.empty();
		}

	private:
		std::queue<TNode> m_Queue{};
	};
}

#endif
