cmake_minimum_required(VERSION 2.8)

project(granite)

if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "-std=c++0x -Wall -msse -msse2 -msse3")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif()

include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/ext/glfw/include")
link_directories("${PROJECT_SOURCE_DIR}/ext/glfw/build/src")

add_subdirectory(base)
add_subdirectory(system)
#add_subdirectory(engine)
add_subdirectory(tests)
