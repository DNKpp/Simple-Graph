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
	template <class TVertex>
	struct DFSNode
	{
		int depth;
		TVertex vertex;
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

	template <class TVertex, class TNeighbourSearcher, class TVisitationTracker, class TPreOrderCallback = EmptyCallback, class TPostOrderCallback = EmptyCallback>
	void traverse_dfs(const TVertex& _begin, TNeighbourSearcher _neighbourSearcher, TVisitationTracker _visitationTracker,
	    TPreOrderCallback _preCB = TPreOrderCallback{}, TPostOrderCallback _postCB = TPostOrderCallback{})
	{
		using Node = DFSNode<TVertex>;
	
	    std::stack<TVertex> stack;
	    _visitationTracker[_begin] = true;
	    stack.push(_begin);
	    int depth = 0;
		if (_detail::shall_return(_preCB, Node{ depth, _begin }))
			return;
	    while (!std::empty(stack))
	    {
	        auto v = stack.top();
	        if (auto child = _neighbourSearcher(Node{ depth, v },
	            [&_visitationTracker](const TVertex& _vertex)
	            {
	                return !_visitationTracker[_vertex];
	            }
	        ))
	        {
	        	++depth;
	        	if (_detail::shall_return(_preCB, Node{ depth, *child }))
					return;
	            _visitationTracker[*child] = true;
	            stack.push(*child);
	        }
	        else
	        {
	            if (_detail::shall_return(_postCB, Node{ depth, v }))
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
