CPMAddPackage("gh:gulrak/filesystem@1.5.14")
CPMAddPackage(
  NAME json
  VERSION 3.11.3
  GITHUB_REPOSITORY nlohmann/json
  OPTIONS "JSON_SystemInclude ON"
  EXCLUDE_FROM_ALL
  SYSTEM)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS *.cpp)
add_library(${PROJECT_NAME}-tyche ${SOURCES})
add_library(${PROJECT_NAME}::tyche ALIAS ${PROJECT_NAME}-tyche)
target_link_libraries(${PROJECT_NAME}-tyche
  PUBLIC ${PROJECT_NAME}::test
  PRIVATE ghc_filesystem nlohmann_json::nlohmann_json)
target_link_libraries(${PROJECT_NAME} INTERFACE ${PROJECT_NAME}::tyche)
clang_format(tyche "${SOURCES}")
