//          Copyright Dominic Koepke 2019 - 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SIMPLE_GRAPH_UTILITY_HPP
#define SIMPLE_GRAPH_UTILITY_HPP

#pragma once

#include <compare>
#include <concepts>
#include <cstdint>
#include <optional>

namespace sl::graph
{
	enum class visit_state : std::uint8_t
	{
		none = 0,
		discovered,
		visited
	};

	template <class T>
	concept vertex_descriptor = std::equality_comparable<T>
								&& std::copyable<T>
								&& std::assignable_from<T&, T>;

	template <class T>
	concept weight = std::totally_ordered<T>
					&& requires(T t)
					{
						t += t;
						{ t + t } -> std::convertible_to<T>;
					};

	template <class T, class TVertex, class TState>
	concept state_map_for = vertex_descriptor<TVertex>
							&& std::equality_comparable<TState>
							&& std::copyable<TState>
							&& std::assignable_from<TState&, TState>
							&& requires(T map, TVertex v)
							{
								{ map[v] } -> std::convertible_to<TState>;
								{ map[v] } -> std::assignable_from<TState>;
							};

	template <vertex_descriptor TVertex, weight TWeight>
	struct weighted_node
	{
		std::optional<TVertex> predecessor{};
		TVertex vertex{};
		TWeight weight_sum{};

		constexpr std::compare_three_way_result<TWeight> operator <=>(const weighted_node& other) const noexcept
		{
			return weight_sum <=> other.weight_sum;
		}
	};

	struct empty_invokable
	{
		constexpr void operator ()(auto&&...) const noexcept
		{
		}
	};

	struct true_constant
	{
		constexpr bool operator ()(auto&&...) const noexcept
		{
			return true;
		}
	};
}

namespace sl::graph::detail
{
	template <class TFunc, class... TArgs>
	[[nodiscard]]
	constexpr bool shall_interrupt(TFunc&& func, TArgs&&... args)
	{
		if constexpr (std::predicate<TFunc, TArgs...>)
		{
			return std::invoke(std::forward<TFunc>(func), std::forward<TArgs>(args)...);
		}

		std::invoke(std::forward<TFunc>(func), std::forward<TArgs>(args)...);
		return false;
	}
}

#endif
