#pragma once
#include "Type.h"
#include "util.h"

struct Value;

namespace ValueOp {
bool _less(const Value& L, const Value& R);
} // namespace ValueOp

template <typename CppType, Type type>
struct BasicValue : Typeinfo<BasicValue<CppType, type>, type> {
    BasicValue(const CppType& value) : _value(value) {};
    CppType _value {};
    CppTypeSumType getSumType() const { return _value; }
    CppType CppValue() { return _value; }
    std::string to_string() const {
        if constexpr (std::is_same_v<CppType, std::string>) {
            return _value;
        } else {
            return std::to_string(_value);
        }
    }
};

template <typename CppType, Type type>
constexpr auto is_basic_type_v<BasicValue<CppType, type>> = true;

struct IntValue : public BasicValue<int64_t, Type::Int> {
    using CppType = int64_t;
    IntValue(const CppType& rhs) : BasicValue<CppType, Type::Int>(rhs) {}
};

struct FloatValue : public BasicValue<double, Type::Float> {
    using CppType = double;
    FloatValue(const CppType& rhs) : BasicValue<CppType, Type::Float>(rhs) {}
};

struct StringValue : public BasicValue<std::string, Type::String> {
    using CppType = std::string;
    StringValue(const CppType& rhs) : BasicValue<CppType, Type::String>(rhs) {}
};

struct BoolValue : public BasicValue<bool, Type::Bool> {
    using CppType = bool;
    BoolValue(const CppType& rhs) : BasicValue<CppType, Type::Bool>(rhs) {}
};

struct VariableMgr;
struct ReferenceWrapping {
    using VariableRef = std::map<std::string, Value>::iterator;
    ReferenceWrapping() {};
    ReferenceWrapping(VariableRef it, VariableMgr* mgr) : _ref(it), _mgr(mgr) {}

    Value& Dereference() const;

    VariableMgr* mgr() const { return _mgr; }

    std::string ref_name() const;

private:
    VariableRef _ref;
    VariableMgr* _mgr;
};

struct ReferenceWrapping;
struct RefValue : public Typeinfo<RefValue, Type::Ref> {
    RefValue(ReferenceWrapping ref);
    Value& Dereference() const;
    ReferenceWrapping _ref;
    std::string ref_debug_info() const;
};

struct ArrValue : public Typeinfo<ArrValue, Type::Arr> {
    ArrValue(const std::vector<Value>& arr) : _arr(arr) {};
    auto& arr() { return _arr; }

private:
    std::vector<Value> _arr;
};

struct StructValue : public Typeinfo<StructValue, Type::Struct> {
    StructValue(const std::map<std::string, Value>& map) : _map(map) {}

    auto& map() { return _map; }

private:
    std::map<std::string, Value> _map;
};

template <typename... Ts>
constexpr auto is_in_type_set = false;

template <typename U, typename head, typename... Ts>
constexpr auto is_in_type_set<U, head, Ts...> = std::is_same_v<U, head> || is_in_type_set<U, Ts...>;

template <typename U, typename T>
constexpr auto is_in_type_set<U, T> = std::is_same_v<U, T>;

template <typename U, typename T>
constexpr auto is_in_variant = false;

template <typename U, typename... Ts>
constexpr auto is_in_variant<U, std::variant<Ts...>> = is_in_type_set<U, Ts...>;

struct Value {
    using autoType = std::variant<IntValue, BoolValue, FloatValue, StringValue, RefValue, ArrValue,
                                  StructValue>;

    static Value default_value;

    Value() : _value(IntValue(0)) {}
    Value(const Value& rhs) = default;
    Value& operator=(const Value& rhs) = default;
    ~Value() = default;

    bool is_ref() const { return type() == Type::Ref; }

    bool is_arr() const { return type() == Type::Arr; }

    bool is_str() const { return type() == Type::String; }

    bool is_struct() const { return type() == Type::Struct; }

    template <typename T>
        requires requires(T x) { x.type(); }
    Value(const T& rhs) : _value(rhs) {}

    Value(ReferenceWrapping rhs) : _value(RefValue {rhs}) {}

    template <typename CppType>
        requires(std::is_integral_v<CppType> && !std::is_same_v<CppType, bool>)
    Value(const CppType& rhs) : _value(IntValue(rhs)) {}

    template <typename CppType>
        requires(std::is_same_v<CppType, bool>)
    Value(const CppType& rhs) : _value(BoolValue(rhs)) {};

    template <typename CppType>
        requires(std::is_floating_point_v<CppType>)
    Value(const CppType& rhs) : _value(FloatValue(rhs)) {}

    Value(const std::string& rhs) : _value(StringValue(rhs)) {}

    Type type() const {
        return std::visit([](auto&& v) { return v.type(); }, _value);
    }

    Value& ref() const {
        CHECK(is_ref(), "use ref must be a ref type now is " + type_to_str(type()));
        return std::get<RefValue>(_value).Dereference();
    }

    std::string ref_debug_info() const {
        return std::visit(
                [](auto&& v) -> std::string {
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<RefValue, T>) {
                        return v.ref_debug_info();
                    } else {
                        return "";
                    }
                },
                _value);
    }
    std::string to_string() const {
        if (is_ref()) {
            return ref().to_string();
        }
        return std::visit(
                [](auto&& v) -> std::string {
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (T::_is_basic) {
                        return v.to_string();
                    }
                    CHECK(0, "should not reach here.");
                    return "";
                },
                _value);
    }

    CppTypeSumType getSumType() const {
        if (is_ref()) {
            return ref().getSumType();
        }
        return std::visit(
                [](auto&& v) -> CppTypeSumType {
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (T::_is_basic) {
                        return v.getSumType();
                    }
                    CHECK(0, "should not reach here.");
                    return "";
                },
                _value);
    }

    int64_t get_int() const {
        return std::visit(
                [](auto&& v) -> int64_t {
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::string>) {
                        return std::stoll(v);
                    } else {
                        return v;
                    }
                },
                getSumType());
    }

    bool get_bool() const { return get_int(); }

    Value& operator[](Value idx) {
        if (is_ref()) return ref()[idx];
        auto& arr = std::get<ArrValue>(_value).arr();
        int id = idx.get_int();
        CHECK(id < arr.size(), "out of bound in arr" + std::string(" now size ") +
                                       std::to_string(arr.size()) + " want " + std::to_string(id));
        return arr[id];
    }

    std::vector<Value> get_arr() {
        if (is_ref()) return ref().get_arr();
        auto& arr = std::get<ArrValue>(_value).arr();
        return arr;
    }

    Value& operator[](const std::string& member_name) {
        if (is_ref()) return ref()[member_name];
        auto& map = std::get<StructValue>(_value).map();
        return map[member_name];
    }

    std::map<std::string, Value> get_struct() {
        if (is_ref()) return ref().get_struct();
        auto& map = std::get<StructValue>(_value).map();
        return map;
    }

    // make function
    template <typename CppType>
        requires(std::is_integral_v<CppType>)
    static auto make_Int(const CppType& rhs) {
        return Value {IntValue(rhs)};
    }

    template <typename CppType>
        requires(std::is_same_v<CppType, bool>)
    static auto make_Bool(const CppType& rhs) {
        return Value {BoolValue(rhs)};
    }

    template <typename CppType>
        requires(std::is_floating_point_v<CppType>)
    static auto make_Float(const CppType& rhs) {
        return Value {FloatValue(rhs)};
    }

    template <typename CppType>
        requires(std::is_same_v<CppType, std::string>)
    static auto make_Int(const CppType& rhs) {
        return Value {StringValue(rhs)};
    }

    static auto make_Ref(ReferenceWrapping ref) { return Value {ref}; }

    static auto make_Arr(const std::vector<Value>& arr) {
        Value v;
        v._value = arr;
        return v;
    }

    static auto make_Struct(const std::map<std::string, Value>& map) {
        Value v;
        v._value = map;
        return v;
    }

    void assign_value(const Value& rhs) {
        if (is_ref()) {
            ref() = rhs;
        } else {
            *this = rhs;
        }
    }
    autoType& value() { return _value; }

private:
    autoType _value;
};

inline std::ostream& operator<<(std::ostream& out, const Value& v) {
    out << v.to_string();
    return out;
}

namespace ValueOp {

Value Add(const Value& L, const Value& R);
Value Sub(const Value& L, const Value& R);
Value Mul(const Value& L, const Value& R);
inline Value Less(const Value& L, const Value& R) {
    return _less(L, R);
}
inline Value Greater(const Value& L, const Value& R) {
    return !_less(L, R);
}
inline Value Equal(const Value& L, const Value& R) {
    return Value {!_less(L, R) && !_less(R, L)};
}

inline Value NotEqual(const Value& L, const Value& R) {
    return Value {!(!_less(L, R) && !_less(R, L))};
}

inline Value GreaterOrEqual(const Value& L, const Value& R) {
    return (!_less(L, R)) || (!_less(L, R) && !_less(R, L));
}

inline Value LessOrEqual(const Value& L, const Value& R) {
    return (_less(L, R)) || (!_less(L, R) && !_less(R, L));
}

inline Value And(const Value& L, const Value& R) {
    return L.get_bool() && R.get_bool();
}

inline Value Or(const Value& L, const Value& R) {
    return L.get_bool() || R.get_bool();
}

} // namespace ValueOp
