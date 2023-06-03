//
// Created by jb030 on 13/05/2023.
//

#ifndef LLVM_LOG_H

#include <iostream>
#include "ast.h"

using namespace ast;
std::unique_ptr<ExpressionAST> error(const char *Str);
Value *errorV(const char *Str);
std::unique_ptr<FunctionPrototypeAST> errorP(const char *Str);

#define LLVM_LOG_H

#endif // LLVM_LOG_H
