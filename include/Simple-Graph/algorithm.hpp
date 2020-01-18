#ifndef SL_GRAPH_ALGORITHM_HPP
#define SL_GRAPH_ALGORITHM_HPP

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

	template <class TVertex, class TNeighbourSearcher, class TVisitationTracker, class TPreOrderCallback = EmptyCallback, class TPostOrderCallback = EmptyCallback>
	void traverse_dfs(const TVertex& _begin, TNeighbourSearcher _neighbourSearcher, TVisitationTracker _visitationTracker,
	    TPreOrderCallback _preCB = TPreOrderCallback{}, TPostOrderCallback _postCB = TPostOrderCallback{})
	{
	    std::stack<TVertex> stack;
	    _visitationTracker[_begin] = true;
	    stack.push(_begin);
	    int depth = 0;
		if (_detail::shall_return(_preCB, _begin, depth))
			return;
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
	        	++depth;
	        	if (_detail::shall_return(_preCB, *child, depth))
					return;
	            _visitationTracker[*child] = true;
	            stack.push(*child);
	        }
	        else
	        {
	            if (_detail::shall_return(_postCB, v, depth))
					return;
	            --depth;
	            stack.pop();
	        }
	    }
	    assert(depth == -1);
	}

	template <class TVertex, class TNeighbourSearcher, class TVisitationTracker, class TCallback = EmptyCallback>
	void traverse_bfs(const TVertex& _begin, TNeighbourSearcher _neighbourSearcher, TVisitationTracker _visitationTracker,
	    TCallback _callback = TCallback{})
	{
	    struct Node
	    {
	        int depth;
	        TVertex vertex;
	    };
	    
	    std::deque<Node> openList{ { 0, _begin } };
	    _visitationTracker[_begin] = true;
	    while (!std::empty(openList))
	    {
	        auto node = openList.back();
	        openList.pop_back();
	        _callback(node.vertex, node.depth);
	        _neighbourSearcher(node.vertex,
	            [&_visitationTracker, &openList, depth = node.depth + 1](const TVertex& _vertex)
	            {
	                if (!_visitationTracker[_vertex])
	                {
	                    openList.emplace_front(Node{ depth, _vertex });
	                    _visitationTracker[_vertex] = true;
	                }
	            }
	        );
	    }
	}
}
#endif 