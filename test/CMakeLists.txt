include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)

set(TEST_SOURCES test_udiff.c)

add_executable(test_udiff ${TEST_SOURCES})
target_link_libraries(test_udiff udiff check)