//
// Created by jb030 on 13/05/2023.
//

#include "global.h"

std::map<char, int> BinopPrecedence;

void initBinopPrecedence(){
  BinopPrecedence['<'] = 10;
  BinopPrecedence['+'] = 20;
  BinopPrecedence['-'] = 20;
  BinopPrecedence['*'] = 40;
}

int CurTok;
std::string IdentifierStr; // Filled in if tok_identifier
double NumVal;             // Filled in if tok_number

struct SourceLocation {
  int Line;
  int Col;
};

SourceLocation CurLoc;
SourceLocation LexLoc = {1, 0};