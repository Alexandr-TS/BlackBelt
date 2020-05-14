#include <cstddef>
#include <utility>
#include <memory>

template <typename T>
struct RawMemory {
    T* buf = nullptr;
    size_t cp = 0;

    static T* Allocate(size_t n) {
        return static_cast<T*>(operator new(n * sizeof(T)));
    }

    void Deallocate(T* buf) {
        operator delete(buf);
    }

    RawMemory& operator = (RawMemory<T>&& rm) {
        this->Swap(rm);
        return *this;
    }

    RawMemory(RawMemory&& rm) noexcept {
        this->Swap(rm);
    }
    RawMemory()
        : buf(nullptr)
        , cp(0)
    {}
    RawMemory(size_t n) {
        buf = RawMemory::Allocate(n);
        cp = n;
    }

    ~RawMemory() {
        Deallocate(buf);
    }

    void Swap(RawMemory<T>& rm) noexcept {
        std::swap(buf, rm.buf);
        std::swap(cp, rm.cp);
    }
};

template <typename T>
class Vector {
public:
    Vector()
        : sz(0)
    {
    }

    Vector(size_t n)
        : sz(n) {
        data = RawMemory<T>(n);
        std::uninitialized_value_construct_n(data.buf, n);
    }

    Vector(const Vector& other)
        : sz(other.Size()) {
        data = RawMemory<T>(other.Size());
        std::uninitialized_copy_n(other.data.buf, other.Size(), data.buf);
    }
    Vector(Vector&& other) {
        sz = std::move(other.sz);
        other.sz = 0;
        data.Swap(other.data);
    }

    ~Vector() {
        std::destroy_n(data.buf, sz);
    }

    Vector& operator = (const Vector& other) {
        if (data.cp < other.Size()) {
            std::destroy_n(data.buf, sz);
            auto data2 = RawMemory<T>(other.Size());
            std::uninitialized_copy_n(other.data.buf, other.sz, data2.buf);
            data.Swap(data2);
        }
        else {
            if (sz > other.Size()) {
                std::destroy_n(data.buf + other.Size(), sz - other.Size());
            }
            for (size_t i = 0; i < other.sz; ++i) {
                *(data.buf + i) = *(other.data.buf + i);
            }
        }
        sz = other.Size();
        return *this;
    }

    Vector& operator = (Vector&& other) noexcept {
        std::swap(sz, other.sz);
        data.Swap(other.data);
        return *this;
    }

    void Reserve(size_t n) {
        if (n <= data.cp) return;
        auto data2 = RawMemory<T>(n);
        std::uninitialized_move_n(data.buf, sz, data2.buf);
        std::destroy_n(data.buf, sz);
        data = std::move(data2);
    }

    void Resize(size_t n) {
        Reserve(n);
        if (n <= sz) {
            std::destroy_n(data.buf + n, sz - n);
        }
        else {
            std::uninitialized_value_construct_n(data.buf + sz, n - sz);
        }
        sz = n;
    }

    void PushBack(const T& elem) {
        DoubleReserve();
        new (data.buf + sz) T(elem);
        sz++;
    }

    void PushBack(T&& elem) {
        if (sz == data.cp) {
            Reserve(!sz ? 1 : 2 * sz);
        }
        new (data.buf + sz) T(std::move(elem));
        sz++;
    }

    template <typename ... Args>
    T& EmplaceBack(Args&& ... args) {
        if (sz == data.cp) {
            Reserve(!sz ? 1 : 2 * sz);
        }
        auto elem = new (data.buf + sz) T(std::forward<Args>(args)...);
        sz++;
        return *elem;
    }

    void PopBack() {
        std::destroy_at(data.buf + sz - 1);
        sz--;
    }

    size_t Size() const noexcept {
        return sz;
    }

    size_t Capacity() const noexcept {
        return data.cp;
    }

    const T& operator[](size_t i) const {
        return *(data.buf + i);
    }

    T& operator[](size_t i) {
        return *(data.buf + i);
    }

    // В данной части задачи реализуйте дополнительно эти функции:
    using iterator = T*;
    using const_iterator = const T*;

    iterator begin() noexcept {
        return data.buf;
    }
    iterator end() noexcept {
        return data.buf + sz;
    }

    const_iterator begin() const noexcept {
        return data.buf;
    }
    const_iterator end() const noexcept {
        return data.buf + sz;
    }

    // Тут должна быть такая же реализация, как и для константных версий begin/end
    const_iterator cbegin() const noexcept {
        return data.buf;
    }
    const_iterator cend() const noexcept {
        return data.buf + sz;
    }

    // Вставляет элемент перед pos
    // Возвращает итератор на вставленный элемент
    iterator Insert(const_iterator pos, const T& elem) {
        size_t shift = pos - data.buf;
        DoubleReserve();
        for (auto it = data.buf + sz - 1; it != data.buf + shift - 1; --it) {
            std::uninitialized_move_n(it, 1, it + 1);
        }
        sz++;
        return new (data.buf + shift) T(elem);
    }

    iterator Insert(const_iterator pos, T&& elem) {
        size_t shift = pos - data.buf;
        DoubleReserve();
        for (iterator it = data.buf + sz - 1; it != data.buf + shift - 1; --it) {
            std::uninitialized_move_n(it, 1, it + 1);
        }
        sz++;
        return new (data.buf + shift) T(std::move(elem));
    }

    // Конструирует элемент по заданным аргументам конструктора перед pos
    // Возвращает итератор на вставленный элемент
    template <typename ... Args>
    iterator Emplace(const_iterator it, Args&&... args) {
        size_t shift = it - data.buf;
        DoubleReserve();
        for (iterator iter = data.buf + sz - 1; iter != data.buf + shift - 1; --iter) {
            std::uninitialized_move_n(iter, 1, iter + 1);
        }
        sz++;
        return new (data.buf + shift) T(std::forward<Args>(args)...);
    }

    // Удаляет элемент на позиции pos
    // Возвращает итератор на элемент, следующий за удалённым
    iterator Erase(const_iterator it) {
        std::destroy_at(it);
        for (iterator iter = data.buf + (it - data.buf) + 1; 
            iter != data.buf + sz; ++iter) {
            std::uninitialized_move_n(iter, 1, iter - 1);
        }
        sz--;
        size_t shift = it - data.buf;
        return data.buf + shift;
    }

private:
    void DoubleReserve() {
        if (sz == data.cp) {
            Reserve(!sz ? 1 : 2 * sz);
        }
    }

    RawMemory<T> data;
    size_t sz = 0;
};
