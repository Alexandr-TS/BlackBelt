#include <test_runner.h>

#include <cassert>
#include <cmath>
#include <stdexcept>
#include <sstream>
#include <stack>
#include <set>
#include <string>

using namespace std;

class Base {
public:
    Base(ostream& os, char close_symb = ' ')
        : os(os)
        , close_symb(close_symb)
    {}

protected:
    void Close() {
        if (!closed) {
            os << close_symb;
            closed = true;
        }
    }
    void Comma() {
        if (cnt_elements > 0) {
            os << ",";
        }
        cnt_elements++;
    }

public:
    void PrintString(string_view str) {
        os << "\"";
        for (auto ch : str) {
            if (set{ '\\', '\"' }.count(ch)) {
                os << '\\';
            }
            os << ch;
        }
        os << "\"";
    }

protected:
    ostream& os;

private:
    bool closed = false;
    int cnt_elements = 0;
    char close_symb;
};

class Empty : Base {
public:
    Empty(ostream& os)
        : Base(os)
    {}
};

template <typename Owner>
class InObjKeyOrEnd;

template <typename Owner>
class InObjValue;

template <typename Owner>
class InArray : public Base {
public:
    InArray(ostream& os, Owner& owner)
        : Base(os, ']')
        , owner(owner)
    {
        os << "[";
    }

    ~InArray() {
        Close();
    }

    InArray& Number(int64_t num) {
        Comma();
        os << num;
        return *this;
    }

    InArray& String(string_view str) {
        Comma();
        PrintString(str);
        return *this;
    }

    InArray& Boolean(bool value) {
        Comma();
        if (value) {
            os << "true";
        }
        else {
            os << "false";
        }
        return *this;
    }

    InArray& Null() {
        Comma();
        os << "null";
        return *this;
    }

    InArray<InArray<Owner>> BeginArray() {
        Comma();
        return InArray<InArray<Owner>>(os, *this);
    }

    Owner& EndArray() {
        Close();
        return owner;
    }

    InObjKeyOrEnd<InArray<Owner>> BeginObject() {
        Comma();
        return InObjKeyOrEnd<InArray<Owner>>(os, *this);
    }

private:
    Owner& owner;
};

template <typename Owner>
class InObjKeyOrEnd : public Base {
public:
    InObjKeyOrEnd(ostream& os, Owner& owner)
        : Base(os, '}')
        , owner(owner)
    {
        os << "{";
    }

    ~InObjKeyOrEnd() {
        Close();
    }

    InObjValue<InObjKeyOrEnd<Owner>> Key(string_view key) {
        Comma();
        PrintString(key);
        os << ":";
        return InObjValue<InObjKeyOrEnd<Owner>>(os, *this);
    }

    Owner& EndObject() {
        Close();
        return owner;
    }

private:
    Owner& owner;
};

template <typename Owner>
class InObjValue : public Base {
public:
    InObjValue(ostream& os, Owner& owner)
        : Base(os)
        , owner(owner)
    {}

    ~InObjValue() {
        if (!had_value) {
            os << "null";
        }
    }

    InArray<Owner> BeginArray() {
        had_value = true;
        return InArray<Owner>(os, owner);
    }

    InObjKeyOrEnd<Owner> BeginObject() {
        had_value = true;
        return InObjKeyOrEnd<Owner>(os, owner);
    }

    Owner& Number(int64_t num) {
        had_value = true;
        os << num;
        return owner;
    }

    Owner& String(string_view str) {
        had_value = true;
        PrintString(str);
        return owner;
    }

    Owner& Boolean(bool value) {
        had_value = true;
        if (value) {
            os << "true";
        }
        else {
            os << "false";
        }
        return owner;
    }

    Owner& Null() {
        os << "null";
        had_value = true;
        return owner;
    }

private:
    bool had_value = false;
    Owner& owner;
};

void PrintJsonString(std::ostream& out, std::string_view str) {
    Base tmp(out);
    tmp.PrintString(str);
}

using ArrayContext = InArray<Empty>;
ArrayContext PrintJsonArray(std::ostream& out) {
    Empty obj(out);
    return ArrayContext(out, obj);
}

using ObjectContext = InObjKeyOrEnd<Empty>;
ObjectContext PrintJsonObject(std::ostream& out) {
    Empty obj(out);
    return ObjectContext(out, obj);
}

void TestArray() {
    std::ostringstream output;

    {
        auto json = PrintJsonArray(output);
        json
            .Number(5)
            .Number(6)
            .BeginArray()
            .Number(7)
            .EndArray()
            .Number(8)
            .String("bingo!");
    }

    ASSERT_EQUAL(output.str(), R"([5,6,[7],8,"bingo!"])");
}

void TestObject() {
    std::ostringstream output;

    {
        auto json = PrintJsonObject(output);
        json
            .Key("id1").Number(1234)
            .Key("id2").Boolean(false)
            .Key("").Null()
            .Key("\"").String("\\");
    }

    //ASSERT_EQUAL(output.str(), R"({"id1":1234,"id2":false,"":null,"\\"":"\\\\"})");
}

void TestAutoClose() {
    std::ostringstream output;

    {
        auto json = PrintJsonArray(output);
        json.BeginArray().BeginObject();
    }

    ASSERT_EQUAL(output.str(), R"([[{}]])");
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestArray);
    RUN_TEST(tr, TestObject);
    RUN_TEST(tr, TestAutoClose);
    return 0;
}
