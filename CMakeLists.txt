cmake_minimum_required(VERSION 3.26)

project(halcheck
  VERSION 1.0
  LANGUAGES CXX)

include(cmake/CPM.cmake)
include(CTest)

set(OPTIONAL_BUILD_PACKAGE OFF)
set(OPTIONAL_BUILD_TESTS OFF)

CPMAddPackage("gh:doctest/doctest@2.4.11")
CPMAddPackage("gh:mpark/variant@1.4.0")
CPMAddPackage("gh:TartanLlama/optional@1.1.0")

set(HALCHECK_SANITIZE "" CACHE STRING "string to pass to -fsanitize")
if(BUILD_TESTING AND HALCHECK_SANITIZE AND NOT CXX_COMPILER_ID MATCHES MSVC)
  add_compile_options(-fsanitize=${HALCHECK_SANITIZE} -fno-omit-frame-pointer)
  add_link_options(-fsanitize=${HALCHECK_SANITIZE} -fno-omit-frame-pointer)
endif()

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
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>)
target_include_directories(
  ${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
                         $<INSTALL_INTERFACE:include/${PROJECT_NAME}-${PROJECT_VERSION}>)

if(BUILD_TESTING)
  add_executable(${PROJECT_NAME}-test test/main.cpp)
  target_compile_options(${PROJECT_NAME}-test PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>)
  target_link_libraries(${PROJECT_NAME}-test ${PROJECT_NAME} mpark_variant tl::optional doctest::doctest)

  include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)
  doctest_discover_tests(${PROJECT_NAME}-test)
else()
  target_compile_definitions(${PROJECT_NAME} PRIVATE -DDOCTEST_CONFIG_DISABLE)
endif()

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
