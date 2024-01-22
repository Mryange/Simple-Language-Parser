#include "Context.h"

#include "Ast.h"
#include "ExprBuild.h"
#include "Function.h"

GlobalContext::GlobalContext() {
    _function_mgr = std::make_unique<FunctionMgr>();
    _struct_mgr = std::make_unique<StructInfoMgr>();
}

void GlobalContext::prepare(Context* ctx) {
    for (auto* func : _func_nodes) {
        func->prepare(ctx);
    }
}