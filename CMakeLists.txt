cmake_minimum_required(VERSION 3.14...3.22)
project(halcheck VERSION 1.0 LANGUAGES CXX)

#[[ Configuration ]]

include(cmake/CPM.cmake)
include(CMakeDependentOption)

if(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
  set(HALCHECK_DEVELOPMENT ON  CACHE BOOL "enable development mode")
else()
  set(HALCHECK_DEVELOPMENT OFF CACHE BOOL "enable development mode")
endif()

set(HALCHECK_SANITIZE OFF CACHE STRING "enable sanitizers")
set(HALCHECK_COVERAGE OFF CACHE BOOL   "enable code coverage")
set(HALCHECK_TYCHE    ON  CACHE BOOL   "enable tyche support")

cmake_dependent_option(HALCHECK_GTEST     "enable gtest support"      ON "HALCHECK_DEVELOPMENT" ON)
cmake_dependent_option(HALCHECK_GLOG      "enable glog support"       ON "HALCHECK_DEVELOPMENT" ON)
cmake_dependent_option(HALCHECK_LIBFUZZER "enable libfuzzer support"  ON "HALCHECK_DEVELOPMENT" ON)

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
    include(FetchContent)
    FetchContent_Declare(
        doxygen-awesome-css
        URL https://github.com/jothepro/doxygen-awesome-css/archive/refs/heads/main.zip
    )
    FetchContent_MakeAvailable(doxygen-awesome-css)
    FetchContent_GetProperties(doxygen-awesome-css SOURCE_DIR AWESOME_CSS_DIR)

    set(DOXYGEN_TAGFILES "${CMAKE_CURRENT_SOURCE_DIR}/doc/cppreference-doxygen-web.tag.xml=http://en.cppreference.com/w/")

    set(DOXYGEN_EXPAND_ONLY_PREDEF YES)
    set(DOXYGEN_EXPAND_AS_DEFINED HALCHECK_REQUIRE HALCHECK_REQUIRE_)
    set(DOXYGEN_MACRO_EXPANSION YES)
    set(DOXYGEN_PREDEFINED HALCHECK_DOXYGEN)
    set(DOXYGEN_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/include)

    set(DOXYGEN_BUILTIN_STL_SUPPORT YES)
    set(DOXYGEN_HIDE_UNDOC_CLASSES YES)
    set(DOXYGEN_HIDE_UNDOC_MEMBERS NO)
    # set(DOXYGEN_HIDE_FRIEND_COMPOUNDS YES)
    set(DOXYGEN_EXTRACT_STATIC YES)
    set(DOXYGEN_EXCLUDE_SYMBOLS "detail::*" "*::detail::*")

    set(DOXYGEN_USE_MATHJAX NO)
    set(DOXYGEN_SHOW_NAMESPACES YES)
    set(DOXYGEN_SEPARATE_MEMBER_PAGES YES)
    # set(DOXYGEN_HTML_EXTRA_STYLESHEET ${AWESOME_CSS_DIR}/doxygen-awesome.css)
    # set(DOXYGEN_GENERATE_TREEVIEW YES)
    # set(DOXYGEN_DISABLE_INDEX NO)
    # set(DOXYGEN_FULL_SIDEBAR NO)
    # set(DOXYGEN_COLORSTYLE LIGHT)

    # set(DOXYGEN_CLANG_ASSISTED_PARSING YES)
    # set(DOXYGEN_CLANG_DATABASE_PATH ${CMAKE_CURRENT_BINARY_DIR})

    doxygen_add_docs(doxygen ${CMAKE_CURRENT_SOURCE_DIR}/include)
  endif()
endif()
