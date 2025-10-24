cmake_minimum_required(VERSION 4.1.1)

set(PROJECT_NAME jsonator)
set(TARGET_NAME jsonator)
set(JSONATOR_LIB jsonator_lib)

project(${PROJECT_NAME} VERSION 1.0.0 LANGUAGES C)

add_library(${JSONATOR_LIB} STATIC src/jsonator.c)

target_include_directories(${JSONATOR_LIB} PUBLIC ${CMAKE_SOURCE_DIR}/src)

add_executable(${TARGET_NAME} src/main.c)

target_link_libraries(${TARGET_NAME} PRIVATE ${JSONATOR_LIB})


