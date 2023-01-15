# Dependencies
set(RAYLIB_VERSION 4.2.0)
find_package(raylib ${RAYLIB_VERSION} QUIET) # QUIET or REQUIRED
if (NOT raylib_FOUND) # If there's none, fetch and build raylib
  FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
  )
  FetchContent_GetProperties(raylib)
  if (NOT raylib_POPULATED) # Have we downloaded raylib yet?
    set(FETCHCONTENT_QUIET NO)
    FetchContent_Populate(raylib)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
    add_subdirectory(${raylib_SOURCE_DIR} ${raylib_BINARY_DIR})
  endif()
endif()


#
set(SOURCE_FILES 
${SOURCE_DIR}/termija.cpp                 
${SOURCE_DIR}/pane.cpp              
${SOURCE_DIR}/drawing.cpp                  
${SOURCE_DIR}/rope.cpp
${SOURCE_DIR}/config.cpp
${SOURCE_DIR}/widgets/text.cpp
${SOURCE_DIR}/widgets/textbox.cpp)
set(SOURCE_FILES ${SOURCE_FILES} PARENT_SCOPE)

set(HEADER_FILES
${SOURCE_DIR}/termija.h
${SOURCE_DIR}/rope.h)
set(HEADER_FILES ${HEADER_FILES} PARENT_SCOPE)

add_library(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)
target_link_libraries(${PROJECT_NAME} PRIVATE plog)
set(raylib_VERBOSE 1)
