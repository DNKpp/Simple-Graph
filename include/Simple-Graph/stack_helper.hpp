//           Copyright Dominic Koepke 2022 - 2022.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)

#ifndef SIMPLE_STACK_HELPER_HPP
#define SIMPLE_STACK_HELPER_HPP

#pragma once

#include "utility.hpp"

#include <stack>

/** \addtogroup utility
* @{
*/

/**
 * \brief Specialization for std::stack.
 * \tparam TArgs Template arguments for std::stack.
 */
template <class... TArgs>
struct sl::graph::take_next_t<std::stack<TArgs...>>
{
	using container_t = std::stack<TArgs...>;
	using value_t = typename container_t::value_type;

	/**
	 * \brief The invocation operator. Retrieves, removes and returns the top element.
	 * \param container The container object.
	 * \return Returns the previous top element.
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
 * \tparam TArgs Template arguments for std::stack.
 */
template <class... TArgs>
struct sl::graph::emplace_t<std::stack<TArgs...>>
{
	using container_t = std::stack<TArgs...>;

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
