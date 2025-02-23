include_directories(${CMAKE_SOURCE_DIR}/src/include)

add_executable(subject_test src/subject_test.cpp)

target_link_libraries(subject_test gtest gtest_main RxLite)

include(GoogleTest)
gtest_discover_tests(subject_test)