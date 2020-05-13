#include <sstream>
#include <stdexcept>
#include <iostream>
#include <map>
#include <unordered_map>
#include <set>
#include <string>
#include <vector>


// Исключение этого типа должно генерироваться при обращении к "пустому" Optional в функции Value
struct BadOptionalAccess {
};

template <typename T>
class Optional {
private:
    // alignas нужен для правильного выравнивания блока памяти
    alignas(T) unsigned char data[sizeof(T)];
    bool defined = false;

public:
    Optional() = default;
    Optional(const T& elem) 
        : defined(true) {
		new (data) T(elem);
    }

    Optional(T&& elem) 
        : defined(true) {
        new (data) T(std::move(elem));
    }

    Optional(const Optional& other) 
        : defined(other.HasValue()) {
		if (other.HasValue()) {
            new (data) T(other.Value());
		}
    }

    Optional(Optional&& other) 
        : defined(other.HasValue()) {
		if (other.HasValue()) {
            new (data) T(std::move(other.Value()));
		}
    }

    Optional& operator=(const T& elem) {
        if (defined) {
            Value() = elem;
        }
        else {
            defined = true;
            new (data) T(elem);
        }
        return *this;
    }

    Optional& operator=(T&& elem) {
        if (defined) {
            Value() = std::move(elem);
        }
        else {
            defined = true;
            new (data) T(std::move(elem));
        }
        return *this;
    }

    Optional& operator=(const Optional& other) {
        if (defined) {
            if (other.HasValue()) {
                Value() = other.Value();
            }
            else {
                defined = other.HasValue();
                reinterpret_cast<T*>(data)->~T();
            }
        }
        else {
            defined = other.HasValue();
            if (other.HasValue()) {
                new (data) T(other.Value());
            }
        }
        return *this;
    }

    Optional& operator=(Optional&& other) {
        if (defined) {
            if (other.HasValue()) {
                Value() = std::move(other.Value());
            }
            else {
                defined = other.HasValue();
                reinterpret_cast<T*>(data)->~T();
            }
        }
        else {
            defined = other.HasValue();
            if (other.HasValue()) {
                new (data) T(std::move(other.Value()));
            }
        }
        return *this;
    }

    bool HasValue() const {
        return defined;
    }

    // Эти операторы не должны делать никаких проверок на пустоту.
    // Проверки остаются на совести программиста.
    T& operator*() {
        return *reinterpret_cast<T*>(data);
    }
    const T& operator*() const {
        return *reinterpret_cast<const T*>(data);
    }
    T* operator->() {
        return reinterpret_cast<T*>(data);
    }
    const T* operator->() const {
        return reinterpret_cast<const T*>(data);
    }

    // Генерирует исключение BadOptionalAccess, если объекта нет
    T& Value() {
        if (!defined) {
            throw BadOptionalAccess();
        }
        return *reinterpret_cast<T*>(data);
    }
    const T& Value() const {
        if (!defined) {
            throw BadOptionalAccess();
        }
        return *reinterpret_cast<const T*>(data);
    }

    void Reset() {
        if (defined) {
            reinterpret_cast<T*>(data)->~T();
            defined = false;
        }
    }

    ~Optional() {
        if (defined) {
            reinterpret_cast<T*>(data)->~T();
        }
    }
};
