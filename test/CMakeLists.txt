CPMAddPackage("gh:google/googletest#v1.17.0")

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS *.cpp)
add_executable(main ${SOURCES})
target_link_libraries(main ${PROJECT_NAME} ${PROJECT_NAME}::gtest ${PROJECT_NAME}::glog)
if(HALCHECK_LIBFUZZER)
  target_link_libraries(main ${PROJECT_NAME}::clang)
endif()

include(GoogleTest)
gtest_discover_tests(main)
