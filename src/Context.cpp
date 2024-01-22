#include "Context.h"

#include "Function.h"

#include "ExprBuild.h"

GlobalContext::GlobalContext() {
    _function_mgr = std::make_unique<FunctionMgr>();
    _struct_mgr = std::make_unique<StructInfoMgr>();
}

void GlobalContext::prepare_expr(Context* ctx) {
    for (auto* expr : _exprs) {
        expr->prepare(ctx);
    }
}