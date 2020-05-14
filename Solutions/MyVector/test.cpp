#include "vector.h"

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
        Vector<C> v(3);
        ASSERT(C::created == 3 && C::assigned == 0 && C::deleted == 0);
    }
    ASSERT(C::deleted == 3);
};


void TestAssign() {
    {
        C::Reset();
        Vector<C> v1(2), v2(3);
        ASSERT(C::created == 5 && C::assigned == 0 && C::deleted == 0);
        v1 = v2;
        ASSERT(C::created == 8 && C::assigned == 0 && C::deleted == 2);
        ASSERT(v1.Size() == 3 && v2.Size() == 3);
    }
    ASSERT(C::deleted == 8);

    {
        C::Reset();
        Vector<C> v1(3), v2(2);
        ASSERT(C::created == 5 && C::assigned == 0 && C::deleted == 0);
        v1 = v2;
        ASSERT(C::created == 5 && C::assigned == 2 && C::deleted == 1);
        ASSERT(v1.Size() == 2 && v2.Size() == 2);
    }
    ASSERT(C::deleted == 5);
}

void TestPushBack() {
    {
        C::Reset();
        Vector<C> v;
        C c;
        v.PushBack(c);
        ASSERT(C::created == 2 && C::assigned == 0 && C::deleted == 0);

        v.PushBack(c);  // reallocation
        ASSERT(C::created == 4 && C::assigned == 0 && C::deleted == 1);
    }
    ASSERT(C::deleted == 4);
}

void TestCtors() {
    auto v = Vector<C>();
    auto v2 = Vector<C>(3);
    auto v3(v2);
    auto v4 = std::move(v3);
}

void TestAssignments() {
    auto v = Vector<C>();
    auto v2 = Vector<C>(3);
    v2 = v;
    auto v3 = v2;
    v3 = move(v);
}

void TestManual() {
    auto v = Vector<pair<int, int>>(3);
    v.Reserve(4);
    v.PushBack({ 1, 2 });
    pair<int, int> p{ 3, 4 };
    v.PushBack(p);
    v.EmplaceBack(5, 6);
    v.PopBack();
    ASSERT_EQUAL(v.Size(), 5);
    ASSERT_EQUAL(v.Capacity(), 8);
    ASSERT_EQUAL(v[4].first, p.first);
    v[4] = make_pair(5, 7);
}

void TestMoves() {
    {
        auto v1 = Vector<int>(3);
        auto v2 = Vector<int>(4);
        ASSERT_EQUAL(v1.Size(), 3);
        ASSERT_EQUAL(v2.Size(), 4);
        v2 = std::move(v1);
        ASSERT_EQUAL(v2.Size(), 3);
        auto v3(move(v2));
        ASSERT_EQUAL(v3.Size(), 3);
    }
    {
        auto v1 = Vector<int>(4);
        auto v2 = Vector<int>(3);
        ASSERT_EQUAL(v1.Size(), 4);
        ASSERT_EQUAL(v2.Size(), 3);
        v2 = std::move(v1);
        ASSERT_EQUAL(v2.Size(), 4);
        auto v3(move(v2));
        ASSERT_EQUAL(v3.Size(), 4);
    }
}


class ClassWithStrangeConstructor {
public:
    int x, y;

    ClassWithStrangeConstructor(int& r, const int& cr) : x(r), y(cr) {
    }
};

void TestInsert() {
    Vector<int> v;
    v.PushBack(1);
    v.PushBack(2);
    auto it = v.Insert(v.cbegin(), 0);
    ASSERT(v.Size() == 3 && v[0] == 0 && v[1] == 1 && v[2] == 2 && it == v.begin());

    it = v.Insert(v.cend(), 3);
    ASSERT(v.Size() == 4 && v[0] == 0 && v[1] == 1 && v[2] == 2 && v[3] == 3 && it + 1 == v.end());
};

void TestEmplace() {
    Vector<ClassWithStrangeConstructor> v;
    int x = 1;
    const int y = 2;
    int z = 3;
    ClassWithStrangeConstructor c(z, z);
    v.PushBack(c);
    auto it = v.Emplace(v.cbegin(), x, y);
    ASSERT(v.Size() == 2 && v[0].x == x && v[0].y == y && v[1].x == z && v[1].y == z && it == v.begin());
};

void TestErase() {
    Vector<int> v;
    v.PushBack(1);
    v.PushBack(2);
    v.PushBack(3);
    auto it = v.Erase(v.cbegin() + 1);
    ASSERT(v.Size() == 2 && v[0] == 1 && v[1] == 3 && it == v.begin() + 1);
};

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestInit);
    RUN_TEST(tr, TestAssign);
    RUN_TEST(tr, TestPushBack);

    RUN_TEST(tr, TestCtors);
    RUN_TEST(tr, TestAssignments);
    RUN_TEST(tr, TestManual);
    RUN_TEST(tr, TestMoves);

    RUN_TEST(tr, TestInsert);
    RUN_TEST(tr, TestEmplace);
    RUN_TEST(tr, TestErase);

    return 0;
}
