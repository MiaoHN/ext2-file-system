cmake_minimum_required(VERSION 3.10)

project(TS)

file(GLOB SRC
  "src/*.h"
  "src/*.c"
)

add_executable(${PROJECT_NAME} ${SRC})