cmake_minimum_required(VERSION 2.8)

project(granite)

option(ENABLE_SSSE3 "Enable SSSE3 optimizations" ON)

if(ENABLE_SSSE3)
  set(SSE_INSTRUCTIONS " -mssse3")
  add_definitions(-DGE_ENABLE_SSSE3)
  message("SSSE3 optimizations enabled")
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y -Wall -msse -msse2 -msse3 ${SSE_INSTRUCTIONS} -Wno-misleading-indentation")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -Wno-misleading-indentation")
endif()

include_directories("${CMAKE_SOURCE_DIR}")
include_directories("${CMAKE_SOURCE_DIR}/ext/glfw/include")
include_directories("${CMAKE_SOURCE_DIR}/ext/lz4")
include_directories("${CMAKE_SOURCE_DIR}/ext/zlib")
include_directories("${CMAKE_SOURCE_DIR}/ext/zlib/build")
include_directories("${CMAKE_SOURCE_DIR}/ext/libjpeg-turbo")
include_directories("${CMAKE_SOURCE_DIR}/ext/libjpeg-turbo/build")
include_directories("${CMAKE_SOURCE_DIR}/ext/libpng")
include_directories("${CMAKE_SOURCE_DIR}/ext/libpng/build")
link_directories("${CMAKE_SOURCE_DIR}/ext/glfw/build/src")
link_directories("${CMAKE_SOURCE_DIR}/ext/glfw/build_vs/src/Release")
link_directories("${CMAKE_SOURCE_DIR}/ext/zlib/build")
link_directories("${CMAKE_SOURCE_DIR}/ext/zlib/build_vs/Release")
link_directories("${CMAKE_SOURCE_DIR}/ext/libjpeg-turbo/build")
link_directories("${CMAKE_SOURCE_DIR}/ext/libjpeg-turbo/build/libs")
link_directories("${CMAKE_SOURCE_DIR}/ext/libjpeg-turbo/build_vs/Release")
link_directories("${CMAKE_SOURCE_DIR}/ext/libpng/build")
link_directories("${CMAKE_SOURCE_DIR}/ext/libpng/build_vs/Release")

add_subdirectory(base)
add_subdirectory(system)
#add_subdirectory(engine)
add_subdirectory(tests)
