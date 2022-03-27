//           Copyright Dominic Koepke 2022 - 2022.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)

#ifndef SIMPLE_STACK_HELPER_HPP
#define SIMPLE_STACK_HELPER_HPP

#pragma once

#include "utility.hpp"

#include <stack>

template <class... TArgs>
struct sl::graph::take_next_t<std::stack<TArgs...>>
{
	using container_t = std::stack<TArgs...>;
	using value_t = typename container_t::value_type;

	[[nodiscard]]
	constexpr value_t operator ()(container_t& container) const
	{
		auto el{ std::move(container.top()) };
		container.pop();
		return el;
	}
};

template <class... TArgs>
struct sl::graph::emplace_t<std::stack<TArgs...>>
{
	using container_t = std::stack<TArgs...>;

	template <class... TCTorArgs>
	constexpr void operator ()(container_t& container, TCTorArgs&&... args) const
	{
		container.emplace(std::forward<TCTorArgs>(args)...);
	}
};

#endif
