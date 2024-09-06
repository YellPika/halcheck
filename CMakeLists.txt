cmake_minimum_required(VERSION 3.14...3.22)
project(halcheck VERSION 1.0 LANGUAGES CXX)

#[[ Configuration ]]

include(cmake/CPM.cmake)

if(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
  set(HALCHECK_DEVELOPMENT ON  CACHE BOOL "enable development mode")
else()
  set(HALCHECK_DEVELOPMENT OFF CACHE BOOL "enable development mode")
endif()

set(HALCHECK_SANITIZE OFF CACHE STRING "enable sanitizers")
set(HALCHECK_COVERAGE OFF CACHE BOOL   "enable code coverage")
set(HALCHECK_GTEST    ON  CACHE BOOL   "enable gtest support")
set(HALCHECK_GLOG     ON  CACHE BOOL   "enable glog support")

if(HALCHECK_DEVELOPMENT)
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "" FORCE)

  if(USE_CCACHE)
    CPMAddPackage(
      NAME Ccache.cmake
      GITHUB_REPOSITORY TheLartians/Ccache.cmake
      VERSION 1.2.5)
  endif()

endif()

if(HALCHECK_SANITIZE)
  add_compile_options("-fsanitize=${HALCHECK_SANITIZE}")
  add_link_options("-fsanitize=${HALCHECK_SANITIZE}")
endif()

#[[ Formatting ]]

if(HALCHECK_DEVELOPMENT)
  find_program(CLANG_FORMAT_EXE NAMES clang-format)
  add_custom_target(format ALL)
endif()

function(clang_format NAME SOURCES)
  if(NOT HALCHECK_DEVELOPMENT OR NOT CLANG_FORMAT_EXE OR NOT SOURCES)
    return()
  endif()

  add_custom_target(format-${NAME}
    COMMAND ${CLANG_FORMAT_EXE} -n ${SOURCES}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

  add_dependencies(format format-${NAME})
endfunction()

#[[ Modules ]]

add_library(${PROJECT_NAME} INTERFACE)
add_subdirectory(src/base)
add_subdirectory(src/lib)
add_subdirectory(src/eff)
add_subdirectory(src/gen)
add_subdirectory(src/test)
add_subdirectory(src/glog)
add_subdirectory(src/gtest)
add_subdirectory(src/clang)

#[[ Tests ]]

if(HALCHECK_DEVELOPMENT)
  include(CTest)
  if(BUILD_TESTING)
    add_subdirectory(test)
  endif()
endif()
