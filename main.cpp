//
// Created by jb030 on 12/05/2023.
//

#include "parser.h"
#include "tokenizer.h"

int main() {
  initBinopPrecedence();
  redirectStream("/home/geraltigas/compilerCourse/llvm16/llvm/GLSL/temp.glsl");
  getNextToken();
  InitializeModule();
  MainLoop();
  TheModule->print(errs(), nullptr);
}
