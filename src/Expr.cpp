#include "Expr.h"

#include <vector>

#include "Context.h"
#include "Value.h"

void VariableNode::create_reference_wrapping(Context* ctx) {
    _ref = ctx->variable_ref(_name);
}

Value& VariableNode::get_variable(Context* ctx) {
    create_reference_wrapping(ctx);
    return _ref.Dereference();
}

Value VariableNode::exec(Context* ctx) {
    create_reference_wrapping(ctx);
    return _ref.Dereference();
}

ReferenceWrapping& VariableNode::ref(Context* ctx) {
    return ctx->variable_ref(_name);
}

void FuncCallOperator::prepare(Context* ctx) {
    ExprNode::prepare(ctx);
    _func_ptr = ctx->func_mgr()->get_func(_signature);
}

Value FuncCallOperator::exec(Context* ctx) {
    std::vector<Value> args;
    for (auto& arg : _child) {
        args.push_back(arg->exec(ctx));
    }
    return _func_ptr->exce(ctx, args);
}

Value RefOperator::exec(Context* ctx) {
    CHECK(_child.size() == 1, "ref must only one child");
    CHECK(child(0)->is_variable(), "ref must be use in variable");
    ReferenceWrapping Ref = dynamic_cast<VariableNode*>(child(0))->ref(ctx);
    return Value::make_Ref(Ref);
}

Value ArrOperator::exec(Context* ctx) {
    std::vector<Value> arr(child(0)->exec(ctx).get_int());
    return Value::make_Arr(arr);
}


Value StructOperator::exec(Context* ctx) {
    CHECK(child(0)->is_variable(), " struct name ");
    auto struct_name = dynamic_cast<VariableNode*>(child(0))->name();
    return ctx->struct_info()->get_default_struct_value(struct_name);
}

Value SubscriptOperator::exec(Context* ctx) {
    return get_variable(ctx);
}

Value& SubscriptOperator::get_variable(Context* ctx) {
    CHECK(_child.size() == 2, "[] must only one child");
    CHECK(child(0)->is_variable(), "[] must be use in variable");
    Value& var = dynamic_cast<VariableNode*>(child(0))->get_variable(ctx);
    return var[child(1)->exec(ctx)];
}


Value MemberAccessOperator::exec(Context* ctx) {
    return get_variable(ctx);
}

Value& MemberAccessOperator::get_variable(Context* ctx) {
    CHECK(_child.size() == 2, "[] must only one child");
    CHECK(child(0)->is_variable(), " var.var ");
    CHECK(child(1)->is_variable(), " var.var ");
    Value& var = dynamic_cast<VariableNode*>(child(0))->get_variable(ctx);
    return var[dynamic_cast<VariableNode*>(child(1))->name()];
}