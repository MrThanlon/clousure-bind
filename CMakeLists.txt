cmake_minimum_required(VERSION 3.0)

project(clousure-bind)

add_library(${PROJECT_NAME} clousure_bind.c)
target_include_directories(${PROJECT_NAME} PUBLIC include)

add_executable(test-${PROJECT_NAME} test.c)
target_link_libraries(test-${PROJECT_NAME} ${PROJECT_NAME})
