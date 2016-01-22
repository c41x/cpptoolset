set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/tests/rosemary/cmake/Modules/")
find_package(glbindingx REQUIRED)

add_executable(rosemary main.cpp) # WIN32

include_directories(${GLBINDING_INCLUDE_DIRS})

target_link_libraries(rosemary system ${GLBINDING_LIBRARIES})

#if (MSVC)
  # this flag allows to use main entry function instead WinMain.
  # with add_executable WIN32 flag this allows us to disable console window
  # set_target_properties(rosemary PROPERTIES LINK_FLAGS "/ENTRY:mainCRTStartup")
#endif ()
