CPMAddPackage("gh:gulrak/filesystem@1.5.14")
CPMAddPackage(
  NAME json
  VERSION 3.11.3
  GITHUB_REPOSITORY nlohmann/json
  OPTIONS "JSON_SystemInclude ON"
          "JSON_MultipleHeaders OFF"
          "JSON_ImplicitConversions OFF"
  EXCLUDE_FROM_ALL
  SYSTEM)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS *.cpp)
add_library(${PROJECT_NAME}-test ${SOURCES})
add_library(${PROJECT_NAME}::test ALIAS ${PROJECT_NAME}-test)
target_link_libraries(${PROJECT_NAME}-test
  PUBLIC ${PROJECT_NAME}::gen
  PRIVATE ghc_filesystem nlohmann_json::nlohmann_json)
target_link_libraries(${PROJECT_NAME} INTERFACE ${PROJECT_NAME}::test)
clang_format(test "${SOURCES}")
