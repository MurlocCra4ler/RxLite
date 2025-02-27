include_directories(${CMAKE_SOURCE_DIR}/src/include)

add_executable(observable_test src/observable_test.cpp)
add_executable(operator_test src/operator_test.cpp)
add_executable(subject_test src/subject_test.cpp)

target_link_libraries(observable_test gtest gtest_main RxLite)
target_link_libraries(subject_test gtest gtest_main RxLite)
target_link_libraries(operator_test gtest gtest_main RxLite)

include(GoogleTest)
gtest_discover_tests(observable_test)
gtest_discover_tests(operator_test)
gtest_discover_tests(subject_test)
