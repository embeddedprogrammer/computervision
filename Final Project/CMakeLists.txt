cmake_minimum_required(VERSION 2.8)

project(RRansac)
find_package(OpenCV REQUIRED)
add_executable(RRansac RRansac.cpp)
target_link_libraries(RRansac ${OpenCV_LIBS} )
