
cmake_minimum_required(VERSION 3.14)
project(lvm VERSION 0.1.0)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/lib")

set(LVM_DEBUG ON CACHE BOOL "Enable debug functions.")

set(LVM_TASK_PRIORITY_MAX 8 CACHE STRING "Number of task priority levels.")
if(LVM_TASK_PRIORITY_MAX LESS 1)
    message(FATAL_ERROR "LVM_TASK_PRIORITY_MAX must be at least 1.")
endif()

set(LVM_TASK_NAME_MAX 32 CACHE STRING "Maximum task name buffer size.")
if(LVM_TASK_NAME_MAX LESS 16)
    message(FATAL_ERROR "LVM_TASK_NAME_MAX must be at least 16.")
endif()

include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_BINARY_DIR}/include"
)

add_definitions(
    -DLUA_USER_H=\"${CMAKE_CURRENT_SOURCE_DIR}/include/lvm/hooks.h\"
)

file(GLOB_RECURSE LVM_SOURCES LIST_DIRECTORIES false
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/*.h.in"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

set(LVM_SHARED ON CACHE BOOL "Build as shared library.")
if(LVM_SHARED)
    set(LVM_LIBTYPE SHARED)
else()
    set(LVM_LIBTYPE STATIC)
endif()

add_library(lvm ${LVM_LIBTYPE} ${LVM_SOURCES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${LVM_SOURCES})

if(LVM_SHARED)
    target_compile_definitions(lvm PRIVATE
        LUA_BUILD_AS_DLL
        LVM_CORE
    )
endif()

configure_file(
    include/lvm/config.h.in
    include/lvm/config.h
)

find_package(libuv REQUIRED)
target_link_libraries(lvm uv)

add_subdirectory(test)
