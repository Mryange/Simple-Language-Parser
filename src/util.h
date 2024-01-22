#pragma once
#include <cstdlib>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>
#define ENABLE_FACTORY_CREATOR(TypeName)                                             \
private:                                                                             \
    void* operator new(std::size_t size) {                                           \
        return ::operator new(size);                                                 \
    }                                                                                \
    void* operator new[](std::size_t size) {                                         \
        return ::operator new[](size);                                               \
    }                                                                                \
                                                                                     \
public:                                                                              \
    void* operator new(std::size_t count, void* ptr) {                               \
        return ::operator new(count, ptr);                                           \
    }                                                                                \
    void operator delete(void* ptr) noexcept {                                       \
        ::operator delete(ptr);                                                      \
    }                                                                                \
    void operator delete[](void* ptr) noexcept {                                     \
        ::operator delete[](ptr);                                                    \
    }                                                                                \
    void operator delete(void* ptr, void* place) noexcept {                          \
        ::operator delete(ptr, place);                                               \
    }                                                                                \
    template <typename... Args>                                                      \
    static std::shared_ptr<TypeName> create_shared(Args&&... args) {                 \
        return std::make_shared<TypeName>(std::forward<Args>(args)...);              \
    }                                                                                \
    template <typename... Args>                                                      \
    static std::unique_ptr<TypeName> create_unique(Args&&... args) {                 \
        return std::unique_ptr<TypeName>(new TypeName(std::forward<Args>(args)...)); \
    }

#define CHECK(expr, info)                                                                \
    if (!(expr)) {                                                                       \
        std::cout << __FILE__ << ":" << __LINE__ << "  " << __FUNCTION__ << "\t"         \
                  << "failed check " + std::string(#expr) << "  info :" << info << "\n"; \
        exit(0);                                                                         \
    }

// =======function by gpt =======
static std::optional<long long> tryParseInt(const std::string& s) {
    std::istringstream iss(s);
    long long value;
    if (iss >> value && iss.eof()) {
        return value;
    } else {
        return std::nullopt; // 表示解析失败
    }
}

static std::optional<double> tryParseFloat(const std::string& s) {
    std::istringstream iss(s);
    double value;
    if (iss >> value && iss.eof()) {
        return value;
    } else {
        return std::nullopt; // 表示解析失败
    }
}

static std::optional<bool> tryParseBool(const std::string& s) {
    std::string lowercased = s;
    std::transform(lowercased.begin(), lowercased.end(), lowercased.begin(), ::tolower);

    if (lowercased == "true") {
        return true;
    } else if (lowercased == "false") {
        return false;
    } else {
        return std::nullopt; // 表示解析失败
    }
}

// =======function by gpt =======

class ObjectPool {
public:
    ObjectPool() = default;

    ~ObjectPool() { clear(); }

    template <class T>
    T* add(T* t) {
        _objects.emplace_back(Element {t, [](void* obj) { delete reinterpret_cast<T*>(obj); }});
        return t;
    }

    void clear() {
        for (auto obj = _objects.rbegin(); obj != _objects.rend(); obj++) {
            obj->delete_fn(obj->obj);
        }
        _objects.clear();
    }

    void acquire_data(ObjectPool* src) {
        _objects.insert(_objects.end(), src->_objects.begin(), src->_objects.end());
        src->_objects.clear();
    }

    uint64_t size() { return _objects.size(); }

private:
    ObjectPool(const ObjectPool&) = delete;
    void operator=(const ObjectPool&) = delete;

    /// A generic deletion function pointer. Deletes its first argument.
    using DeleteFn = void (*)(void*);

    /// For each object, a pointer to the object and a function that deletes it.
    struct Element {
        void* obj = nullptr;
        DeleteFn delete_fn;
    };

    std::vector<Element> _objects;
};

#define RETURN_IF_TRUE(stmt) \
    auto __ret__ = (stmt);   \
    if ((__ret__)) {         \
        return __ret__;      \
    }
