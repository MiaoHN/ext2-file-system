cmake_minimum_required(VERSION 3.10)

project(file_system_shell)

include_directories(${CMAKE_SOURCE_DIR}/include/fs)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

file(GLOB SRC
  "src/*.c"
  "src/fs/*.c"
)

add_executable(${PROJECT_NAME} ${SRC})