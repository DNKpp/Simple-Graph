cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(Simple-Graph
	LANGUAGES CXX
	VERSION 1.0.0
)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/")
include("get_cpm")

add_library(${PROJECT_NAME} INTERFACE)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
target_include_directories(
	${PROJECT_NAME}
	INTERFACE
	"${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_compile_features(
	${PROJECT_NAME}
	INTERFACE
	cxx_std_20
	# msvc disabled ranges for c++20; force /std:c++latest instead
	$<$<CXX_COMPILER_ID:MSVC>:cxx_std_23>
)

OPTION(${PROJECT_NAME}_BUILD_TESTS "Determines whether tests will be build or not." OFF)
if (${PROJECT_NAME}_BUILD_TESTS)
	add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/tests")
endif()
