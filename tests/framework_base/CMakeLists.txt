add_executable(framework_base WIN32 main.cpp)
target_link_libraries(framework_base system)
if (MSVC)
  # this flag allows to use main entry function instead WinMain.
  # with add_executable WIN32 flag this allows us to disable console window
  set_target_properties(framework_base PROPERTIES LINK_FLAGS "/ENTRY:mainCRTStartup")
endif ()
