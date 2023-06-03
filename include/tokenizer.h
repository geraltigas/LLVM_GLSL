//
// Created by jb030 on 12/05/2023.
//


#ifndef LLVM_TOKENIZER_H

#include "global.h"
#include <fstream>
#include <iostream>
#include <string>
#include <memory>

int gettok();
int advance();
int getNextToken();
void redirectInput(std::string filePath);
void rediectOutput(std::string filePath);
void Tokenize();
void printTokens();

struct Token {
    TokenType type;
    std::unique_ptr<std::string> value;
    Token(TokenType type, std::unique_ptr<std::string> value) : type(type), value(std::move(value)) {}
    Token(TokenType type,const char *value) : type(type), value(std::make_unique<std::string>(value)) {}
    Token(Token&& other) noexcept : type(other.type), value(std::move(other.value)) {}
    std::string toString();
};

#define LLVM_TOKENIZER_H

#endif // LLVM_TOKENIZER_H
