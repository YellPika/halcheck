CPMAddPackage("gh:google/googletest#release-1.12.1")

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS *.cpp)
add_library(${PROJECT_NAME}-gtest ${SOURCES})
add_library(${PROJECT_NAME}::gtest ALIAS ${PROJECT_NAME}-gtest)
target_link_libraries(${PROJECT_NAME}-gtest PUBLIC ${PROJECT_NAME}::test gtest)
clang_format(gtest "${SOURCES}")
