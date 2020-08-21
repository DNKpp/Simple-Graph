//          Copyright Dominic Koepke 2019 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)


#ifndef SL_GRAPH_ALGORITHM_HPP
#define SL_GRAPH_ALGORITHM_HPP

#pragma once

#include <stack>
#include <vector>
#include <algorithm>
#include <cassert>
#include <deque>
#include <queue>
#include <optional>

namespace sl::graph
{
	template <class TVertex>
	struct DFSNode
	{
		int depth;
		TVertex vertex;

		[[nodiscard]] bool operator ==(const DFSNode&) const = default;
	};

	template <class TVertex>
	using BFSNode = DFSNode<TVertex>;
	
	struct EmptyCallback
	{
	    template <class... Args>
	    constexpr void operator ()(Args&&...) const noexcept {}  
	};

namespace _detail
{
	template <class TFunction, class... TParams>
	bool shall_return(TFunction& _func, TParams... _params)
	{
		if constexpr (std::is_same_v<bool, std::invoke_result_t<decltype(_func), TParams...>>)
		{
			return _func(std::forward<TParams>(_params)...);
		}
		else
		{
			_func(std::forward<TParams>(_params)...);
			return false;
		}
	}
}

	template <class TVertex, class TNeighbourSearcher, class TVisitationTracker, class TCallback = EmptyCallback>
	void traverse_bfs(const TVertex& _begin, TNeighbourSearcher _neighbourSearcher, TVisitationTracker _visitationTracker,
	    TCallback _callback = TCallback{})
	{
	    using Node = BFSNode<TVertex>;
	    
	    std::deque<Node> openList{ { 0, _begin } };
	    _visitationTracker[_begin] = true;
	    while (!std::empty(openList))
	    {
	        auto node = openList.back();
	        openList.pop_back();
	        if (_detail::shall_return(_callback, node))
				return;
	        _neighbourSearcher(node,
	            [&_visitationTracker, &openList, depth = node.depth + 1](const TVertex& _vertex)
	            {
	        		auto&& check = _visitationTracker[_vertex];
	                if (!check)
	                {
	                    openList.emplace_front(Node{ depth, _vertex });
	                    check = true;
	                }
	            }
	        );
	    }
	}
}
#endif 
