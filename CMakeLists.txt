cmake_minimum_required(VERSION 3.11) # FetchContent is available in 3.11+
project(termija)

include(FetchContent)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out)
set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)

FetchContent_Declare(
  plog
  GIT_REPOSITORY https://github.com/SergiusTheBest/plog
  GIT_TAG        1.1.8
)
FetchContent_MakeAvailable(plog)

add_subdirectory(${SOURCE_DIR})

#if (${CMAKE_BUILD_TYPE} EQUAL "DEBUG")
  add_subdirectory(tests)
#endif()

