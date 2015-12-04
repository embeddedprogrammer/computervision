cmake_minimum_required(VERSION 2.8)

project(TIPSetup)
find_package(OpenCV REQUIRED)
add_executable(TIPSetup TipSetup.cpp)
target_link_libraries(TIPSetup ${OpenCV_LIBS})

project(GraphCut)
find_package(OpenCV REQUIRED)
add_executable(GraphCut
	GraphCut.cpp
	block.h
	graph.h graph.cpp
	instances.inc 
	maxflow.cpp)
target_link_libraries(GraphCut ${OpenCV_LIBS})
