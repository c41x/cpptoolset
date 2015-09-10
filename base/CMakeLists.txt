add_library(base
  log.cpp
  math.cpp
  string.cpp
  timer.cpp
  tokenizer.cpp
  glisp.cpp
  random.cpp
  hwinfo.cpp
  image.cpp
  ../ext/lz4/lz4.c
  fs.cpp
  profiler.cpp)

target_link_libraries(base jpeg)
if(NOT WIN32)
  target_link_libraries(base z)
  target_link_libraries(base png)
  target_link_libraries(base pthread)
else()
  target_link_libraries(base png16_static)
  target_link_libraries(base zlibstatic)
endif()

install(TARGETS base DESTINATION lib)

install(FILES
  base.hpp
  common.hpp
  fs.hpp
  glisp.hpp
  glisp.inc.hpp
  gstdlib.hpp
  hwinfo.hpp
  image.hpp
  image.inc.hpp
  includes.hpp
  log.hpp
  math.hpp
  math.inc.hpp
  math.string.hpp
  profiler.hpp
  random.hpp
  random.inc.hpp
  scheduler.hpp
  sigslot.hpp
  simd_vector.hpp
  stream.hpp
  stream.inc.hpp
  string.hpp
  string.inc.hpp
  timer.hpp
  tokenizer.hpp
  DESTINATION include/base)