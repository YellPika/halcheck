cmake_minimum_required(VERSION 3.14...3.22)
project(halcheck VERSION 1.0 LANGUAGES CXX)

#[[ Configuration ]]

include(cmake/CPM.cmake)
include(cmake/target-link-system-libraries.cmake)
include(CMakeDependentOption)

if(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
  set(HALCHECK_DEVELOPMENT ON  CACHE BOOL "enable development mode")
else()
  set(HALCHECK_DEVELOPMENT OFF CACHE BOOL "enable development mode")
endif()

set(HALCHECK_SANITIZE  OFF CACHE STRING "enable sanitizers")
set(HALCHECK_COVERAGE  OFF CACHE BOOL   "enable code coverage")
set(HALCHECK_TYCHE     ON  CACHE BOOL   "enable tyche support")
set(HALCHECK_LIBFUZZER ON  CACHE BOOL   "enable libfuzzer support")

cmake_dependent_option(HALCHECK_GTEST  "enable gtest support"       ON "NOT HALCHECK_DEVELOPMENT" ON)
cmake_dependent_option(HALCHECK_GLOG   "enable glog support"        ON "NOT HALCHECK_DEVELOPMENT" ON)
cmake_dependent_option(HALCHECK_FORMAT "enable clang-format checks" ON "HALCHECK_DEVELOPMENT"     OFF)

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

if(HALCHECK_FORMAT)
  find_program(CLANG_FORMAT_EXE NAMES clang-format)
  add_custom_target(format ALL)
endif()

function(clang_format NAME SOURCES)
  if(NOT HALCHECK_FORMAT OR NOT CLANG_FORMAT_EXE OR NOT SOURCES)
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
add_subdirectory(src/gen)
add_subdirectory(src/test)

if(HALCHECK_GLOG)
  add_subdirectory(src/glog)
endif()

if(HALCHECK_GTEST)
  add_subdirectory(src/gtest)
endif()

if(HALCHECK_TYCHE)
  add_subdirectory(src/tyche)
endif()

if(HALCHECK_LIBFUZZER)
  add_subdirectory(src/clang)
endif()

#[[ Tests ]]

if(HALCHECK_DEVELOPMENT)
  include(CTest)
  if(BUILD_TESTING)
    add_subdirectory(test)
  endif()
endif()

#[[ Documentation ]]

if(HALCHECK_DEVELOPMENT)
  find_package(Doxygen)
  if(DOXYGEN_FOUND)
    set(DOXYGEN_TAGFILES "${CMAKE_CURRENT_SOURCE_DIR}/doc/cppreference-doxygen-web.tag.xml=http://en.cppreference.com/w/")
    set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

    set(DOXYGEN_EXPAND_ONLY_PREDEF YES)
    set(DOXYGEN_EXPAND_AS_DEFINED HALCHECK_REQUIRE HALCHECK_REQUIRE_)
    set(DOXYGEN_MACRO_EXPANSION YES)
    set(DOXYGEN_PREDEFINED HALCHECK_DOXYGEN)
    set(DOXYGEN_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/include)

    set(DOXYGEN_BUILTIN_STL_SUPPORT YES)
    set(DOXYGEN_HIDE_UNDOC_CLASSES YES)
    set(DOXYGEN_HIDE_UNDOC_MEMBERS NO)
    set(DOXYGEN_EXTRACT_STATIC YES)
    set(DOXYGEN_EXCLUDE_SYMBOLS "detail::*" "*::detail::*")

    set(DOXYGEN_USE_MATHJAX YES)
    set(DOXYGEN_SHOW_NAMESPACES NO)
    set(DOXYGEN_SHOW_FILES NO)
    set(DOXYGEN_SEPARATE_MEMBER_PAGES YES)
    set(DOXYGEN_INLINE_INHERITED_MEMB YES)

    set(DOXYGEN_WARN_IF_UNDOCUMENTED NO)

    doxygen_add_docs(doxygen
      ${CMAKE_CURRENT_SOURCE_DIR}/README.md
      ${CMAKE_CURRENT_SOURCE_DIR}/doc/intro.md
      ${CMAKE_CURRENT_SOURCE_DIR}/include)

    add_custom_target(
      upload-doxygen
      COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/doc/upload ${CMAKE_CURRENT_BINARY_DIR}
      DEPENDS doxygen)
  endif()
endif()
