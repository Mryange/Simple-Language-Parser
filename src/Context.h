#pragma once
#include "Function.h"
#include "Variable.h"
#include "util.h"

struct ExprNode;
struct FuncNode;


struct FunctionContext {
    FunctionContext(const std::string name) : _func_name(name) {
        _func_var = std::make_unique<VariableMgr>("func VariableMgr, func name " + _func_name);
    }
    VariableMgr* variable_mgr() { return _func_var.get(); }

private:
    std::string _func_name;
    std::unique_ptr<VariableMgr> _func_var;
};
struct GlobalContext {
    GlobalContext();
    void prepare(Context* ctx);
private:
    friend class Context;
    ObjectPool _obj_pool;
    std::vector<FunctionContext> _func_stack;
    std::unique_ptr<FunctionMgr> _function_mgr;
    std::unique_ptr<StructInfoMgr> _struct_mgr;
    std::vector<FuncNode*> _func_nodes;
};

struct Context {
public:
    Context(GlobalContext* global) : _global(global) {}

    auto global() const { return _global; }
    auto* func() { return &global()->_func_stack.back(); }


    // function level
    VariableMgr* variable_mgr(const std::string& name) { return func()->variable_mgr(); }

    ReferenceWrapping& variable_ref(const std::string& name) {
        return func()->variable_mgr()->get_ref(name);
    }


    // global level
    void push_func(const std::string& name) { global()->_func_stack.emplace_back(name); }
    void pop_func() { global()->_func_stack.pop_back(); }

    auto* obj_pool() { return &global()->_obj_pool; }

    auto* func_mgr() { return global()->_function_mgr.get(); }

    void add_func_node(FuncNode* func_node) { global()->_func_nodes.push_back(func_node); }

    std::vector<FuncNode*> all_func_node() { return global()->_func_nodes; }

    auto* struct_info() const { return global()->_struct_mgr.get(); }


    Value ret() { return _ret; }
    void set_ret(Value ret) { _ret = ret; }


private:
    GlobalContext* _global;

    Value _ret = Value::default_value;


};
