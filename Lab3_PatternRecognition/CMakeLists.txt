cmake_minimum_required(VERSION 2.8)

project(Pattern)
find_package(OpenCV REQUIRED)
add_executable(Pattern Pattern.cpp)
target_link_libraries(Pattern ${OpenCV_LIBS})
