cmake_minimum_required(VERSION 2.8)

project(Lab2)
find_package(OpenCV REQUIRED)
add_executable(Lab2
	Lab2.cpp
	block.h
	graph.h graph.cpp
	instances.inc 
	maxflow.cpp)
target_link_libraries(Lab2 ${OpenCV_LIBS} )
