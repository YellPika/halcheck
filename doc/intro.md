# Introduction {#intro}

_Property-Based Testing_ checks whether a specification holds over a wide range of inputs. Property-based testing provides the following benefits over standard testing techniques:

1. _Better coverage:_ standard example-based tests check scenarios that we think of; property-based testing allows us to check scenarios that we forgot to think about.

2. _Better scalability:_ a single property-based test can replace a large number of example-based tests.

For example, consider an implementation of binary search:

```cpp
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

Is `binary_search` implemented correctly? We can write a few simple example-based tests:

```cpp
TEST(BinarySearch, Examples) {
  EXPECT_FALSE(binary_search({}, 0));
  EXPECT_TRUE(binary_search({0}, 0));
  EXPECT_TRUE(binary_search({0, 1, 2}, 2));
  EXPECT_TRUE(binary_search({0, 1, 2, 3, 4, 5}, 2));
}
```

Code coverage tools will confirm that the above test executes every line of code in `binary_search`. Nonetheless, `binary_search` _does_ have a bug, which is revealed with a simple property-based test:

```cpp
HALCHECK_TEST(BinarySearch, Membership) {
  // Generate a random ("arbitrary") sorted vector.
  auto xs = gen::arbitrary<std::vector<int>>("xs"_s); // (a)
  std::sort(xs.begin(), xs.end());

  // Generate a random element of xs.
  auto x = gen::element_of("x"_s, xs); // (b)

  // halcheck does no logging by default.
  // It is good practice to log your inputs in case the test fails.
  LOG(INFO) << "xs: " << testing::PrintToString(xs);
  LOG(INFO) << "x: " << testing::PrintToString(x);

  // Since x is an element of xs,
  // binary_search(xs, x) should always return true.
  EXPECT_TRUE(binary_search(xs, x)); // (c)
}
```

The above test reveals that `binary_search({0, 1}, 0)` returns `false` instead of `true`. Fixing this bug is left as an exercise to the reader.

## Elements of a Property-Based Test

Property-based tests require three things:

1. a _system under test_ whose correctness we would like to test,
2. a set of _generators_ which produce inputs for the system under test, and
3. an _oracle_ which determines if the system under test behaves correctly.

In the above example, the system under test is the function `binary_search`. There are two generators, marked by `(a)` and `(b)`. Finally, the oracle is marked by `(c)`[note].

The system under test is usually given, so the main challenge of property-based testing lies in coming up with generators and oracles.

[note]: . "Oracles need not be explicit. Sometimes you just want to test that a piece of code does not crash."
