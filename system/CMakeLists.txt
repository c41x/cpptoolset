add_library(system
  framework.cpp)

target_link_libraries(system base)
target_link_libraries(system glfw3)
if(${MINGW})
  target_link_libraries(system opengl32)
else()
  target_link_libraries(system GL X11 Xxf86vm pthread Xrandr Xcursor Xi)
endif()

install(TARGETS system DESTINATION bin)

install(FILES
  system.h
  includes.h
  framework.h
  DESTINATION include_system)