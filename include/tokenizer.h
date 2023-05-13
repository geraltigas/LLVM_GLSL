//
// Created by jb030 on 12/05/2023.
//


#ifndef LLVM_TOKENIZER_H

#include "global.h"
#include <fstream>
#include <iostream>
#include <string>

int gettok();
int advance();
int getNextToken();
void redirectStream(std::string filePath);

#define LLVM_TOKENIZER_H

#endif // LLVM_TOKENIZER_H
