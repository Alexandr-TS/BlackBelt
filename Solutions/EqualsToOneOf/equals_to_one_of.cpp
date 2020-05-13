//#include "test_runner.h"

//#include <iostream>

// EqualsToOneOf(x, "apple", "orange") означает (x == "apple" || x == "orange")

template <typename T>
bool EqualsToOneOf(const T& x) {
    return false;
}

template <typename T, typename U, typename ... TArgs>
auto EqualsToOneOf(const T& x, const U& y, const TArgs& ... args) {
    return (x == y) || EqualsToOneOf(x, args...);
}

/*
void Test() {
    int xx = 10;
    ASSERT(EqualsToOneOf(xx, 1, 2, 3, 10));
    ASSERT(!EqualsToOneOf(xx, 1, 2, 3, 4, 5));

    auto x = "pear";
    ASSERT(EqualsToOneOf(x, "pear"));
    ASSERT(!EqualsToOneOf(x, "apple"));
    ASSERT(EqualsToOneOf(x, "apple", "pear"));
    ASSERT(!EqualsToOneOf(x, "apple", "banana"));
    ASSERT(EqualsToOneOf(x, "apple", "banana", "pear"));
    ASSERT(!EqualsToOneOf(x, "apple", "banana", "peach"));
    ASSERT(EqualsToOneOf(x, "apple", "banana", "pear", "orange"));
    ASSERT(!EqualsToOneOf(x, "apple", "banana", "peach", "orange"));
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, Test);
    return 0;
}
*/
