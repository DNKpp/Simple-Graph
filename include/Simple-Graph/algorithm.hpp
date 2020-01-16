#ifndef SL_ALGORITHM_HPP
#define SL_ALGORITHM_HPP

#pragma once

#include <stack>
#include <vector>
#include <algorithm>
#include <cassert>
#include <deque>
#include <queue>

namespace sl::graph
{
	struct EmptyCallback
	{
	    template <class TVertex>
	    constexpr void operator ()(const TVertex&, int) const {}  
	};

	template <class TVertex, class TNeighbourSearcher, class TVisitationTracker, class TPreOrderCallback = EmptyCallback, class TPostOrderCallback = EmptyCallback>
	void traverse_dfs(const TVertex& _begin, TNeighbourSearcher _neighbourSearcher, TVisitationTracker _visitationTracker,
	    TPreOrderCallback _preCB = TPreOrderCallback{}, TPostOrderCallback _postCB = TPostOrderCallback{})
	{
	    std::stack<TVertex> stack;
	    _visitationTracker[_begin] = true;
	    stack.push(_begin);
	    int depth = 0;
		_preCB(_begin, depth);
	    while (!std::empty(stack))
	    {
	        auto v = stack.top();
	        if (auto child = _neighbourSearcher(v,
	            [&_visitationTracker](const TVertex& _vertex)
	            {
	                return !_visitationTracker[_vertex];
	            }
	        ))
	        {
	            _visitationTracker[*child] = true;
	            stack.push(*child);
	            ++depth;
	        	_preCB(stack.top(), depth);
	        }
	        else
	        {
	            _postCB(v, depth);
	            --depth;
	            stack.pop();
	        }
	    }
	    assert(depth == -1);
	}
}
#endif 
