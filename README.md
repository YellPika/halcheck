# A Property-Based Testing Library for C/C++

`halcheck` is a [Property-Based Testing](https://en.wikipedia.org/wiki/Software_testing#Property_testing) library for C/C++.

## Disclaimer

This library is in early stages of development.

## Installation

`halcheck` uses [CMake](https://cmake.org/) and can be integrated using standard CMake methods (e.g. via [`add_subdirectory`](https://cmake.org/cmake/help/latest/command/add_subdirectory.html).)

```cmake
add_subdirectory(path/to/halcheck/repository)
target_link_libraries(my-project
  halcheck         # Main library
  halcheck::gtest) # For Google Test integration
```

Additionally, `halcheck` can be integrated using [CPM.cmake](https://github.com/TheLartians/CPM.cmake).

```cmake
CPMAddPackage("gh:YellPika/halcheck@version")
target_link_libraries(my-project
  halcheck         # Main library
  halcheck::gtest) # For Google Test integration
```

## Documentation

See the [Introduction](@ref intro) and [Reference](@ref ref).
