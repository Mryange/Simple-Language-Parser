#pragma once

#include <stack>

#include "Expr.h"
#include "Token.h"

struct Context;

struct Expr {
    Expr() {}
    Expr(std::unique_ptr<ExprNode> node) : _node(std::move(node)) {}
    auto operator()(Context* ctx) { return _node->exec(ctx); }
    auto& node() { return _node; }
    auto expr_name() { return _node->name(); }
    void prepare(Context* ctx) { _node->prepare(ctx); }

private:
    std::unique_ptr<ExprNode> _node;
    std::string _expr_name;
};

struct ExprBuild {
    static Value parser_constant(const Token& token);
    Expr build(Context* ctx, TokenStream& stream);

private:
    std::stack<std::unique_ptr<ExprNode>> stk;
    std::stack<Token> stk_op;
    std::unique_ptr<ExprNode> build_constant(Token token);
    std::unique_ptr<ExprNode> build_variable(Context* ctx, TokenStream& stream, Token token);
    std::unique_ptr<ExprNode> build_function(Context* ctx, TokenStream& stream, Token token);
    std::unique_ptr<ExprNode> build_subscript(Context* ctx, TokenStream& stream, Token token);

    std::unique_ptr<ExprNode> build_op(Token token);

    template <typename T>
    std::unique_ptr<ExprNode> build_binary_op();

    template <typename T>
    std::unique_ptr<ExprNode> build_unary_op();
};