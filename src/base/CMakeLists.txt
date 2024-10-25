file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/include/*.hpp)
add_library(${PROJECT_NAME}-base INTERFACE)
add_library(${PROJECT_NAME}::base ALIAS ${PROJECT_NAME}-base)
target_include_directories(${PROJECT_NAME}-base SYSTEM INTERFACE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include/${PROJECT_NAME}-${PROJECT_VERSION}>)
target_link_libraries(${PROJECT_NAME} INTERFACE ${PROJECT_NAME}::base)
clang_format(base "${HEADERS}")


if(HALCHECK_DEVELOPMENT)
  target_compile_options(${PROJECT_NAME}-base INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic -Wno-dangling-pointer>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic -Wdocumentation>)

  if(HALCHECK_COVERAGE)
    target_compile_options(${PROJECT_NAME}-base INTERFACE
      $<$<CXX_COMPILER_ID:GNU>:--coverage>
      $<$<CXX_COMPILER_ID:Clang>:--coverage>)
    target_link_options(${PROJECT_NAME}-base INTERFACE
      $<$<CXX_COMPILER_ID:GNU>:--coverage>
      $<$<CXX_COMPILER_ID:Clang>:--coverage>)
  endif()
endif()
