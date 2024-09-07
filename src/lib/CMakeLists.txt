CPMAddPackage("gh:mpark/variant@1.4.0")

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS *.cpp)
add_library(${PROJECT_NAME}-lib ${SOURCES})
add_library(${PROJECT_NAME}::lib ALIAS ${PROJECT_NAME}-lib)
target_link_libraries(${PROJECT_NAME}-lib ${PROJECT_NAME}::base mpark_variant)
target_link_libraries(${PROJECT_NAME} INTERFACE ${PROJECT_NAME}::lib)
clang_format(lib "${SOURCES}")
