if (NOT ${HALCHECK_GTEST})
  return()
endif()

if (NOT TARGET gtest)
  CPMAddPackage("gh:google/googletest@1.14.0")
endif()

if (NOT TARGET rang)
  # rang: cross-platform colored printing
  # * For nice GTest output
  CPMAddPackage("gh:agauniyal/rang@3.2")
endif()

add_library(${PROJECT_NAME}-gtest gtest.cpp)
target_compile_options(${PROJECT_NAME}-gtest PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4>
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic -Wdocumentation>
  $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>)
target_link_libraries(${PROJECT_NAME}-gtest PUBLIC ${PROJECT_NAME} gtest PRIVATE rang)
set_target_properties(${PROJECT_NAME}-gtest PROPERTIES EXPORT_COMPILE_COMMANDS ${HALCHECK_DEVELOPMENT})

if (NOT ${HALCHECK_DEVELOPMENT})
  return()
endif()

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp)

add_executable(${PROJECT_NAME}-gtest-test ${SOURCES})
target_compile_options(${PROJECT_NAME}-gtest-test PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4>
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic -Wdocumentation>
  $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>)
target_link_libraries(${PROJECT_NAME}-gtest-test ${PROJECT_NAME} ${PROJECT_NAME}-gtest)
set_target_properties(${PROJECT_NAME}-gtest-test PROPERTIES EXPORT_COMPILE_COMMANDS ON)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME}-gtest-test)

if (CLANG_FORMAT_EXE)
  add_custom_target(format-gtest
    ALL
    COMMAND ${CLANG_FORMAT_EXE} -n ${SOURCES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()

if (CLANG_TIDY_EXE)
  add_custom_target(tidy-gtest
    COMMAND ${CLANG_TIDY_EXE} -p ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json ${SOURCES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()

install(TARGETS ${PROJECT_NAME}-gtest)
