//           Copyright Dominic Koepke 2022 - 2022.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)

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

	template <class T, class TVertex>
	concept neighbor_searcher_for = vertex_descriptor<TVertex>
									&& std::invocable<T, TVertex>
									&& std::ranges::input_range<std::invoke_result_t<T, TVertex>>;

	template <class T, class TNode>
	concept vertex_predicate_for = std::predicate<T, TNode, node_vertex_t<TNode>>;

	template <class T, class TNode>
	concept node_callback = std::invocable<T, TNode>;

	template <class T, class TVertex>
	concept weight_calculator_for = vertex_descriptor<TVertex>
									&& std::invocable<T, TVertex, TVertex>;

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

	struct empty_invokable_t
	{
		constexpr void operator ()(auto&&...) const noexcept
		{
		}
	};

	template <auto VValue>
	struct constant_t
	{
		using value_type = decltype(VValue);
		static constexpr value_type value{ VValue };

		constexpr value_type operator()(auto&&...) const { return VValue; }
	};

	using true_constant_t = constant_t<true>;

	template <class>
	struct take_next_t;

	template <class TContainer>
	[[nodiscard]]
	constexpr decltype(auto) take_next(TContainer& container)
	{
		return take_next_t<std::remove_cvref_t<TContainer>>{}(container);
	}

	template <class>
	struct emplace_t;

	template <class TContainer, class... TCTorArgs>
	constexpr void emplace(TContainer& container, TCTorArgs&&... args)
	{
		return emplace_t<std::remove_cvref_t<TContainer>>{}(container, std::forward<TCTorArgs>(args)...);
	}
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

	template <class TWeightCalculator, vertex_descriptor TVertex>
		requires weight_calculator_for<TWeightCalculator, TVertex>
	using weight_type_of_t = std::remove_cvref_t<std::invoke_result_t<TWeightCalculator, TVertex, TVertex>>;

	template <vertex_descriptor TVertex, std::invocable<TVertex, TVertex> TWeightCalculator>
	struct weighted_node_factory_t
	{
		using vertex_t = TVertex;
		using weight_t = weight_type_of_t<TWeightCalculator, TVertex>;
		using node_t = weighted_node<vertex_t, weight_t>;

		TWeightCalculator weightCalculator{};

		constexpr node_t operator ()(const node_t& predecessor, const TVertex& cur_vertex)
		{
			weight_t rel_weight{ std::invoke(weightCalculator, predecessor.vertex, cur_vertex) };
			assert(weight_t{} <= rel_weight && "relative weight between nodes must be greater or equal zero.");

			return
			{
				.predecessor = predecessor.vertex,
				.vertex = cur_vertex,
				.weight_sum = predecessor.weight_sum + rel_weight
			};
		}
	};

	template <vertex_descriptor TVertex, class TWeightCalculator>
	[[nodiscard]]
	constexpr weighted_node_factory_t<TVertex, TWeightCalculator> make_weighted_node_factory(TWeightCalculator&& weightCalc)
	{
		return { .weightCalculator = std::forward<TWeightCalculator>(weightCalc) };
	}
}

namespace sl::graph
{
	template <class T, class TNode>
	concept open_list_for = requires(const T& container) { std::empty(container); }
							&& requires(T& container)
							{
								emplace(container, std::declval<TNode>());
								{ take_next(container) } -> std::convertible_to<TNode>;
							};
}

#endif
