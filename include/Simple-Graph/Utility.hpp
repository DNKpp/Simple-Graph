//          Copyright Dominic Koepke 2019 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SL_GRAPH_UTILITY_HPP
#define SL_GRAPH_UTILITY_HPP

#pragma once

#include <type_traits>
#include <utility>
#include <concepts>

namespace sl::graph
{
	struct EmptyCallback
	{
		template <class... TArgs>
		constexpr void operator ()(TArgs&&...) const noexcept
		{
		}
	};
	
	enum class NodeState
	{
		none,
		open,
		closed
	};
}

namespace sl::graph::detail
{
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

	template <class T1, class T2>
	struct _IsNothrowComparableImpl :
		std::false_type
	{
	};

	template <class T1, class T2>
	requires std::equality_comparable_with<T1, T2>
	struct _IsNothrowComparableImpl<T1, T2> :
		std::bool_constant<noexcept(std::declval<T1>() == std::declval<T2>())>
	{
	};

	template <class T1, class T2 = T1>
	struct IsNothrowComparable :
		_IsNothrowComparableImpl<std::remove_cvref_t<T1>, std::remove_cvref_t<T2>>
	{
	};

	template <class T1, class T2 = T1>
	inline constexpr bool IsNothrowComparable_v = IsNothrowComparable<T1, T2>::value;

	template <class T>
	concept Vertex = std::copyable<T>;

	template <class T, class TVertex, class TNodeState>
	concept NeighbourSearcherWith = std::invocable<T, const TVertex&, const TNodeState&, DummyCallable<TVertex>>;

	template <class T, class TVertex, class TNodeState>
	concept StateMapWith = requires(std::remove_cvref_t<T>& stateMap)
	{
		{
			stateMap[std::declval<TVertex>()]
		} -> std::same_as<std::add_lvalue_reference_t<TNodeState>>;
	};
}

#endif
