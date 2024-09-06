file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS *.cpp)
add_library(${PROJECT_NAME}-clang ${SOURCES})
add_library(${PROJECT_NAME}::clang ALIAS ${PROJECT_NAME}-clang)
target_link_libraries(${PROJECT_NAME}-clang PUBLIC ${PROJECT_NAME}::test)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  find_library(CLANG_FUZZER_NO_MAIN
    NAMES clang_rt.fuzzer_no_main
          clang_rt.fuzzer_no_main-x86_64
    PATH_SUFFIXES clang/18/lib/linux)
  if("${CLANG_FUZZER_NO_MAIN}" STREQUAL "CLANG_FUZZER_NO_MAIN-NOTFOUND")
    unset(CLANG_FUZZER_NO_MAIN CACHE)
    set(CLANG_FUZZER_NO_MAIN clang_rt.fuzzer_no_main)
  endif()
  target_compile_options(${PROJECT_NAME}-clang PUBLIC -fsanitize=fuzzer-no-link)
  target_link_libraries(${PROJECT_NAME}-clang PUBLIC ${CLANG_FUZZER_NO_MAIN})
endif()

clang_format(clang "${SOURCES}")
