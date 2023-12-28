cmake_minimum_required(VERSION 3.26)

project(halcheck
  VERSION 1.0
  LANGUAGES CXX)

include(cmake/CPM.cmake)

### Dependencies ###

# doctest: Testing Framework
# * For running tests.
# * For compiling the source since tests are written in the same file.
CPMAddPackage("gh:doctest/doctest@2.4.11")

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

add_library(${PROJECT_NAME} OBJECT ${SOURCES})
target_sources(${PROJECT_NAME} PUBLIC
  FILE_SET HEADERS
  BASE_DIRS include
  FILES ${HEADERS})
target_link_libraries(${PROJECT_NAME}
  PUBLIC mpark_variant tl::optional
  PRIVATE doctest::doctest)
target_compile_options(${PROJECT_NAME} PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4>
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic -Wdocumentation>
  $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>)
target_include_directories(
  ${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
                         $<INSTALL_INTERFACE:include/${PROJECT_NAME}-${PROJECT_VERSION}>)

if(NOT CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  target_compile_definitions(${PROJECT_NAME} PRIVATE -DDOCTEST_CONFIG_DISABLE)
  return()
endif()

### Development ###

# Sanitization options
set(SANITIZE "" CACHE STRING "string to pass to -fsanitize")
if(SANITIZE AND NOT CXX_COMPILER_ID MATCHES MSVC)
  add_compile_options(-fsanitize=${SANITIZE} -fno-omit-frame-pointer)
  add_link_options(-fsanitize=${SANITIZE} -fno-omit-frame-pointer)
endif()

### Testing ###

include(CTest)

file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp)

add_executable(${PROJECT_NAME}-test ${TEST_SOURCES})
target_compile_options(${PROJECT_NAME}-test PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4>
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic -Wdocumentation>
  $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>)
target_link_libraries(${PROJECT_NAME}-test ${PROJECT_NAME} mpark_variant tl::optional doctest::doctest)

include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)
doctest_discover_tests(${PROJECT_NAME}-test)

set_target_properties(${PROJECT_NAME} ${PROJECT_NAME}-test PROPERTIES EXPORT_COMPILE_COMMANDS ON)

### Installation ###

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}.pc.in
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc
  @ONLY)

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
  COMPATIBILITY AnyNewerVersion)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}-config.cmake
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME})

install(
  TARGETS ${PROJECT_NAME}
  EXPORT ${PROJECT_NAME}Targets
  FILE_SET HEADERS)

install(
  EXPORT ${PROJECT_NAME}Targets
  NAMESPACE ${PROJECT_NAME}::
  DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME})

### Maintenance ###

find_program(CLANG_FORMAT_EXE NAMES clang-format)
if (CLANG_FORMAT_EXE)
  add_custom_target(format
    ALL
    COMMAND ${CLANG_FORMAT_EXE} -n ${HEADERS} ${SOURCES} ${TEST_SOURCES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()

find_program(CLANG_TIDY_EXE NAMES clang-tidy)
if (CLANG_TIDY_EXE)
  add_custom_target(tidy
    COMMAND ${CLANG_TIDY_EXE} -p ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json ${SOURCES} ${TEST_SOURCES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()
