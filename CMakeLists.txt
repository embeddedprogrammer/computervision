cmake_minimum_required(VERSION 2.8)

#project( Graph )
#add_executable( Graph graph.cpp )


project( DisplayImage )
find_package( OpenCV REQUIRED )
add_executable(DisplayImage
	DisplayImage.cpp
	block.h
	graph.h graph.cpp
	instances.inc 
	maxflow.cpp)
target_link_libraries( DisplayImage ${OpenCV_LIBS} )

