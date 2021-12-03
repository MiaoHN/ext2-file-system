cmake_minimum_required(VERSION 3.20)

project(Ext2FileSystem)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

file(GLOB SRCS 
  "src/*.c"
  "src/*.h"
)

include_directories(src)

add_executable(${PROJECT_NAME} ${SRCS})
