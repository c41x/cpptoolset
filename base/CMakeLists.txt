add_library(base
  log.cpp
  math.cpp
  string.cpp
  timer.cpp
  tokenizer.cpp
  glisp.cpp
  random.cpp
  hwinfo.cpp
  ../ext/lz4/lz4.c)

install(TARGETS base DESTINATION bin)

install(FILES
  base.h
  common.h
  includes.h
  log.h
  math.h
  math.inc.h
  sigslot.h
  string.h
  string.inc.h
  timer.h
  tokenizer.h
  gstdlib.h
  random.h
  hwinfo.h
  stream.h
  stream.inc.h
  ../ext/lz4/lz4.h
  DESTINATION include_base)