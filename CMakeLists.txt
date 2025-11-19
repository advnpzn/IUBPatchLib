cmake_minimum_required(VERSION 3.20)
project(IUBPatchLib VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(IUBPATCH_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(IUBPATCH_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(IUBPATCH_VERSION_PATCH ${PROJECT_VERSION_PATCH})

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/include/iubpatch/version.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/include/iubpatch/version.h"
    @ONLY
)

option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(BUILD_STATIC_LIBS "Build static libraries" ON)
option(IUB_ENABLE_MMAP "Enable mmap support" ON)
option(BUILD_EXAMPLES "Build examples" OFF)
option(BUILD_TESTS "Build tests" ON)
option(BUILD_BENCHMARKS "Build benchmarks" OFF)
option(BUILD_TOOLS "Build command-line tools" ON)

add_subdirectory(src)

if(BUILD_TOOLS)
  add_subdirectory(tools)
endif()

if(BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

if(BUILD_BENCHMARKS)
  add_subdirectory(bench)
endif()

add_subdirectory(packaging)

include(ExportIUB)
