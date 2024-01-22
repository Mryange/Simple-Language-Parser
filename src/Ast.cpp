#include "Ast.h"

#include "Token.h"

namespace BuildAst {
const Token Def = Token("def", Token::Type::key_word);
const Token Struct = Token("struct", Token::key_word);

void buildAst(TokenStream& stream, Context* ctx) {
    while (!stream.end()) {
        if (stream.top() == Def) {
            parser_func(stream, ctx);
        } else if (stream.top() == Struct) {
            parser_struct(stream, ctx);
        }
    }
    CHECK(stream.end(), "has other token");
}

void parser_func(TokenStream& stream, Context* ctx) {
    stream.eat(Def); //eat def
    auto token = stream.eat(Token::Type::variable);
    std::string function_name = token.str();
    stream.eat("(");

    // func_name( arg , arg, arg ....)
    std::vector<std::string> args_name;
    while (stream.top().str() != ")") {
        auto arg = stream.eat(Token::Type::variable).str();
        args_name.push_back(arg);
        if (stream.top().str() != ")") {
            stream.eat(",");
        }
    }
    stream.eat(")");
    stream.eat("{");

    auto* func_node = ctx->obj_pool()->add(new FuncNode {function_name});
    ctx->func_mgr()->register_func(function_name, args_name, func_node);
    auto* stmt_node = parser_stmt(stream, ctx);
    func_node->set_stmt(stmt_node);
    ctx->add_func_node(func_node);
    stream.eat("}");
}

void parser_struct(TokenStream& stream, Context* ctx) {
    stream.eat(Struct);
    auto struct_name = stream.eat(Token::Type::variable).str();
    if (stream.top_equal("extends")) {
        stream.get();
        auto extends_strcut_name = stream.eat(Token::Type::variable).str();
        ctx->struct_info()->extends(struct_name, extends_strcut_name);
    }
    stream.eat("{");
    std::map<std::string, Value> struct_map;
    while (!stream.top_equal("}")) {
        auto member_name = stream.eat(Token::Type::variable).str();
        stream.eat("=");
        if (stream.top_equal(Token::constant)) {
            auto const_token = stream.get();
            struct_map[member_name] = ExprBuild::parser_constant(const_token);
        } else if (stream.top() == Struct) {
            stream.get();
            auto other_struct_name = stream.eat(Token::Type::variable).str();
            struct_map[member_name] =
                    ctx->struct_info()->get_default_struct_value(other_struct_name);
        }
        stream.eat(";");
    }
    ctx->struct_info()->put_struct_info(struct_name, struct_map);
    stream.eat("}");
}
StmtNode* parser_stmt(TokenStream& stream, Context* ctx) {
    auto* pool = ctx->obj_pool();
    auto* stmt_node = pool->add(new StmtNode {});
    ExprBuild builder;

    while (!stream.top_equal("}")) {
        if (stream.top().type() == Token::Type::key_word) {
            auto str = stream.top().str();
            if (str == "if") {
                stream.get(); // eat if
                stmt_node->add_command(parser_if(stream, ctx));
            } else if (str == "while") {
                stream.get(); // eat while
                stmt_node->add_command(parser_while(stream, ctx));
            } else if (str == "else") {
                stream.get(); // eat else
                CHECK(stmt_node->back_command()->is_if(), "else must be used after if.");
                auto* if_node = dynamic_cast<IfNode*>(stmt_node->back_command());
                parser_else(stream, ctx, if_node);
            } else if (str == "for") {
                stream.get(); // eat for
                stmt_node->add_command(parser_for(stream, ctx));
            } else if (str == "foreach") {
                stream.get(); // eat foreach
                stmt_node->add_command(parser_foreach(stream, ctx));
            } else if (str == "return") {
                stream.get(); // eat return
                stmt_node->add_command(parser_return(stream, ctx));
            }
        } else {
            auto expr = builder.build(ctx, stream);
            stmt_node->add_command(pool->add(new ExprExecuteNode(expr)));
            stream.eat(";");
        }
    }
    return stmt_node;
}

AstNode* parser_while(TokenStream& stream, Context* ctx) {
    ExprBuild builder;
    stream.eat("(");
    auto expr = builder.build(ctx, stream);
    stream.eat(")");
    stream.eat("{");
    auto* while_stmt = parser_stmt(stream, ctx);
    stream.eat("}");
    return ctx->obj_pool()->add(new WhileNode(expr, while_stmt));
}

IfNode* parser_if(TokenStream& stream, Context* ctx) {
    ExprBuild builder;
    stream.eat("(");
    auto expr = builder.build(ctx, stream);
    stream.eat(")");
    stream.eat("{");
    auto* if_stmt = parser_stmt(stream, ctx);
    stream.eat("}");
    return ctx->obj_pool()->add(new IfNode(expr, if_stmt));
}

void parser_else(TokenStream& stream, Context* ctx, IfNode* if_node) {
    stream.eat("{");
    auto* else_stmt = parser_stmt(stream, ctx);
    if_node->set_else(else_stmt);
    stream.eat("}");
}

AstNode* parser_for(TokenStream& stream, Context* ctx) {
    ExprBuild builder;
    stream.eat("(");
    auto expr1 = builder.build(ctx, stream);
    stream.eat(";");
    auto expr2 = builder.build(ctx, stream);
    stream.eat(";");
    auto expr3 = builder.build(ctx, stream);
    stream.eat(")");
    stream.eat("{");
    auto* for_stmt = parser_stmt(stream, ctx);
    stream.eat("}");
    return ctx->obj_pool()->add(new ForNode(expr1, expr2, expr3, for_stmt));
}

AstNode* parser_foreach(TokenStream& stream, Context* ctx) {
    ExprBuild builder;
    stream.eat("(");
    CHECK(stream.top_equal(Token::Type::variable), "foreach var : arr");
    auto name = stream.eat(Token::Type::variable).str();
    stream.eat(":");
    auto expr = builder.build(ctx, stream);
    stream.eat(")");
    stream.eat("{");
    auto* for_stmt = parser_stmt(stream, ctx);
    stream.eat("}");
    return ctx->obj_pool()->add(new ForeachNode(expr, name, for_stmt));
}

AstNode* parser_return(TokenStream& stream, Context* ctx) {
    ExprBuild builder;
    auto ret_expr = builder.build(ctx, stream);
    stream.eat(";");
    return ctx->obj_pool()->add(new ReturnNode(ret_expr));
}
} // namespace BuildAst