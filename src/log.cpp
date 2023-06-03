//
// Created by jb030 on 13/05/2023.
//

#include "log.h"
#include <iostream>
#include <memory>
#include "ast.h"

using namespace ast;

std::unique_ptr<ExpressionAST> error(const char *Str) {
    fprintf(stderr, "Error: %s\n", Str);
    return nullptr;
}

std::unique_ptr<FunctionPrototypeAST> errorP(const char *Str){
    error(Str);
    return nullptr;
};


Value *errorV(const char *Str) {
    error(Str);
    return nullptr;
}