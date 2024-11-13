# Introduction {#intro}

_Property-Based Testing_ checks whether an executable specification --- called a _property_ --- holds over a wide range of inputs. Standard unit tests check inputs that we think of; property-based testing allows us to check inputs that we forgot to think about. For example, consider an implementation of binary search:

```c++
bool binary_search(const std::vector<int> &xs, int key) {
  std::size_t max = xs.size(), min = 0;
  while (min < max) {
    auto mid = lib::midpoint(min, max);
    if (key < xs[mid])
      max = mid - 1;
    else if (key > xs[mid])
      min = mid + 1;
    else
      return true;
  }
  return false;
}
```

Is `binary_search` implemented correctly? We can write a few simple unit tests:

```c++
TEST(BinarySearch, Examples) {
  EXPECT_FALSE(binary_search({}, 0));
  EXPECT_TRUE(binary_search({0}, 0));
  EXPECT_TRUE(binary_search({0, 1, 2}, 2));
  EXPECT_TRUE(binary_search({0, 1, 2, 3, 4, 5}, 2));
}
```

Code coverage tools will confirm that the above executes every line of code in `binary_search`. Is this enough to ensure correctness? Also, is it scalable? In a large codebase we would have to write many tests to ensure complete code coverage.

With `halcheck`, we can avoid these problems by testing `binary_search` against a large number of randomly selected inputs. Indeed, the following test reveals that `binary_search({0, 1}, 0)` returns `false` instead of `true`:

```c++
HALCHECK_TEST(BinarySearch, Membership) {
  // Generate a random ("arbitrary") sorted vector.
  auto xs = gen::arbitrary<std::vector<int>>("xs"_s);
  std::sort(xs.begin(), xs.end());

  // Generate a random element of xs.
  auto x = gen::element_of("x"_s, xs);

  // halcheck does no logging by default.
  // It is good practice to log your inputs in case the test fails.
  LOG(INFO) << "xs: " << testing::PrintToString(xs);
  LOG(INFO) << "x: " << testing::PrintToString(x);

  // Since x is an element of xs,
  // binary_search(xs, x) should always return true.
  EXPECT_TRUE(binary_search(xs, x));
}
```

Fixing the bug in `binary_search` is left as an exercise to the reader.
