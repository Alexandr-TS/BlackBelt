#include "optional.h"

#include "test_runner.h"

using namespace std;

class C {
public:
    inline static int created = 0;
    inline static int assigned = 0;
    inline static int deleted = 0;
    static void Reset() {
        created = assigned = deleted = 0;
    }

    C() {
        ++created;
    }
    C(const C& other) {
        ++created;
    }
    C& operator=(const C& other) {
        ++assigned;
        return *this;
    }
    ~C() {
        ++deleted;
    }
};

void TestInit() {
    {
        C::Reset();
        C c;
        Optional<C> o(c);
        ASSERT(C::created == 2 && C::assigned == 0 && C::deleted == 0);
    }
    ASSERT(C::deleted == 2);
};


void TestAssign() {
    Optional<C> o1, o2;

    { // Assign a Value to empty
        C::Reset();
        C c;
        o1 = c;
        ASSERT(C::created == 2 && C::assigned == 0 && C::deleted == 0);
    }
    { // Assign a non-empty to empty
        C::Reset();
        o2 = o1;
        ASSERT(C::created == 1 && C::assigned == 0 && C::deleted == 0);
    }
    { // Assign non-empty to non-empty
        C::Reset();
        o2 = o1;
        ASSERT(C::created == 0 && C::assigned == 1 && C::deleted == 0);
    }
}

void TestReset() {
    C::Reset();
    Optional<C> o = C();
    o.Reset();
    ASSERT(C::created == 2 && C::assigned == 0 && C::deleted == 2);
}

void TestHasValue() {
    Optional<int> o;
    ASSERT(!o.HasValue());

    o = 42;
    ASSERT(o.HasValue());

    o.Reset();
    ASSERT(!o.HasValue());
}

int plusOne(const Optional<int>& x) {
    return *x + 1;
}

int plusOne2(const Optional<pair<int, int>>& x) {
    return x->second + 1;
}

void TestStars() {
    Optional<int> o = int(12);
    int x = *o;
    int y = plusOne(x);
    ASSERT(x == 12);
    ASSERT(y == 13);

    Optional<pair<int, int>> o2 = pair<int, int>(1, 2);
    ASSERT(o2->first == 1);
    int y2 = plusOne2(o2);
    ASSERT(y2 == 3);

    Optional<int> o3;
    o = o3;
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestInit);
    RUN_TEST(tr, TestAssign);
    RUN_TEST(tr, TestReset);
    RUN_TEST(tr, TestHasValue);
    RUN_TEST(tr, TestStars);
    return 0;
}
