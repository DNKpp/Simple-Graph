cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

include(CTest)

CPMAddPackage("gh:catchorg/Catch2@3.1.1")
CPMAddPackage("gh:DNKpp/Simple-Vector@1.3.0")

add_executable(
	${PROJECT_NAME}-Tests
)

target_include_directories(
	${PROJECT_NAME}-Tests
	PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(
	${PROJECT_NAME}-Tests
	PRIVATE
	${PROJECT_NAME}::${PROJECT_NAME}
	Simple-Vector::Simple-Vector
	Catch2::Catch2WithMain
)

add_subdirectory(src)

add_test(${PROJECT_NAME}-TestSuite ${PROJECT_NAME}-Tests)
