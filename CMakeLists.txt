cmake_minimum_required(VERSION 3.0.0)
project(The_PNG_Project VERSION 0.1.0)

include(CTest)
enable_testing()


file (GLOB_RECURSE Project_SOURCES CONFIGURE_DEPENDS "./*.cc")
#file (GLOB_RECURSE Project_HEADERS CONFIGURE_DEPENDS "./*.h")
#include_directories("include")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include") 
add_definitions(-DCMAKE_EXPORT_COMPILE_COMMANDS=ON)

add_executable(The_PNG_Project  ${Project_SOURCES})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
