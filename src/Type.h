#pragma once
#include <map>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

#include "util.h"
enum class Type { Int = 0, Bool, Float, String, Ref, Arr, Struct };

inline std::string type_to_str(Type type) {
    switch (type) {
    case Type::Int:
        return "Int";
    case Type::Bool:
        return "Bool";
    case Type::Float:
        return "Float";
    case Type::String:
        return "String";
    case Type::Ref:
        return "Ref";
    case Type::Arr:
        return "Arr";
    case Type::Struct:
        return "Struct";
    default:
        return "Non";
    }
}

template <typename T>
constexpr auto is_basic_type_v = false;
using CppTypeSumType = std::variant<int64_t, double, std::string, bool>;
template <typename Derived, Type _type>
struct Typeinfo {
    constexpr static auto _is_ref = _type == Type::Ref;
    constexpr static auto _is_basic = is_basic_type_v<Derived>;
    constexpr Type type() const { return _type; }
    constexpr auto is_ref() { return _type == Type::Ref; }
};
