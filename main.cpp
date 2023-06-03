//
// Created by jb030 on 12/05/2023.
//

#include "parser.h"
#include "tokenizer.h"

extern std::unique_ptr<TopLevelAST> topLevelAst;

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
  // LOG_EXPR(topLevelAst)
  // TheModule->print(errs(), nullptr);
}
