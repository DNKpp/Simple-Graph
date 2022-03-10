//          Copyright Dominic Koepke 2019 - 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SIMPLE_QUEUE_HELPER_HPP
#define SIMPLE_QUEUE_HELPER_HPP

#pragma once

#include "utility.hpp"

#include <queue>

template <class... TArgs>
struct sl::graph::detail::take_next_func_t<std::priority_queue<TArgs...>>
{
	constexpr auto operator ()(std::priority_queue<TArgs...>& container) const
	{
		auto el{ std::move(container.top()) };
		container.pop();
		return el;
	}
};

template <class T, class TContainer>
struct sl::graph::detail::take_next_func_t<std::queue<T, TContainer>>
{
	constexpr T operator ()(std::queue<T, TContainer>& container) const
	{
		auto el{ std::move(container.front()) };
		container.pop();
		return el;
	}
};

#endif
