add_executable(
	Simple-Graph-Example-Traverse_2d_grid
	"main.cpp"
)

target_link_libraries(
	Simple-Graph-Example-Traverse_2d_grid
	PRIVATE
	Simple-Graph::Simple-Graph
)
