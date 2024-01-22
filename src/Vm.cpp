#include "Vm.h"

#include "Function.h"



void VM::run() {
    // by gpt
    std::filesystem::path currentPath = std::filesystem::current_path();

    std::string fileExtension = ".ycc";

    auto _global_ctx = std::make_unique<GlobalContext>();
    Context ctx {_global_ctx.get()};


    for (const auto& entry : std::filesystem::directory_iterator(currentPath)) {

        if (entry.path().extension() == fileExtension) {

            std::ifstream file(entry.path());
            if (file.is_open()) {
                std::stringstream buffer;
                buffer << file.rdbuf();
                auto Text = buffer.str();
                build(Text,&ctx);
                file.close();
            } else {
                std::cerr << "Error opening file: " << entry.path() << std::endl;
            }
        }
    }


    ctx.global()->prepare_expr(&ctx);

    auto main_func = ctx.func_mgr()->get_func({"main", 0});

    auto ret = main_func->exce(&ctx, {});

    std::cout << "end and return : " << ret << "\n";

    // 输出全部经过优化后的代码，还没写完
    // std::cout << "tree : "
    //           << "\n";
    // for (auto* f : ctx.all_func_node()) {
    //     std::cout << f->ast_string() << "\n";
    // }
}



void VM::build(std::string& text , Context * ctx) {
    text += "                           ";
    auto tokenstream = Tokenizer::get_token_stream(text);
    BuildAst::buildAst(tokenstream, ctx);
}
