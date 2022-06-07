cmake_minimum_required(VERSION 3.23)
project(aim_trainer C)

set(CMAKE_C_STANDARD 23)

find_package(OpenGL REQUIRED COMPONENTS OpenGL)

if(CMAKE_SYSTEM MATCHES Windows)
    set(CMAKE_PREFIX_PATH "C:/freeglut")
endif()

find_package(GLUT REQUIRED)

include_directories("${CMAKE_CURRENT_LIST_DIR}/include")
include_directories("${CMAKE_CURRENT_LIST_DIR}/src")
include_directories("${CMAKE_CURRENT_LIST_DIR}/lib")

FILE(GLOB_RECURSE SRC_LIST ${CMAKE_CURRENT_LIST_DIR}/src/*.*)
FILE(GLOB_RECURSE LIB_LIST ${CMAKE_CURRENT_LIST_DIR}/lib/*.*)

add_executable(aim_trainer ${SRC_LIST})
target_link_libraries(aim_trainer PRIVATE OpenGL::GL GLUT::GLUT ${LIB_LIST})
