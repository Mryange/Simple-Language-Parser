
#pragma once
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <string>

#include "Context.h"
#include "ExprBuild.h"
#include "Token.h"
#include "Type.h"
#include "Ast.h"
#include "util.h"
struct VM {
    void build(std::string& text , Context * ctx);
    void run();
    std::unique_ptr<GlobalContext> _global_ctx;
};