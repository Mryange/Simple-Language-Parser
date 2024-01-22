#pragma once
#include "Function.h"
#include "Value.h"
#include "Variable.h"
#include "util.h"
struct Context;

struct ExprNode {
    virtual Value exec(Context*) = 0;
    ExprNode* child(int id) const  {
        CHECK(id < _child.size(), "out of bound");
        return _child[id].get();
    }
    void add_child(std::unique_ptr<ExprNode> node) { _child.push_back(std::move(node)); }
    std::vector<std::unique_ptr<ExprNode>> _child;
    virtual ~ExprNode() = default;
    virtual int Opnums() = 0;
    virtual bool is_variable() { return false; }
    virtual bool is_constant() { return false; }
    virtual void prepare(Context* ctx) {
        CHECK(_child.size() == Opnums(), "child size can not macth " +
                                                 std::to_string(_child.size()) + " vs " +
                                                 std::to_string(Opnums()));
        for (auto& child : _child) {
            child->prepare(ctx);
        }
    }
    virtual std::string name() const = 0;
};

struct ConstantValueNode : ExprNode {
    ENABLE_FACTORY_CREATOR(ConstantValueNode);

    template <typename CppType>
    ConstantValueNode(const CppType value) : _value(value) {};

    bool is_constant() override { return true; }

    Value exec(Context*) override { return _value; }

    Value _value;

    int Opnums() override { return 0; }

    std::string name() const override { return _value.to_string(); }
};

template <typename BinaryOp>
struct BinaryOperator : ExprNode {
    ENABLE_FACTORY_CREATOR(BinaryOperator);

    Value exec(Context* ctx) override {
        if (_fold) {
            return _fold_value;
        }
        return BinaryOp::op(Lchild()->exec(ctx), Rchild()->exec(ctx));
    }

    ExprNode* Lchild()  const { return child(0); }

    ExprNode* Rchild() const  { return child(1); }

    int Opnums() override { return 2; }

    std::string name() const override {
        if (_fold) {
            return _fold_value.to_string();
        }
        return Lchild()->name() + " " + BinaryOp::name + " " + Rchild()->name();
    }

    bool is_constant() override { return _fold; }

    void prepare(Context* ctx) override {
        ExprNode::prepare(ctx);
        if (Lchild()->is_constant() && Rchild()->is_constant()) {
            _fold_value = BinaryOp::op(Lchild()->exec(ctx), Rchild()->exec(ctx));
            _fold = true;
            _child.clear();
        }
    }

private:
    bool _fold = false;
    Value _fold_value;
};

struct AddOperator : BinaryOperator<AddOperator> {
    inline static auto op = ValueOp::Add;
    inline static auto name = "+";
    ENABLE_FACTORY_CREATOR(AddOperator);
};

struct SubOperator : BinaryOperator<SubOperator> {
    inline static auto op = ValueOp::Sub;
    inline static auto name = "-";
    ENABLE_FACTORY_CREATOR(SubOperator);
};

struct EqualOperator : BinaryOperator<EqualOperator> {
    inline static auto op = ValueOp::Equal;
    inline static auto name = "==";
    ENABLE_FACTORY_CREATOR(EqualOperator);
};

struct NotEqualOperator : BinaryOperator<NotEqualOperator> {
    inline static auto op = ValueOp::NotEqual;
    inline static auto name = "!=";
    ENABLE_FACTORY_CREATOR(NotEqualOperator);
};

struct MulOperator : BinaryOperator<MulOperator> {
    inline static auto op = ValueOp::Mul;
    inline static auto name = "*";
    ENABLE_FACTORY_CREATOR(MulOperator);
};

struct LessOperator : BinaryOperator<LessOperator> {
    inline static auto op = ValueOp::Less;
    inline static auto name = "<";
    ENABLE_FACTORY_CREATOR(LessOperator);
};

struct GreaterOperator : BinaryOperator<GreaterOperator> {
    inline static auto op = ValueOp::Greater;
    inline static auto name = ">";
    ENABLE_FACTORY_CREATOR(GreaterOperator);
};

struct GreaterOrEqualOperator : BinaryOperator<GreaterOrEqualOperator> {
    inline static auto op = ValueOp::GreaterOrEqual;
    inline static auto name = ">=";
    ENABLE_FACTORY_CREATOR(GreaterOrEqualOperator);
};

struct LessOrEqualOperator : BinaryOperator<LessOrEqualOperator> {
    inline static auto op = ValueOp::LessOrEqual;
    inline static auto name = "<=";
    ENABLE_FACTORY_CREATOR(LessOrEqualOperator);
};

struct AndOperator : BinaryOperator<AndOperator> {
    inline static auto op = ValueOp::And;
    inline static auto name = "&&";
    ENABLE_FACTORY_CREATOR(AndOperator);
};

struct OrOperator : BinaryOperator<OrOperator> {
    inline static auto op = ValueOp::Or;
    inline static auto name = "||";
    ENABLE_FACTORY_CREATOR(OrOperator);
};

struct VariableNode : ExprNode {
    ENABLE_FACTORY_CREATOR(VariableNode);
    VariableNode(const std::string& name) : _name(name) {}

    Value exec(Context* ctx) override;
    virtual Value& get_variable(Context* ctx);
    ReferenceWrapping& ref(Context* ctx);
    int Opnums() override { return 0; }

    std::string name() const override { return _name; }
    bool is_variable() override { return true; }

private:
    void create_reference_wrapping(Context* ctx);
    ReferenceWrapping _ref;
    std::string _name;
};

struct SubscriptOperator : VariableNode {
    ENABLE_FACTORY_CREATOR(SubscriptOperator);
    SubscriptOperator() : VariableNode("[]") {}
    int Opnums() override { return 2; }
    Value exec(Context* ctx) override;
    Value& get_variable(Context* ctx) override;
};

struct MemberAccessOperator : VariableNode {
    ENABLE_FACTORY_CREATOR(MemberAccessOperator);
    MemberAccessOperator() : VariableNode("[]") {}
    int Opnums() override { return 2; }
    Value exec(Context* ctx) override;
    Value& get_variable(Context* ctx) override;
};

inline Value fake_binary_op(const Value&, const Value&) {
    return Value::default_value;
}

struct AssignOperator : BinaryOperator<AssignOperator> {
    inline static auto op = fake_binary_op;
    inline static auto name = "=";
    ENABLE_FACTORY_CREATOR(AssignOperator);
    Value exec(Context* ctx) override {
        CHECK(Lchild()->is_variable(), "= must use in var");
        Value& ref = dynamic_cast<VariableNode*>(Lchild())->get_variable(ctx);
        ref.assign_value((Rchild()->exec(ctx)));
        return ref;
    }
    int Opnums() override { return 2; }
};

struct Function;
using FunctionPtr = std::shared_ptr<Function>;

struct FuncCallOperator : ExprNode {
    ENABLE_FACTORY_CREATOR(FuncCallOperator);

    FuncCallOperator(FunctionSignature signature) : _signature(signature) {}
    Value exec(Context* ctx) override;
    int Opnums() override { return _signature.second; }

    std::string name() const override {
        auto func_call_name = _signature.first + "(";

        for (auto& child_expr : _child) {
            func_call_name += child_expr->name() + " ";
        }

        func_call_name += ")";

        return func_call_name;
    }

    void prepare(Context* ctx) override;

    FunctionSignature _signature;

    std::shared_ptr<Function> _func_ptr;
};

struct RefOperator : ExprNode {
    ENABLE_FACTORY_CREATOR(RefOperator);

    RefOperator() {};
    Value exec(Context* ctx) override;
    int Opnums() override { return 1; }

    std::string name() const override {
        return "Ref " + _child[0]->name();
    }
};

struct ArrOperator : ExprNode {
    ENABLE_FACTORY_CREATOR(ArrOperator);
    Value exec(Context* ctx) override;
    int Opnums() override { return 1; }
     std::string name() const override {
        return "Arr " + _child[0]->name();
    }
};

struct StructOperator : ExprNode {
    ENABLE_FACTORY_CREATOR(StructOperator);
    Value exec(Context* ctx) override;
    int Opnums() override { return 1; }
    std::string name() const override {
        return "Let " + _child[0]->name();
    }
};