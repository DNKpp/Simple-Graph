//          Copyright Dominic Koepke 2019 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SL_GRAPH_UTILITY_HPP
#define SL_GRAPH_UTILITY_HPP

#pragma once

#include <cassert>
#include <type_traits>
#include <utility>

namespace sl::graph
{
	enum class NodeState
	{
		none,
		open,
		closed
	};
}

namespace sl::graph::detail
{
	struct EmptyCallback
	{
		template <class... TArgs>
		constexpr void operator ()(TArgs&&...) const noexcept
		{
		}
	};

	template <class TVertex>
	struct DummyCallable
	{
		void operator ()(const TVertex&)
		{
		}
	};

	template <class TFunction, class... TParams>
	bool shallReturn(TFunction& func, TParams&&... params)
	{
		if constexpr (std::is_same_v<bool, std::invoke_result_t<decltype(func), TParams...>>)
		{
			return func(std::forward<TParams>(params)...);
		}
		else
		{
			func(std::forward<TParams>(params)...);
			return false;
		}
	}
}
#endif
