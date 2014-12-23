set(SOURCE_FILES udiff.c)
add_library(udiff STATIC ${SOURCE_FILES})

install (TARGETS udiff DESTINATION lib)
install (FILES udiff.h DESTINATION include)