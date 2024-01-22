#pragma once

#include <map>
#include <optional>
#include <string>
#include <utility>

#include "Value.h"
#include "iostream"
#include "util.h"

struct Context;
struct FuncNode;

using FunctionSignature = std::pair<std::string, int>;

struct Function {
    Function(const FunctionSignature& signature) : _signature(signature) {}
    virtual Value exce(Context* ctx, std::vector<Value> args) = 0;
    std::string func_name() const { return _signature.first; }
    auto args() const { return _signature.second; }

private:
    const FunctionSignature _signature;
};

struct BuiltInFunction : Function {
    using BuiltInType = std::function<Value(Context*, std::vector<Value>)>;
    BuiltInFunction(BuiltInType func, const FunctionSignature& signature)
            : _func(func), Function(signature) {}
    Value exce(Context* ctx, std::vector<Value> args) override {
        std::vector<Value> remove_ref_args;
        for (auto& v : args) {
            if (v.is_ref())
                remove_ref_args.push_back(v.ref());
            else
                remove_ref_args.push_back(v);
        }
        return _func(ctx, remove_ref_args);
    }
    BuiltInType _func;
};

struct DefFunction : public Function {
    DefFunction(FuncNode* funcnode,  const std::vector<std::string>& args_name,
                const FunctionSignature& signature)
            : _funcnode(funcnode), _args_name(args_name), Function(signature) {}

    Value exce(Context* ctx, std::vector<Value> args) override;
    virtual ~DefFunction() = default;
    FuncNode* _funcnode;
    const std::vector<std::string> _args_name;
};

struct FunctionMgr {
    FunctionMgr() { built_in_functions(); }
    std::map<FunctionSignature, std::shared_ptr<Function>> _mgr;
    auto get_func(FunctionSignature signature) {
        CHECK(_mgr.contains(signature), "can not find funciton name  " + signature.first);
        auto func = _mgr[signature];
        return func;
    }

    void register_func(std::string name, std::vector<std::string> args_name, FuncNode* func_node) {
        FunctionSignature signature = {name, args_name.size()};
        _mgr.insert({signature, std::make_shared<DefFunction>(func_node, args_name, signature)});
    }

    std::optional<FunctionSignature> get_func_signature(std::string name) {
        for (auto& [signature, _] : _mgr) {
            auto& [func_name, args] = signature;
            if (func_name == name) {
                return signature;
            }
        }
        return std::nullopt;
    }

private:
    template <typename build_in_func>
    void register_built_in() {
        FunctionSignature signature = {build_in_func::name, build_in_func::counter};
        _mgr.insert({signature, std::make_shared<BuiltInFunction>(build_in_func::exec, signature)});
    }
    void built_in_functions();
};
