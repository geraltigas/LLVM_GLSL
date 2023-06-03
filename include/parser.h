//
// Created by jb030 on 12/05/2023.
//

#ifndef LLVM_PARSER_H

#include <memory>
#include <map>
#include <ast.h>
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

using namespace llvm;
using namespace ast;

void InitializeModule();

static std::unique_ptr<LLVMContext> TheContext;
static std::unique_ptr<Module> TheModule;
static std::unique_ptr<IRBuilder<>> Builder;

std::unique_ptr<ExpressionAST> ParseExpression();
std::unique_ptr<NumberExprAST> ParseNumberExpr();
std::unique_ptr<SentencesAST> ParseSentences();
std::unique_ptr<VariableDefinitionAST> ParseVariableDefinition();
std::unique_ptr<GlobalVariableDefinitionAST> ParseGlobalVariableDefinition();
std::unique_ptr<FunctionDefinitionAST> ParseFunctionDefinition();

int parseAST();

#define LLVM_PARSER_H

#endif // LLVM_PARSER_H
