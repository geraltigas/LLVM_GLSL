//
// Created by jb030 on 12/05/2023.
//

#include "parser.h"
#include "tokenizer.h"

extern std::unique_ptr<TopLevelAST> topLevelAst;

int main() {
  initBinopPrecedence();
  redirectInput("/home/geraltigas/compilerCourse/llvm16/llvm/GLSL/temp.glsl");
  rediectOutput("/home/geraltigas/compilerCourse/llvm16/llvm/GLSL/tempAst.json");
  Tokenize();
  //printTokens();
  InitializeModule();
  parseAST();
  std::cout << topLevelAst->toString() << std::endl;
  //LOG_EXPR(topLevelAst)
  //TheModule->print(errs(), nullptr);
}
