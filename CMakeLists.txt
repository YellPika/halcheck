cmake_minimum_required(VERSION 3.22)

project(halcheck
  VERSION 1.0
  LANGUAGES CXX)

# We default to development mode if we're not included from another directory.
if(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
  set(HALCHECK_DEVELOPMENT ON CACHE BOOL "enable development mode")
else()
  set(HALCHECK_DEVELOPMENT OFF CACHE BOOL "enable development mode")
endif()
set(HALCHECK_DOCTEST OFF CACHE BOOL "enable doctest integration")
set(HALCHECK_GTEST   OFF CACHE BOOL "enable gtest integration")

if(${HALCHECK_DEVELOPMENT})
  set(HALCHECK_DOCTEST              ON CACHE BOOL "" FORCE)
  set(HALCHECK_GTEST                ON CACHE BOOL "" FORCE)

  include(CTest)

  # Sanitization options
  set(SANITIZE "" CACHE STRING "string to pass to -fsanitize")
  if(SANITIZE AND NOT CXX_COMPILER_ID MATCHES MSVC)
    add_compile_options(-fsanitize=${SANITIZE} -fno-omit-frame-pointer)
    add_link_options(-fsanitize=${SANITIZE} -fno-omit-frame-pointer)
  endif()
  if(SANITIZE AND CXX_COMPILER_ID MATCHES MSVC)
    add_compile_options(/fsanitize=${SANITIZE})
    add_link_options(/fsanitize=${SANITIZE})
  endif()

  find_program(CLANG_FORMAT_EXE NAMES clang-format)
  find_program(CLANG_TIDY_EXE NAMES clang-tidy)
endif()

### Dependencies ###

include(cmake/CPM.cmake)

# mpark::variant: std::variant implementation
# * For C++11/14 compatibility
CPMAddPackage("gh:mpark/variant@1.4.0")

# tl::optional: std::optional implementation
# * For C++11/14 compatibility
set(OPTIONAL_BUILD_PACKAGE OFF) # tl::optional does not need to be packaged
set(OPTIONAL_BUILD_TESTS OFF)   # and tested when used as a dependency.
CPMAddPackage("gh:TartanLlama/optional@1.1.0")

### Configuration ###

file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} mpark_variant tl::optional)
target_compile_options(${PROJECT_NAME} PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4>
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic -Wdocumentation>
  $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>)
target_include_directories(
  ${PROJECT_NAME} SYSTEM PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
                                $<INSTALL_INTERFACE:include/${PROJECT_NAME}-${PROJECT_VERSION}>)
set_target_properties(${PROJECT_NAME} PROPERTIES EXPORT_COMPILE_COMMANDS ${HALCHECK_DEVELOPMENT})

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test/doctest)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test/gtest)

### Development ###

if (NOT HALCHECK_DEVELOPMENT)
  return()
endif()

if (CLANG_FORMAT_EXE)
  add_custom_target(format
    ALL
    COMMAND ${CLANG_FORMAT_EXE} -n ${HEADERS} ${SOURCES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()

if (CLANG_TIDY_EXE)
  add_custom_target(tidy
    COMMAND ${CLANG_TIDY_EXE} -p ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json ${SOURCES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()

### Installation ###

install(TARGETS ${PROJECT_NAME})
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/ DESTINATION include)
