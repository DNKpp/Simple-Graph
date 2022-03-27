# Simple-Vector C++20 library

[![run tests](https://github.com/DNKpp/Simple-Vector/actions/workflows/run_tests.yml/badge.svg)](https://github.com/DNKpp/Simple-Vector/actions/workflows/run_tests.yml)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/ba448bbe4bc04b6289e24d302b68ef44)](https://www.codacy.com/gh/DNKpp/Simple-Vector/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=DNKpp/Simple-Vector&amp;utm_campaign=Badge_Grade)
[![codecov](https://codecov.io/gh/DNKpp/Simple-Vector/branch/master/graph/badge.svg?token=4ZKAEU6V5A)](https://codecov.io/gh/DNKpp/Simple-Vector)

## Author
Dominic Koepke  
Mail: [DNKpp2011@gmail.com](mailto:dnkpp2011@gmail.com)

## License

[BSL-1.0](https://github.com/DNKpp/Simple-Log/blob/master/LICENSE_1_0.txt) (free, open source)

```text
          Copyright Dominic Koepke 2021 - 2022.
 Distributed under the Boost Software License, Version 1.0.
    (See accompanying file LICENSE_1_0.txt or copy at
          https://www.boost.org/LICENSE_1_0.txt)
```

## Description
This library provides an implementation of a simple to use but versatile mathematically vector class. Due to the usage of lots of c++20 features, this library currently doesn't compile on any clang compiler.

Tested Compilers:
*   msvc v143 (Visual Studio 2022)
*   gcc10
*   gcc11

One of the main intentions behind this vector implementation is being able to do as much during compile-time as possible, thus all functions are declared as constexpr.

## Example
```cpp
/* With inclusion of this header, everything of the library will be available.*/
#include <Simple-Vector/Simple-Vector.hpp>

#include <iostream>
#include <ranges>

int main()
{
	// make the Vector class locally available with less verbosity 
	using sl::vec::Vector;
	
	Vector<int, 2> intVec1;			// this will default construct all elements to 0
	intVec1 += 2;					// adds 2 to each element
	
	Vector<int, 2> intVec2{ 1, 3 }; // constructs a new vector with x == 1 and y == 3

	Vector difference = static_cast<Vector<float, 2>>(intVec1 - intVec2);	// binary vector + and - arithmetic is possible as well as casting to vectors with other value types
	auto distance = length(difference);										// length is a free function, which is luckily available due to ADL even if it sits in the sl::vec namespace
	
	auto tmp = difference.x();					// each vector has a .x() member function, which returns a ref to the first element
	tmp = difference.y();						// vectors with two or more dimensions also have a y.() function; lastly vectors with three or more dimensions have a .z() function
	tmp = difference[0];						// but users may also access the elements via operator []

	for (auto el : difference)					// or simply iterate over each element
	{
		std::cout << el << " ";
	}
	std::cout << std::endl;

	for (auto el : intVec2 | std::views::take(1)) // vectors are also fully compatible to the new ranges or the older stl algorithms
	{
		std::cout << el;
	}
	
	Vector doubles{ 42. };		// this will be deduced to Vector<double, 1>
	auto doubles2 = static_cast<Vector<double, 3>>(doubles);		// casting between different dimensions is also possible
}
```

## Installation with CMake
This library can easily be integrated into your project via CMake target_link_libraries command.

```cmake
target_link_libraries(
	<your_target_name>
	PRIVATE
	Simple-Vector::Simple-Vector
)
```
This will add the the include path "<simple_vector_install_dir>/include", thus you are able to include the headers via
```cpp
#include <Simple-Vector/Simple-Vector.hpp>
```

### FetchContent
It is possible fetching this library via CMakes FetchContent module.

```cmake
cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(<your_project_name>)

include(FetchContent)

FetchContent_Declare(
	Simple_Vector
	GIT_REPOSITORY	https://github.com/DNKpp/Simple-Vector
	GIT_TAG		origin/vx.y.z
)
FetchContent_MakeAvailable(Simple_Vector)

target_link_libraries(
	<your_target_name>
	PUBLIC
	Simple-Vector::Simple-Vector
)
```
