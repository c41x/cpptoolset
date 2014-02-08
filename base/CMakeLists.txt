add_library(base
  log.cpp
  math.cpp
  string.cpp
  timer.cpp
  tokenizer.cpp)

install(TARGETS base DESTINATION bin)

install(FILES
  base.h
  common.h
  includes.h
  log.h
  math.h
  sigslot.h
  string.h
  timer.h
  tokenizer.h
  DESTINATION include)