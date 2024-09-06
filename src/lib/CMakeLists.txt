CPMAddPackage("gh:mpark/variant@1.4.0")
CPMAddPackage(
  NAME optional
  VERSION 1.1.0
  GITHUB_REPOSITORY TartanLlama/optional
  OPTIONS "OPTIONAL_BUILD_PACKAGE OFF"
          "OPTIONAL_BUILD_TESTS OFF"
  EXCLUDE_FROM_ALL
  SYSTEM)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS *.cpp)
add_library(${PROJECT_NAME}-lib ${SOURCES})
add_library(${PROJECT_NAME}::lib ALIAS ${PROJECT_NAME}-lib)
target_link_libraries(${PROJECT_NAME}-lib ${PROJECT_NAME}::base tl::optional mpark_variant)
target_link_libraries(${PROJECT_NAME} INTERFACE ${PROJECT_NAME}::lib)
clang_format(lib "${SOURCES}")
