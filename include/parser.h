//
// Created by jb030 on 12/05/2023.
//

#ifndef LLVM_PARSER_H

#include <memory>
#include <map>
#include <ast.h>

using namespace llvm;
using namespace ast;

void InitializeModule();

std::unique_ptr<ExpressionAST> ParseExpression();
std::unique_ptr<NumberExprAST> ParseNumberExpr();
std::unique_ptr<SentencesAST> ParseSentences();
std::unique_ptr<VariableDefinitionAST> ParseVariableDefinition();
std::unique_ptr<GlobalVariableDefinitionAST> ParseGlobalVariableDefinition();
std::unique_ptr<FunctionDefinitionAST> ParseFunctionDefinition();

int parseAST();

#define LLVM_PARSER_H

#endif // LLVM_PARSER_H
