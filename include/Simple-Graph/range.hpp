//          Copyright Dominic Koepke 2019 - 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SIMPLE_GRAPH_RANGE_HPP
#define SIMPLE_GRAPH_RANGE_HPP

#pragma once

#include "Simple-Graph/searcher.hpp"

#include <optional>
#include <ranges>

namespace sl::graph
{
	template <class TAlgorithm, concepts::graph TGraph>
	class Range final
	{
	public:
		using node_type = typename TAlgorithm::node_type;
		using graph_type = TGraph;
		using vertex_type = graph_vertex_type<graph_type>;
		using searcher_type = detail::searcher_type<TAlgorithm, TGraph>;

		[[nodiscard]]
		constexpr Range(const graph_type& graph, const vertex_type& begin)
			: m_Searcher{graph, begin}
		{}

		class Sentinel final
		{};

		class Iterator final
		{
		public:
			using iterator_concept = std::input_iterator_tag;
			using element_type = node_type;
			using difference_type = std::ptrdiff_t;

			[[nodiscard]]
			explicit constexpr Iterator(searcher_type& algorithm)
				: m_Algorithm{std::addressof(algorithm)},
				m_CurrentNode{algorithm.next()}
			{ }

			constexpr ~Iterator() noexcept = default;

			constexpr Iterator(Iterator&&) = default;
			constexpr Iterator& operator =(Iterator&&) = default;

			constexpr Iterator(const Iterator&) = delete;
			constexpr Iterator& operator =(const Iterator&) = delete;

			constexpr Iterator& operator ++()
			{
				m_CurrentNode = m_Algorithm->next();

				return *this;
			}

			constexpr Iterator& operator ++(int)
			{
				return operator ++();
			}

			[[nodiscard]]
			constexpr const node_type& operator *() const
			{
				return *m_CurrentNode;
			}

			[[nodiscard]]
			constexpr bool operator ==(const Iterator&) const = default;

			[[nodiscard]]
			constexpr bool operator ==([[maybe_unused]] const Sentinel) const
			{
				return !m_CurrentNode;
			}

		private:
			searcher_type* m_Algorithm{};
			std::optional<node_type> m_CurrentNode{};
		};

		[[nodiscard]]
		constexpr Iterator begin()
		{
			return Iterator{m_Searcher};
		}

		[[nodiscard]]
		static constexpr Sentinel end()
		{
			return {};
		}

	private:
		searcher_type m_Searcher;
	};

	template <template <concepts::vertex> class TAlgorithm, concepts::graph TGraph>
	[[nodiscard]]
	constexpr Range<TAlgorithm<graph_vertex_type<TGraph>>, TGraph> make_range(
		const TGraph& graph,
		const graph_vertex_type<TGraph>& begin
	)
	{
		return {graph, begin};
	}

	template <template <concepts::vertex, concepts::weight> class TAlgorithm, concepts::weighted_graph TGraph>
	[[nodiscard]]
	constexpr Range<TAlgorithm<graph_vertex_type<TGraph>, graph_weight_type<TGraph>>, TGraph> make_range(
		const TGraph& graph,
		const graph_vertex_type<TGraph>& begin
	)
	{
		return {graph, begin};
	}
}

#endif
