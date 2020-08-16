cmake_minimum_required(VERSION 3.13 FATAL_ERROR)
project(simple_graph)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(
	${PROJECT_NAME}
	INTERFACE
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(
	${PROJECT_NAME}
	INTERFACE
	cxx_std_20
)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
	include(CTest)

	add_executable(
		test_simple_graph
		${CMAKE_CURRENT_SOURCE_DIR}/test/test.cpp
	)

	target_link_libraries(
		test_simple_graph
		PRIVATE
		simple_graph
	)

	enable_testing()
	add_test(
		NAME Simple-Graph_TestSuite
		COMMAND test_simple_graph
	)
endif()
