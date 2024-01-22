#include "Value.h"

#include "Context.h"
#include "Variable.h"
namespace ValueOp {

template <typename T>
    requires(!std::is_same_v<std::remove_cvref_t<T>, std::string>)
std::string operator+(const std::string l, T&& r) {
    return l + std::to_string(r);
}

template <typename T>
    requires(!std::is_same_v<std::remove_cvref_t<T>, std::string>)
std::string operator+(T&& l, const std::string r) {
    return std::to_string(l) + r;
}

template <typename T>
    requires(!std::is_same_v<std::remove_cvref_t<T>, std::string>)
bool operator<(const std::string l, T&& r) {
    return l < std::to_string(r);
}

template <typename T>
    requires(!std::is_same_v<std::remove_cvref_t<T>, std::string>)
bool operator<(T&& l, const std::string r) {
    return std::to_string(l) < r;
}

template <typename T>
    requires(!std::is_same_v<std::remove_cvref_t<T>, std::string>)
std::string operator*(T&& l, const std::string r) {
    std::size_t num = l < 0 ? 0 : l;
    std::string ret;
    while (num--) {
        ret += r;
    }
    return ret;
}

template <typename T>
    requires(!std::is_same_v<std::remove_cvref_t<T>, std::string>)
std::string operator*(const std::string l, T&& r) {
    std::size_t num = r < 0 ? 0 : r;
    std::string ret;
    while (num--) {
        ret += l;
    }
    return ret;
}

bool _less(const Value& L, const Value& R) {
    if (L.type() != R.type()) {
        return L.type() < R.type();
    }
    return std::visit([](auto&& l, auto&& r) { return l < r; }, L.getSumType(), R.getSumType());
}

Value Add(const Value& L, const Value& R) {
    return std::visit([](auto&& l, auto&& r) { return Value {l + r}; }, L.getSumType(),
                      R.getSumType());
}

Value Sub(const Value& L, const Value& R) {
    return std::visit(
            [](auto&& l, auto&& r) -> Value {
                using ltype = std::decay_t<decltype(l)>;
                using rtype = std::decay_t<decltype(r)>;
                if constexpr (std::is_same_v<ltype, std::string> ||
                              std::is_same_v<rtype, std::string>) {
                    return Value::default_value;
                } else {
                    return Value {l - r};
                }
            },
            L.getSumType(), R.getSumType());
}

Value Mul(const Value& L, const Value& R) {
    return std::visit(
            [](auto&& l, auto&& r) -> Value {
                using ltype = std::decay_t<decltype(l)>;
                using rtype = std::decay_t<decltype(r)>;
                if constexpr (std::is_same_v<ltype, std::string> &&
                              std::is_same_v<rtype, std::string>) {
                    return Value::default_value;
                    ;
                } else {
                    return Value {l * r};
                }
            },
            L.getSumType(), R.getSumType());
}

} // namespace ValueOp

RefValue::RefValue(ReferenceWrapping ref) : _ref(ref) {}

Value & RefValue::Dereference() const {
    return _ref.Dereference();
}
std::string RefValue::ref_debug_info() const {
    std::string info = "ref from : " + _ref.mgr()->_name;
    return info;
}

Value& ReferenceWrapping::Dereference() const {
    return _ref->second;
}

std::string ReferenceWrapping::ref_name() const {
    return _ref->first;
}