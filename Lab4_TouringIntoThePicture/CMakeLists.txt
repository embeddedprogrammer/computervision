cmake_minimum_required(VERSION 2.8)

project(TIPSetup)
find_package(OpenCV REQUIRED)
add_executable(TIPSetup TipSetup.cpp)
target_link_libraries(TIPSetup ${OpenCV_LIBS} )

#project(Lab2)
#find_package(OpenCV REQUIRED)
#add_executable(Lab2
#	GraphCut.cpp
#	block.h
#	graph.h graph.cpp
#	instances.inc 
#	maxflow.cpp)
#target_link_libraries(Lab2 ${OpenCV_LIBS} )
