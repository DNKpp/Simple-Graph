//           Copyright Dominic Koepke 2022 - 2022.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)

#ifndef SIMPLE_QUEUE_HELPER_HPP
#define SIMPLE_QUEUE_HELPER_HPP

#pragma once

#include "utility.hpp"

#include <queue>

/** \addtogroup utility
* @{
*/

/**
 * \brief Specialization for std::stack.
 * \tparam TArgs Template arguments for std::priority_queue.
 */
template <class... TArgs>
struct sl::graph::take_next_t<std::priority_queue<TArgs...>>
{
	using container_t = std::priority_queue<TArgs...>;
	using value_t = typename container_t::value_type;

	/**
	 * \brief The invocation operator. Retrieves, removes and returns the next element.
	 * \param container The container object.
	 * \return Returns the next element.
	 */
	[[nodiscard]]
	constexpr value_t operator ()(container_t& container) const
	{
		auto el{ std::move(container.top()) };
		container.pop();
		return el;
	}
};

/**
 * \brief Specialization for std::stack.
 * \tparam TArgs Template arguments for std::queue.
 */
template <class... TArgs>
struct sl::graph::take_next_t<std::queue<TArgs...>>
{
	using container_t = std::queue<TArgs...>;
	using value_t = typename container_t::value_type;

	/**
	 * \brief The invocation operator. Retrieves, removes and returns the next element.
	 * \param container The container object.
	 * \return Returns the next element.
	 */
	[[nodiscard]]
	constexpr value_t operator ()(container_t& container) const
	{
		auto el{ std::move(container.front()) };
		container.pop();
		return el;
	}
};

/**
 * \brief Specialization for std::stack.
 * \tparam TArgs Template arguments for std::priority_queue.
 */
template <class... TArgs>
struct sl::graph::emplace_t<std::priority_queue<TArgs...>>
{
	using container_t = std::priority_queue<TArgs...>;

	/**
	 * \brief The invocation operator. Calls emplace on the container with the given arguments.
	 * \tparam TCTorArgs The types of the provided constructor arguments.
	 * \param container The container object.
	 * \param args The provided constructor arguments.
	 */
	template <class... TCTorArgs>
	constexpr void operator ()(container_t& container, TCTorArgs&&... args) const
	{
		container.emplace(std::forward<TCTorArgs>(args)...);
	}
};

/**
 * \brief Specialization for std::stack.
 * \tparam TArgs Template arguments for std::queue.
 */
template <class... TArgs>
struct sl::graph::emplace_t<std::queue<TArgs...>>
{
	using container_t = std::queue<TArgs...>;

	/**
	 * \brief The invocation operator. Calls emplace on the container with the given arguments.
	 * \tparam TCTorArgs The types of the provided constructor arguments.
	 * \param container The container object.
	 * \param args The provided constructor arguments.
	 */
	template <class... TCTorArgs>
	constexpr void operator ()(container_t& container, TCTorArgs&&... args) const
	{
		container.emplace(std::forward<TCTorArgs>(args)...);
	}
};

/** @}*/

#endif
