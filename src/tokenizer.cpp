//
// Created by jb030 on 12/05/2023.
//
#include "tokenizer.h"

extern int CurTok;
extern std::string IdentifierStr; // Filled in if tok_identifier
extern double NumVal;             // Filled in if tok_number

struct SourceLocation {
  int Line;
  int Col;
};

extern SourceLocation CurLoc;
extern SourceLocation LexLoc;

int advance() {
  int LastChar = getchar();

  if (LastChar == '\n' || LastChar == '\r') {
    LexLoc.Line++;
    LexLoc.Col = 0;
  } else
    LexLoc.Col++;
  return LastChar;
}

int gettok() {
  static int LastChar = ' ';

  // Skip any whitespace.
  while (isspace(LastChar))
    LastChar = advance();

  CurLoc = LexLoc;

  // keywords and identifiers
  if (isalpha(LastChar)) { // identifier: [a-zA-Z][a-zA-Z0-9]*
    IdentifierStr = LastChar;
    while (isalnum((LastChar = advance())))
      IdentifierStr += LastChar;

    if (IdentifierStr == "const")
      return tok_const;
    if (IdentifierStr == "if")
      return tok_if;
    if (IdentifierStr == "while")
      return tok_while;
    if (IdentifierStr == "do")
      return tok_do;
    if (IdentifierStr == "for")
      return tok_for;
    if (IdentifierStr == "break")
      return tok_break;
    if (IdentifierStr == "continue")
      return tok_continue;
    if (IdentifierStr == "return")
      return tok_return;
    if (IdentifierStr == "uniform")
      return tok_uniform;
    if (IdentifierStr == "in")
      return tok_in;
    if (IdentifierStr == "out")
      return tok_uniform_out;
    if (IdentifierStr == "layout")
      return tok_layout;
    if (IdentifierStr == "location")
      return tok_location;
    if (IdentifierStr == "binding")
      return tok_binding;
    if (IdentifierStr == "void")
      return tok_void;
    if (IdentifierStr == "int")
      return tok_int;
    if (IdentifierStr == "uint")
      return tok_uint;
    if (IdentifierStr == "bool")
      return tok_bool;
    if (IdentifierStr == "float")
      return tok_float;
    if (IdentifierStr == "double")
      return tok_double;
    if (IdentifierStr == "vec2")
      return tok_vec2;
    if (IdentifierStr == "vec3")
      return tok_vec3;
    if (IdentifierStr == "vec4")
      return tok_vec4;
    if (IdentifierStr == "dvec2")
      return tok_dvec2;
    if (IdentifierStr == "dvec3")
      return tok_dvec3;
    if (IdentifierStr == "dvec4")
      return tok_dvec4;
    if (IdentifierStr == "bvec2")
      return tok_bvec2;
    if (IdentifierStr == "bvec3")
      return tok_bvec3;
    if (IdentifierStr == "bvec4")
      return tok_bvec4;
    if (IdentifierStr == "ivec2")
      return tok_ivec2;
    if (IdentifierStr == "ivec3")
      return tok_ivec3;
    if (IdentifierStr == "ivec4")
      return tok_ivec4;
    if (IdentifierStr == "uvec2")
      return tok_uvec2;
    if (IdentifierStr == "uvec3")
      return tok_uvec3;
    if (IdentifierStr == "uvec4")
      return tok_uvec4;
    if (IdentifierStr == "mat2")
      return tok_mat2;
    if (IdentifierStr == "mat3")
      return tok_mat3;
    if (IdentifierStr == "mat4")
      return tok_mat4;
    return tok_identifier;
  }

  // numbers
  if (isdigit(LastChar) || LastChar == '.') { // Number: [0-9.]+
    std::string NumStr;
    do {
      NumStr += LastChar;
      LastChar = advance();
    } while (isdigit(LastChar) || LastChar == '.');

    NumVal = strtod(NumStr.c_str(), nullptr);
    return tok_number;
  }

  // version
  if (LastChar == '#') {
    // parsing #version
    char version[7];
    int i = 0;
    while (i < 7) {
      version[i] = LastChar;
      LastChar = advance();
      i++;
    }
    if (version[0] == '#' && version[1] == 'v' && version[2] == 'e' &&
        version[3] == 'r' && version[4] == 's' && version[5] == 'i' &&
        version[6] == 'o' && version[7] == 'n') {
      return tok_version;
    } else {
      return tok_eof;
    }
  }

  // Check for end of file.  Don't eat the EOF.
  if (LastChar == EOF)
    return tok_eof;

  // Otherwise, just return the character as its ascii value.
  int ThisChar = LastChar;
  LastChar = advance();
  return ThisChar;
}

int getNextToken() { return CurTok = gettok(); }

void redirectStream(std::string filePath) {
  //  freopen("filePath","r",stdin);
  freopen(filePath.c_str(), "r", stdin);
}
