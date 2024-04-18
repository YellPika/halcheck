if (NOT ${HALCHECK_DOCTEST})
  return()
endif()

if (NOT TARGET doctest::doctest)
  CPMAddPackage("gh:doctest/doctest@2.4.11")
endif()

add_library(${PROJECT_NAME}-doctest doctest.cpp)
target_compile_options(${PROJECT_NAME}-doctest PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4>
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic -Wdocumentation>
  $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>)
target_link_libraries(${PROJECT_NAME}-doctest ${PROJECT_NAME} doctest::doctest)
set_target_properties(${PROJECT_NAME}-doctest PROPERTIES EXPORT_COMPILE_COMMANDS ${HALCHECK_DEVELOPMENT})

if (NOT ${HALCHECK_DEVELOPMENT})
  return()
endif()

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp)

add_executable(${PROJECT_NAME}-doctest-test ${SOURCES})
target_compile_options(${PROJECT_NAME}-doctest-test PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4>
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic -Wdocumentation>
  $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>)
target_link_libraries(${PROJECT_NAME}-doctest-test ${PROJECT_NAME} ${PROJECT_NAME}-doctest)
set_target_properties(${PROJECT_NAME}-doctest-test PROPERTIES EXPORT_COMPILE_COMMANDS ON)

include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)
doctest_discover_tests(${PROJECT_NAME}-doctest-test)

if (CLANG_FORMAT_EXE)
  add_custom_target(format-doctest
    ALL
    COMMAND ${CLANG_FORMAT_EXE} -n ${SOURCES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()

if (CLANG_TIDY_EXE)
  add_custom_target(tidy-doctest
    COMMAND ${CLANG_TIDY_EXE} -p ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json ${SOURCES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()

install(TARGETS ${PROJECT_NAME}-doctest)
