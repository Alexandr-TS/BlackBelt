// ���������� ����� ���� ������ �������������� ��� ��������� � "�������" Optional � ������� Value
struct BadOptionalAccess {
};

template <typename T>
class Optional {
private:
    // alignas ����� ��� ����������� ������������ ����� ������
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
        new (data) T(move(elem));
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
            new (data) T(move(other.Value()));
		}
    }

    Optional& operator=(const T& elem) {
        if (defined) {
            Value().~T();
        }
        else {
            defined = true;
        }
        new (data) T(elem);
        return *this;
    }

    Optional& operator=(T&& elem) {
        if (defined) {
            Value().~T();
        }
        else {
            defined = true;
        }
        new (data) T(move(elem));
        return *this;
    }

    Optional& operator=(const Optional& other) {
        if (defined) {
            if (other.HasValue()) {
                Value() = other.Value();
            }
            else {
                defined = other.HasValue();
                Value().~T();
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
                Value() = move(other.Value());
            }
            else {
                defined = other.HasValue();
                Value().~T();
            }
        }
        else {
            defined = other.HasValue();
            if (other.HasValue()) {
                new (data) T(move(other.Value()));
            }
        }
        return *this;
    }

    bool HasValue() const {
        return defined;
    }

    // ��� ��������� �� ������ ������ ������� �������� �� �������.
    // �������� �������� �� ������� ������������.
    T& operator*() {
        return *reinterpret_cast<T*>(data);
    }
    const T& operator*() const {
        return *reinterpret_cast<T*>(data);
    }
    T* operator->() {
        return reinterpret_cast<T*>(data);
    }
    const T* operator->() const {
        return reinterpret_cast<T*>(data);
    }

    // ���������� ���������� BadOptionalAccess, ���� ������� ���
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
            Value().~T();
            defined = false;
        }
    }

    ~Optional() {
        reinterpret_cast<T*>(data)->~T();
    }
};
