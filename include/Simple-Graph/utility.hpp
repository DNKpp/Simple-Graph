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
#include <ranges>

namespace sl::graph
{
	/** \addtogroup utility
	 * @{
	 * \brief A collection of utilities for several parts of the library.
	 */

	/**
	 * \brief Basic helper type for convenient access to the node's properties.  
	 * \tparam TNode The node type.
	 */
	template <class TNode>
	struct node_traits;

	/**
	 * \brief Helper alias for convenient access to the node's vertex type. 
	 * \tparam TNode The node type.
	 */
	template <class TNode>
	using node_vertex_t = typename node_traits<TNode>::vertex_type;

	/**
	 * \brief Helper alias for convenient access to the node's weight type. 
	 * \tparam TNode The node type.
	 */
	template <class TNode>
	using node_weight_t = typename node_traits<TNode>::weight_type;

	/**
	 * \brief State type of some of the implemented algorithms.
	 */
	enum class visit_state : std::uint8_t
	{
		/**
		 * \brief Not yet discovered.
		 */
		none = 0,
		/**
		 * \brief Discovered but not yet visited.
		 */
		discovered,
		/**
		 * \brief Visited, thus finalized.
		 */
		visited
	};

	/**
	 * \brief Checks whether a type satisfies the requirements for a vertex type.
	 * \tparam T Type to check.
	 */
	template <class T>
	concept vertex_descriptor = std::equality_comparable<T>
								&& std::copyable<T>
								&& std::assignable_from<T&, T>;

	/**
	 * \brief Checks whether a type satisfies the requirements for a weight type.
	 * \tparam T Type to check.
	 */
	template <class T>
	concept weight = std::totally_ordered<T>
					&& std::regular<T>
					&& requires(T& w, const T& o)
					{
						w += o;
						{ o + o } -> std::convertible_to<T>;
					};

	/**
	 * \brief Checks whether a type satisfies the requirements for a state map type.
	 * \tparam T Type to check.
	 * \tparam TVertex The used vertex type.
	 * \tparam TState The used state type.
	 */
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

	/**
	 * \brief Checks whether a type satisfies the requirements for a neighbor searcher type.
	 * \tparam T Type to check.
	 * \tparam TVertex The used vertex type.
	 */
	template <class T, class TVertex>
	concept neighbor_searcher_for = vertex_descriptor<TVertex>
									&& std::invocable<T, TVertex>
									&& std::ranges::input_range<std::invoke_result_t<T, TVertex>>;

	/**
	 * \brief Checks whether a type satisfies the requirements for a vertex predicate type.
	 * \tparam T Type to check.
	 * \tparam TNode The used node type.
	 */
	template <class T, class TNode>
	concept vertex_predicate_for = std::predicate<T, TNode, node_vertex_t<TNode>>;

	/**
	 * \brief Checks whether a type satisfies the requirements for a node callback type.
	 * \tparam T Type to check.
	 * \tparam TNode The used node type.
	 */
	template <class T, class TNode>
	concept node_callback = std::invocable<T, TNode>;

	/**
	 * \brief Checks whether a type satisfies the requirements for a weight calculator type.
	 * \tparam T Type to check.
	 * \tparam TVertex The used vertex type.
	 */
	template <class T, class TVertex>
	concept weight_calculator_for = vertex_descriptor<TVertex>
									&& std::invocable<T, TVertex, TVertex>;

	/**
	 * \brief Node type used by several algorithms.
	 * \tparam TVertex The used vertex type.
	 * \tparam TWeight The used weight type.
	 */
	template <vertex_descriptor TVertex, weight TWeight>
	struct weighted_node
	{
		/**
		 * \brief Alias for the given vertex type.
		 */
		using vertex_type = TVertex;

		/**
		 * \brief Alias for the given weight type.
		 */
		using weight_type = TWeight;

		/**
		 * \brief The predecessor of the current node, if any.
		 */
		std::optional<TVertex> predecessor{};

		/**
		 * \brief The vertex of the current node.
		 */
		TVertex vertex{};

		/**
		 * \brief The summed weight needed reaching the vertex.
		 */
		TWeight weight_sum{};

		/**
		 * \brief Default equality operator.
		 * \param other The other node.
		 * \return Returns true if both nodes compare equally.
		 */
		[[nodiscard]]
		constexpr bool operator ==(const weighted_node& other) const noexcept = default;

		/**
		 * \brief Three-way-comparison operator.
		 * \param other The other node.
		 * \return Returns the three-way-comparison result of the member weight_sum.
		 */
		[[nodiscard]]
		constexpr std::compare_three_way_result_t<weight_type> operator <=>(const weighted_node& other) const noexcept
		{
			return weight_sum <=> other.weight_sum;
		}

		/**
		 * \brief Conversion operator to the weight type.
		 */
		[[nodiscard]]
		explicit constexpr operator TWeight() const noexcept
		{
			return weight_sum;
		}
	};

	/**
	 * \brief Specialized for weighted_node types.
	 * \tparam TVertex The used vertex type.
	 * \tparam TWeight The used weight type.
	 */
	template <class TVertex, class TWeight>
	struct node_traits<weighted_node<TVertex, TWeight>>
	{
		using node_t = weighted_node<TVertex, TWeight>;
		using vertex_type = typename node_t::vertex_type;
		using weight_type = typename node_t::weight_type;
	};

	/**
	 * \brief Helper type which may be invoked with arbitrary parameters, and does and returns nothing.
	 */
	struct empty_invokable_t
	{
		/**
		 * \brief Invoke operator.
		 */
		constexpr void operator ()(auto&&...) const noexcept
		{
		}
	};

	/**
	 * \brief Helper type which may be invoked with arbitrary parameters and returns a copy of VValue.
	 * \tparam VValue The value to be returned.
	 */
	template <auto VValue>
	struct constant_t
	{
		/**
		 * \brief Alias to the type of VValue.
		 */
		using value_type = decltype(VValue);

		/**
		 * \brief Provides access to the used value.
		 */
		static constexpr value_type value{ VValue };

		/**
		 * \brief Invoke operator.
		 * \return Returns always a copy of VValue.
		 */
		constexpr value_type operator()(auto&&...) const { return VValue; }
	};

	/**
	 * \brief Alias type, which always returns true.
	 */
	using true_constant_t = constant_t<true>;

	/**
	 * \brief Basic helper type which serves as an abstraction, thus users may use their own container types.
	 * \details This type may be specialized by users. The specialized type should at least contain the invoke operator
	 * matching the following signature.
	 * \code
	 * value_t operator ()(container_t& container)
	 * \endcode
	 * The invoke operator must remove the next element from the container and return it.
	 * \tparam TContainer The container type.
	 */
	template <class TContainer>
	struct take_next_t;

	/**
	 * \brief Helper function which forwards its parameters to the appropriate take_next_t specialization.
	 * \tparam TContainer The container type.
	 * \param container The container object.
	 * \return Simply forwards the returned object as given from the specialization.
	 */
	template <class TContainer>
	[[nodiscard]]
	constexpr decltype(auto) take_next(TContainer& container)
	{
		return take_next_t<std::remove_cvref_t<TContainer>>{}(container);
	}

	/**
	 * \brief Basic helper type which serves as an abstraction, thus users may use their own container types.
	 * \details This type may be specialized by users. The specialized type should at least contain the invoke operator
	 * matching the following signature, where TNode is the node type of the used algorithm.
	 * \code
	 * void operator ()(TContainer& container, TNode&& node)
	 * \endcode
	 * The invoke operator must push the given node to the container.
	 * \tparam TContainer The container type.
	 */
	template <class TContainer>
	struct emplace_t;

	/**
	 * \brief Helper function which forwards its parameters to the appropriate emplace_t specialization.
	 * \tparam TContainer The container type.
	 * \tparam TCTorArgs The constructor argument types.
	 * \param container The container object.
	 * \param args The construct arguments.
	 */
	template <class TContainer, class... TCTorArgs>
	constexpr void emplace(TContainer& container, TCTorArgs&&... args)
	{
		emplace_t<std::remove_cvref_t<TContainer>>{}(container, std::forward<TCTorArgs>(args)...);
	}

	/** @}*/
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

	[[nodiscard]]
	constexpr decltype(auto) empty_helper(const auto& container)
	{
		using std::empty;
		return empty(container);
	}
}

namespace sl::graph
{
	/** \addtogroup utility
	 * @{
	 */

	/**
	 * \brief Checks whether a type satisfies the requirements for an open list type.
	 * \tparam T Type to check.
	 * \tparam TNode The used node type.
	 */
	template <class T, class TNode>
	concept open_list_for = requires(const T& container)
							{
								{ detail::empty_helper(container) } -> std::convertible_to<bool>;
							}
							&& requires(T& container)
							{
								emplace(container, std::declval<TNode>());
								{ take_next(container) } -> std::convertible_to<TNode>;
							};

	/** @}*/
}

#endif
