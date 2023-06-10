//
// Created by jb030 on 12/05/2023.
//

#include "parser.h"
#include "tokenizer.h"
#include "generator.h"
#include "scope.h"

extern std::unique_ptr<TopLevelAST> topLevelAst;
extern std::set<std::shared_ptr<Scope>> scopeSet;

int main() {
  initBinopPrecedence();
  redirectInput(GLSL_FILE);
  rediectOutput(JSON_FILE);
  Tokenize();
  printTokens();
  InitializeModule();
  if (parseAST() < 0) {
    consolePrint("reject");
    return -1;
  } else {
    consolePrint("accept");
  }
  std::cout << topLevelAst->toString() << std::endl;
  topLevelAst->codegen();
  codeGen(IR_FILE);
  scopeSet.clear();
}
