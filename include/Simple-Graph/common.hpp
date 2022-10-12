//          Copyright Dominic Koepke 2019 - 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SIMPLE_GRAPH_COMMON_HPP
#define SIMPLE_GRAPH_COMMON_HPP

#pragma once

#include <concepts>

namespace sl::graph::concepts
{
	template <class T>
	concept vertex = std::same_as<T, std::remove_cvref_t<T>>
					&& std::equality_comparable<T>
					&& std::copyable<T>;

	template <class T>
	concept weight = std::same_as<T, std::remove_cvref_t<T>>
					&& std::totally_ordered<T>
					&& std::regular<T>
					&& requires(T& w, const T& o)
					{
						w += o;
						{ o + o } -> std::convertible_to<T>;
					};
}

#endif
