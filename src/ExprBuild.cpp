#include "ExprBuild.h"

#include "Context.h"
#include "Function.h"

Expr ExprBuild::build(Context* ctx, TokenStream& stream) {
    int left_bracket = 0;

    CHECK(stk.empty(), "stk must be empty()");
    CHECK(stk_op.empty(), "stk_op must be empty()");
    auto token_pos_begin = stream.cur_pos();
    while (!stream.end()) {
        auto token = stream.top();
        if (token.type() == Token::Type::constant) {
            stream.get();
            stk.push(std::move(build_constant(token)));
        } else if (token.type() == Token::variable) {
            stream.get();
            stk.push(std::move(build_variable(ctx, stream, token)));
        } else if (token.type() == Token::Type::op) {
            stream.get();
            if (stk_op.empty()) {
                stk_op.push(token);
            } else {
                while (!stk_op.empty() && stk_op.top() != Token {"(", Token::Type::bracket} &&
                       stk_op.top().priority() >= token.priority()) {
                    stk.push(std::move(build_op(stk_op.top())));
                    stk_op.pop();
                }
                stk_op.push(token);
            }
        } else if (token == Token {"(", Token::Type::bracket}) {
            stream.get();
            stk_op.push(token);
            left_bracket++;
        } else if (token == Token {")", Token::Type::bracket} && left_bracket) {
            stream.get();
            while (!stk_op.empty() && stk_op.top() != Token {"(", Token::Type::bracket}) {
                stk.push(std::move(build_op(stk_op.top())));
                stk_op.pop();
            }
            CHECK((!stk_op.empty() && stk_op.top() == Token {"(", Token::Type::bracket}),
                  ") match fail stk size " + std::to_string(stk.size()));
            stk_op.pop();
            left_bracket--;
        } else if (token == Token {"[", Token::Type::bracket}) {
            stream.get();
            stk.push(std::move(build_subscript(ctx, stream, token)));
        } else {
            break;
        }
    }
    CHECK(!stk.empty(), "!stk.empty()");
    while (!stk_op.empty()) {
        stk.push(std::move(build_op(stk_op.top())));
        stk_op.pop();
    }
    CHECK(stk.size() == 1, "stk.size() == 1");
    auto expr_name = stream.get_token_string(token_pos_begin, stream.cur_pos());
    Expr expr {std::move(stk.top())};
    stk.pop();
    return expr;
}

Value ExprBuild::parser_constant(const Token& token) {
    auto str = token.str();
    if (str.front() == '\"') {
        return Value {str.substr(1, str.size() - 1)};
    } else {
        if (auto v = tryParseBool(str)) {
            return Value {*v};
        } else if (auto v = tryParseInt(str)) {
            return Value {*v};
        } else if (auto v = tryParseFloat(str)) {
            return Value {*v};
        } else {
            return Value::default_value;
        }
    }
}

std::unique_ptr<ExprNode> ExprBuild::build_constant(Token token) {
    auto str = token.str();
    return ConstantValueNode::create_unique(parser_constant(token));
}

template <typename T>
std::unique_ptr<ExprNode> ExprBuild::build_binary_op() {
    CHECK(stk.size() >= 2, "binary op failed");
    std::unique_ptr<ExprNode> rhs = std::move(stk.top());
    stk.pop();
    std::unique_ptr<ExprNode> lhs = std::move(stk.top());
    stk.pop();
    auto op = T::create_unique();
    op->add_child(std::move(lhs));
    op->add_child(std::move(rhs));
    return op;
}

template <typename T>
std::unique_ptr<ExprNode> ExprBuild::build_unary_op() {
    CHECK(stk.size() >= 1, "unary op failed");
    std::unique_ptr<ExprNode> child = std::move(stk.top());
    stk.pop();
    auto op = T::create_unique();
    op->add_child(std::move(child));
    return op;
}

std::unique_ptr<ExprNode> ExprBuild::build_op(Token token) {
    auto s = token.str();
    if (s == "+") {
        return build_binary_op<AddOperator>();
    }
    if (s == "*") {
        return build_binary_op<MulOperator>();
    }
    if (s == "-") {
        return build_binary_op<SubOperator>();
    }
    if (s == "==") {
        return build_binary_op<EqualOperator>();
    }
    if (s == "!=") {
        return build_binary_op<NotEqualOperator>();
    }
    if (s == "=") {
        return build_binary_op<AssignOperator>();
    }
    if (s == "<=") {
        return build_binary_op<LessOrEqualOperator>();
    }
    if (s == ">=") {
        return build_binary_op<GreaterOrEqualOperator>();
    }
    if (s == "<") {
        return build_binary_op<LessOperator>();
    }
    if (s == ">") {
        return build_binary_op<GreaterOperator>();
    }
    if (s == "&&") {
        return build_binary_op<AndOperator>();
    }
    if (s == "||") {
        return build_binary_op<OrOperator>();
    }

    if (s == ".") {
        return build_binary_op<MemberAccessOperator>();
    }

    if (s == "&") {
        return build_unary_op<RefOperator>();
    }

    if (s == "arr") {
        return build_unary_op<ArrOperator>();
    }

    if (s == "let") {
        return build_unary_op<StructOperator>();
    }

    return nullptr;
}

std::unique_ptr<ExprNode> ExprBuild::build_variable(Context* ctx, TokenStream& stream,
                                                    Token token) {
    const bool is_func = stream.top_equal("(");
    if (is_func) {
        return build_function(ctx, stream, token);
    }
    return VariableNode::create_unique(token.str());
}

std::unique_ptr<ExprNode> ExprBuild::build_function(Context* ctx, TokenStream& stream,
                                                    Token token) {
    const auto& func_name = token.str();
    ExprBuild builder;
    std::vector<Expr> params;
    auto args = 0;
    stream.eat("(");
    while (!stream.top_equal(")")) {
        auto expr = builder.build(ctx, stream);
        if (!stream.top_equal(")")) stream.eat(",");
        params.push_back(std::move(expr));
        args++;
    }
    stream.eat(")");
    auto func_call = FuncCallOperator::create_unique(FunctionSignature {func_name, args});
    for (auto& expr : params) func_call->add_child(std::move(expr.node()));
    return func_call;
}

std::unique_ptr<ExprNode> ExprBuild::build_subscript(Context* ctx, TokenStream& stream,
                                                     Token token) {
    while (!stk_op.empty() && stk_op.top() != Token {"(", Token::Type::bracket} &&
           stk_op.top().priority() >= token.priority()) {
        stk.push(std::move(build_op(stk_op.top())));
        stk_op.pop();
    }
    auto Lchild = std::move(stk.top());
    stk.pop();
    ExprBuild builder;
    auto Rchild = builder.build(ctx, stream);
    auto subscript_op = SubscriptOperator::create_unique();
    subscript_op->add_child(std::move(Lchild));
    subscript_op->add_child(std::move(Rchild.node()));
    stream.eat("]");
    return subscript_op;
}
