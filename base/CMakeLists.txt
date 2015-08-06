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
  ../ext/lz4/lz4.c)

#target_link_libraries(base zlibstatic)

install(TARGETS base DESTINATION granite/bin)

install(FILES
  base.h
  common.h
  fs.h
  glisp.h
  glisp.inc.h
  gstdlib.h
  hwinfo.h
  image.h
  includes.h
  log.h
  math.h
  math.inc.h
  random.h
  random.inc.h
  scheduler.h
  sigslot.h
  stream.h
  stream.inc.h
  string.h
  string.inc.h
  timer.h
  tokenizer.h
  DESTINATION granite/include/base)