add_library(system
  framework.cpp
  hotkey.cpp)

target_link_libraries(system base)
target_link_libraries(system glfw3)
if (${MINGW})
  target_link_libraries(system opengl32)
else ()
  if (MSVC)
    target_link_libraries(system opengl32)
  else()
    target_link_libraries(system GL X11 Xxf86vm pthread Xrandr Xcursor Xi dl Xinerama)
  endif()
endif()

install(TARGETS system DESTINATION lib)

install(FILES
  framework.hpp
  hotkey.hpp
  includes.hpp
  keycodes.hpp
  system.hpp
  DESTINATION include/system)
