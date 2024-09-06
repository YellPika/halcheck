CPMAddPackage("gh:google/googletest#release-1.12.1")

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS *.cpp)
add_executable(main ${SOURCES})
target_link_libraries(main ${PROJECT_NAME} ${PROJECT_NAME}::gtest ${PROJECT_NAME}::glog ${PROJECT_NAME}::clang)

include(GoogleTest)
gtest_discover_tests(main)
