#include "Function.h"

#include "Ast.h"
#include "Context.h"

Value DefFunction::exce(Context* ctx, std::vector<Value> args) {
    ctx->push_func(func_name());
    for (int i = 0; i < args.size(); i++) {
        ctx->func()->variable_mgr()->set(_args_name[i], args[i]);
    }
    auto ret = _funcnode->exec(ctx);
    ctx->pop_func();
    return ctx->ret();
}

namespace build_in_function {

struct println {
    constexpr static auto name = "println";
    constexpr static auto counter = 1;

    static Value exec(Context* ctx, std::vector<Value> args) {
        std::cout << args[0] << "\n";
        return Value::default_value;
    }
};

struct type {
    constexpr static auto name = "type";
    constexpr static auto counter = 1;

    static Value exec(Context* ctx, std::vector<Value> args) {
        Value v = args[0];
        std::string type_info = type_to_str(v.type());
        if (v.type() == Type::Ref) {
            type_info += "  " + v.ref_debug_info();
        }
        return type_info;
    }
};

struct len {
    constexpr static auto name = "len";
    constexpr static auto counter = 1;

    static Value exec(Context* ctx, std::vector<Value> args) {
        Value v = args[0];
        if (v.is_arr()) {
            return std::get<ArrValue>(v.value()).arr().size();
        } else if (v.is_str()) {
            return std::get<StringValue>(v.value()).CppValue().size();
        } else if (v.is_ref()) {
            return exec(ctx, {v.ref()});
        }
        return 0;
    }
};

struct input {
    constexpr static auto name = "input";
    constexpr static auto counter = 0;

    static Value exec(Context* ctx, std::vector<Value> args) {
        std::string input_str;
        std::cin >> input_str;
        return input_str;
    }
};

struct Int {
    constexpr static auto name = "int";
    constexpr static auto counter = 1;

    static Value exec(Context* ctx, std::vector<Value> args) {
        Value v = args[0];
        return v.get_int();
    }
};

struct trans {
    constexpr static auto name = "trans";
    constexpr static auto counter = 1;

    static Value exec(Context* ctx, std::vector<Value> args) {
        Value v = args[0];
        CHECK(v.is_struct(), "trans must use in struct");
        std::vector<Value> arr;
        auto map = v.get_struct();
        for (auto& [name, var] : map) {
            arr.push_back(var);
        }
        return Value::make_Arr(arr);
    }
};

struct range_3 {
    constexpr static auto name = "range";
    constexpr static auto counter = 3;

    static Value exec(Context* ctx, std::vector<Value> args) {
        Value L = args[0];
        Value R = args[1];
        Value step = args[2];
        std::vector<Value> arr;
        for (auto i = L.get_int(); i < R.get_int(); i += step.get_int()) {
            arr.push_back(i);
        }
        return Value::make_Arr(arr);
    }
};

struct range_2 {
    constexpr static auto name = "range";
    constexpr static auto counter = 2;

    static Value exec(Context* ctx, std::vector<Value> args) {
        args.push_back(1);
        return range_3::exec(ctx, args);
    }
};

}; // namespace build_in_function

void FunctionMgr::built_in_functions() {
    register_built_in<build_in_function::println>();
    register_built_in<build_in_function::type>();
    register_built_in<build_in_function::len>();
    register_built_in<build_in_function::input>();
    register_built_in<build_in_function::Int>();
    register_built_in<build_in_function::trans>();
    register_built_in<build_in_function::range_3>();
    register_built_in<build_in_function::range_2>();
}