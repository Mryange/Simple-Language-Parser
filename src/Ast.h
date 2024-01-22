#pragma once

#include "Context.h"
#include "ExprBuild.h"
#include "util.h"

struct AstNode {
    AstNode() {};
    // return true when using return
    [[nodiscard]] virtual bool exec(Context* ctx) { return true; }
    virtual ~AstNode() = default;
    virtual bool is_if() const { return false; }
    std::string ast_string() {
        std::stringstream out;

        debug_string(out, "");
        return out.str();
    }
    virtual void debug_string(std::stringstream& out, const std::string& prefix) {}

    std::string counter() const { return "[ " + std::to_string(_count) + " ]"; }

protected:
    size_t _count = 0;
};

struct ExprExecuteNode : AstNode {
    ExprExecuteNode(Expr& expr) : _expr(std::move(expr)) {}
    [[nodiscard]] bool exec(Context* ctx) override {
        _count++;
        _expr(ctx);
        return false;
    }
    void debug_string(std::stringstream& out, const std::string& prefix) override {
        out << prefix << counter() << _expr.expr_name() << "\n";
    }

private:
    Expr _expr;
};

struct ReturnNode : AstNode {
    ReturnNode(Expr& expr) : _ret_expr(std::move(expr)) {}
    [[nodiscard]] bool exec(Context* ctx) override {
        _count++;
        ctx->set_ret(_ret_expr(ctx));
        return true;
    }
    void debug_string(std::stringstream& out, const std::string& prefix) override {
        out << prefix << counter() << "return " << _ret_expr.expr_name() << "\n";
    }

private:
    Expr _ret_expr;
};

struct StmtNode : AstNode {
    [[nodiscard]] bool exec(Context* ctx) override {
        _count++;
        for (auto* node : _command) {
            RETURN_IF_TRUE(node->exec(ctx));
        }
        return false;
    }
    void add_command(AstNode* node) { _command.push_back(node); }
    AstNode* back_command() { return _command.back(); }
    void debug_string(std::stringstream& out, const std::string& prefix) override {
        for (auto* cmd : _command) {
            cmd->debug_string(out, prefix);
        }
    }

private:
    std::vector<AstNode*> _command;
};

struct IfNode : AstNode {
    IfNode(Expr& expr, StmtNode* if_stmt) : _expr(std::move(expr)), _if_stmt(if_stmt) {}

    bool is_if() const override { return true; }
    [[nodiscard]] bool exec(Context* ctx) override {
        _count++;
        auto expr_value = _expr(ctx);
        if (expr_value.get_bool()) {
            RETURN_IF_TRUE(_if_stmt->exec(ctx));
        } else {
            if (_else_stmt) {
                RETURN_IF_TRUE(_else_stmt->exec(ctx));
            }
        }
        return false;
    }

    void set_else(StmtNode* stmt) { _else_stmt = stmt; }

    void debug_string(std::stringstream& out, const std::string& prefix) override {
        out << prefix << counter() << "if " << _expr.expr_name() << "\n";
        _if_stmt->debug_string(out, prefix + "\t");
        if (_else_stmt) {
            out << prefix << counter() << "else"
                << "\n";
            _else_stmt->debug_string(out, prefix + "\t");
        }
    }

private:
    Expr _expr;
    StmtNode* _if_stmt = nullptr;
    StmtNode* _else_stmt = nullptr;
};

struct WhileNode : AstNode {
    WhileNode(Expr& expr, StmtNode* while_stmt) : _expr(std::move(expr)), _while_stmt(while_stmt) {}
    [[nodiscard]] bool exec(Context* ctx) override {
        while (_expr(ctx).get_bool()) {
            _count++;
            RETURN_IF_TRUE(_while_stmt->exec(ctx));
        }
        return false;
    }

    void debug_string(std::stringstream& out, const std::string& prefix) override {
        out << prefix << counter() << "while " << _expr.expr_name() << "\n";
        _while_stmt->debug_string(out, prefix + "\t");
    }

private:
    Expr _expr;
    StmtNode* _while_stmt;
};

struct ForNode : AstNode {
    ForNode(Expr& expr1, Expr& expr2, Expr& expr3, StmtNode* for_stmt)
            : _expr1(std::move(expr1)),
              _expr2(std::move(expr2)),
              _expr3(std::move(expr3)),
              _for_stmt(for_stmt) {}
    [[nodiscard]] bool exec(Context* ctx) override {
        for (_expr1(ctx); _expr2(ctx).get_bool(); _expr3(ctx)) {
            RETURN_IF_TRUE(_for_stmt->exec(ctx));
            _count++;
        }
        return false;
    }

    void debug_string(std::stringstream& out, const std::string& prefix) override {
        out << prefix << counter() << "for " << _expr1.expr_name() << " " << _expr2.expr_name()
            << " " << _expr3.expr_name() << "\n";
        _for_stmt->debug_string(out, prefix + "\t");
    }

private:
    Expr _expr1;
    Expr _expr2;
    Expr _expr3;
    StmtNode* _for_stmt;
};


struct ForeachNode : AstNode {
    ForeachNode(Expr& expr, const std::string var_name , StmtNode* for_stmt)
            : _expr(std::move(expr)),
            _var_name(var_name),
              _for_stmt(for_stmt) {}
    [[nodiscard]] bool exec(Context* ctx) override {
        auto arr = _expr(ctx);
        for(Value v :         arr.get_arr()){
             ctx->func()->variable_mgr()->set(_var_name, v);
             RETURN_IF_TRUE(_for_stmt->exec(ctx));
        }
        return false;
    }

    void debug_string(std::stringstream& out, const std::string& prefix) override {
        out << prefix << counter() << "for each " << _var_name << " : " << _expr.expr_name()
            << "\n";

        _for_stmt->debug_string(out, prefix + "\t");
    }

private:
    Expr _expr;
    std::string _var_name;
    StmtNode* _for_stmt;
};


struct FuncNode : AstNode {
    FuncNode(const std::string func_name) : _func_name(func_name) {}
    bool exec(Context* ctx) override {
        _count++;
        RETURN_IF_TRUE(_stmt->exec(ctx));
        return false;
    }
    void set_stmt(StmtNode* stmt) { _stmt = stmt; }

    void debug_string(std::stringstream& out, const std::string& prefix) override {
        out << prefix << counter() << "func " << _func_name << "\n";
        _stmt->debug_string(out, prefix + "\t");
    }

private:
    StmtNode* _stmt;
    std::string _func_name;
};

namespace BuildAst {
void buildAst(TokenStream& stream, Context* ctx);

void parser_func(TokenStream& stream, Context* ctx);
void parser_struct(TokenStream& stream, Context* ctx);

StmtNode* parser_stmt(TokenStream& stream, Context* ctx);

AstNode* parser_while(TokenStream& stream, Context* ctx);

IfNode* parser_if(TokenStream& stream, Context* ctx);

void parser_else(TokenStream& stream, Context* ctx, IfNode* if_node);

AstNode* parser_for(TokenStream& stream, Context* ctx);

AstNode* parser_foreach(TokenStream& stream, Context* ctx);

AstNode* parser_return(TokenStream& stream, Context* ctx);

} // namespace BuildAst
