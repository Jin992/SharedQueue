cmake_minimum_required(VERSION 3.15)
project(SharedQueue)

set(CMAKE_CXX_STANDARD 14)
find_package(Boost COMPONENTS system filesystem unit_test_framework REQUIRED)

add_executable(${PROJECT_NAME} main.cpp SharedQueueTest.cpp)
target_link_libraries(${PROJECT_NAME}   -pthread
                                        ${Boost_FILESYSTEM_LIBRARY}
                                        ${Boost_SYSTEM_LIBRARY}
                                        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})