# Simple-Graph C++20 header-only library

[![run tests](https://github.com/DNKpp/Simple-Graph/actions/workflows/run_tests.yml/badge.svg)](https://github.com/DNKpp/Simple-Graph/actions/workflows/run_tests.yml)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/800b2ba82cd047b5b2c53387750dec87)](https://www.codacy.com/gh/DNKpp/Simple-Graph/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=DNKpp/Simple-Graph&amp;utm_campaign=Badge_Grade)
[![Codacy Badge](https://app.codacy.com/project/badge/Coverage/800b2ba82cd047b5b2c53387750dec87)](https://www.codacy.com/gh/DNKpp/Simple-Graph/dashboard?utm_source=github.com&utm_medium=referral&utm_content=DNKpp/Simple-Graph&utm_campaign=Badge_Coverage)
[![codecov](https://codecov.io/gh/DNKpp/Simple-Graph/branch/master/graph/badge.svg?token=CIJMPLQCMA)](https://codecov.io/gh/DNKpp/Simple-Graph)

## Author
Dominic (DNKpp) Koepke  
Mail: [DNKpp2011@gmail.com](mailto:dnkpp2011@gmail.com)

## License

[BSL-1.0](https://github.com/DNKpp/Simple-Log/blob/master/LICENSE_1_0.txt) (free, open source)

```text
          Copyright Dominic Koepke 2019 - 2022.
 Distributed under the Boost Software License, Version 1.0.
    (See accompanying file LICENSE_1_0.txt or copy at
          https://www.boost.org/LICENSE_1_0.txt)
```

## Description
This library provides implementations for the following graph algorithms:
*   depth first search -> depth_first_search.hpp
*   breadth first search -> breadth_first_search.hpp
*   dijkstra -> dijkstra.hpp
*   astar (A*) -> astar.hpp

The library is designed to require a minimal setup for the users, to make any of the algorithm work in their scenarios. This means one can rely on tested algorithms instead of implementing, and maintaining, their own implementations.
Sure, the minimal setup won't be as fast as one could get with a custom implementation, but users are able to exchange almost any type used of the algorithms and therefore probably come very close to it.
For example by default all of the algorithm are using a ``std::map`` to track the states of each node, but users may simply exchange that type with a custom one. For example in a 2d grid environment one could use a boolean-2d-grid type,
which will than have a constant speed; a huge speed-up, with minimal effort.

Another example are the ``neighbor searcher``. Those searchers have the task, to hand back all adjacent vertices to the current vertex. A simple and straight forward solution would be to return a ``std::vector`` filled with all neighbors. But the requirements of the
library are more relaxed: It only requires a ``std::ranges::input_range``. A more performant, but of course also more complex, solution could return a custom view, returning each neighbor one by one. This way users get rid of many dynamic allocations, which probably
result in far better performance.

This being said, I think its a good time to remind on the commonly known sentence ``premature optimization is the root of all evil``, thus I think its a good idea getting the feature running and tweak it afterwards if necessary. And exactly this is what I try to
achieve with this library.

### Library Terminologies
#### Vertex
A vertex is an object, which uniquely identifies a node of a graph. This may be an integer, a string, a vector or what-ever suits your needs.

For the technical requirements have a look at the ``sl::graph::vertex_descriptor`` concept.

### Neighbor Searcher
The neighbor searcher is an invokable object, which is responsible to provide a range of adjacent vertices to the given vertex. This range must satisfy the requirements of a ``std::ranges::input_range``, thus it may be either a final collection or lazy filled during iteration.

For the technical requirements have a look at the ``sl::graph::neighbor_searcher_for`` concept.

### (Node)Callback
The library uses the term ``callback`` and ``node callback`` interchangeably. These callbacks will be invoked for each finally visited node. If the callback returns a boolean-convertible type, the algorithm will stop if ``true`` is returned. Any other returned type will be
discarded and ignored, thus returning ``void`` is also possible.

For the technical requirements have a look at the ``sl::graph::node_callback`` concept.

### Vertex Predicate
As the ``neighbor searcher`` is responsible returning a range of adjacent vertices, the ``vertex predicate`` will be invoked for each such retrieved vertex and must return a boolean-convertible type. If ``false`` is returned, the vertex will be skipped.
It is not required that the predicate always yield the same result for invocations with the same set of arguments.

If users do not provide such a predice, the default predicate will return ``true`` for each vertex.

For the technical requirements have a look at: ``sl::graph::vertex_predicate_for`` concept.

### State Map
The state map will be queried with vertices and is expected to return the state. The state map must always return a valid reference (or proxy) to an existing state, even if the queried vertex has never been set before (in that case, a default constructed state will do fine).
The algorithm tracks the visitation state of each vertex in it.

For the technical requirements have a look at: ``sl::graph::state_map_for`` concept.

### Open List
The open list serves as a container for discovered, but not yet visited, nodes. Even if user provided, at the beginning the open list must be empty.

The library makes usage of several abstractions, which users may specialize to make their custom types working.

For the technical requirements have a look at: ``sl::graph::open_list_for`` concept.

### Weight Calculator
Some algorithms expects the ``weight`` between two adjacent vertices to be dynamically retrieved. During one execution of an algorithm, the ``weight calculator`` should always return the same value for the same set of arguments, thus the weight between two vertices
should be constant during the whole time the algorithm runs. Note that the weights for different directions (going from ``A -> B`` or ``B -> A``) is not required to be equally.

For the technical requirements have a look at: ``sl::graph::weight_calculator_for`` concept.

### Heuristic
The ``heuristic`` is an invocable object, which should return the estimated cost left, to travel from the current vertex to the destination. The return type must be convertible to the type returned by ``weight calculator``.

For the technical requirements have a look at: ``sl::graph::astar::compatible_heuristic_for`` concept.

## Tested Compilers
*   msvc v142 (Visual Studio 2017)
*   msvc v143 (Visual Studio 2022)
*   gcc10
*   gcc11

As this library heavily relys on c++20 features, which clang(-cl) doesn't fully support yet, clang isn't listed above.

## Example
Have a look into the examples directory.

## Installation with CMake
This library can easily be integrated into your project via CMake target_link_libraries command.

```cmake
target_link_libraries(
	<your_target_name>
	PRIVATE
	Simple-Graph::Simple-Graph
)
```
This will add the the include path "<simple_vector_install_dir>/include", thus you are able to include the headers via
```cpp
#include <Simple-Graph/astar.hpp>
```

### FetchContent
It is possible fetching this library via CMakes FetchContent module.

```cmake
cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(<your_project_name>)

include(FetchContent)

FetchContent_Declare(
	Simple_Graph
	GIT_REPOSITORY	https://github.com/DNKpp/Simple-Graph
	GIT_TAG		origin/vx.y.z
)
FetchContent_MakeAvailable(Simple_Graph)

target_link_libraries(
	<your_target_name>
	PUBLIC
	Simple-Graph::Simple-Graph
)
```

### CPM
The [CPM](https://github.com/cpm-cmake/CPM.cmake) CMake module is a featureful wrapper around the ``FetchContent`` module. To use it simply add ``CPM.cmake`` or ``get_cmp.make`` (which will pull ``CPM.cmake`` on the fly)
from the latest release into your project folder and include it into your ``CMakeLists.txt``. If you need an example, have a look how this library uses cpm.

```cmake
include(CPM.cmake) # or include(get_cpm.cmake)

CPMAddPackage("gh:DNKpp/Simple-Graph#vX.Y.Z")
# do not forget linking via target_link_libraries as shown above
```
