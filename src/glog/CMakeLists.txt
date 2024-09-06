CPMAddPackage(
  NAME glog
  VERSION 0.6.0
  GITHUB_REPOSITORY google/glog
  OPTIONS "WITH_GFLAGS OFF"
          "WITH_GTEST OFF"
          "BUILD_EXAMPLES OFF"
          "BUILD_TESTING OFF"
  EXCLUDE_FROM_ALL
  SYSTEM)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS *.cpp)
add_library(${PROJECT_NAME}-glog ${SOURCES})
add_library(${PROJECT_NAME}::glog ALIAS ${PROJECT_NAME}-glog)
target_link_libraries(${PROJECT_NAME}-glog PUBLIC ${PROJECT_NAME}::gen glog)
clang_format(glog "${SOURCES}")
