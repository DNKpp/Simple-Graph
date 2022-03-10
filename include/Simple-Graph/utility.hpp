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
#include <functional>
#include <optional>

namespace sl::graph
{
	template <class>
	struct node_traits;

	template <class TNode>
	using node_vertex_t = typename node_traits<TNode>::vertex_type;

	template <class TNode>
	using node_weight_t = typename node_traits<TNode>::weight_type;

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
					&& std::regular<T>
					&& requires(T& w, const T& o)
					{
						w += o;
						{ o + o } -> std::convertible_to<T>;
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
		using vertex_type = TVertex;
		using weight_type = TWeight;

		std::optional<TVertex> predecessor{};
		TVertex vertex{};
		TWeight weight_sum{};

		[[nodiscard]]
		constexpr bool operator ==(const weighted_node& other) const noexcept = default;

		[[nodiscard]]
		constexpr std::compare_three_way_result_t<weight_type> operator <=>(const weighted_node& other) const noexcept
		{
			return weight_sum <=> other.weight_sum;
		}

		[[nodiscard]]
		explicit constexpr operator TWeight() const noexcept
		{
			return weight_sum;
		}
	};

	template <class TVertex, class TWeight>
	struct node_traits<weighted_node<TVertex, TWeight>>
	{
		using node_t = weighted_node<TVertex, TWeight>;
		using vertex_type = typename node_t::vertex_type;
		using weight_type = typename node_t::weight_type;
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

	template <class>
	struct take_next_func_t;

	template <class TContainer>
	constexpr decltype(auto) take_next(TContainer& container)
	{
		return take_next_func_t<std::remove_cvref_t<TContainer>>{}(container);
	}

	template <class TWeightCalculator, vertex_descriptor TVertex>
		requires std::invocable<TWeightCalculator, TVertex, TVertex>
	using weight_type_of_t = std::remove_cvref_t<std::invoke_result_t<TWeightCalculator, TVertex, TVertex>>;
}

#endif
