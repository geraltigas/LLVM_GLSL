//
// Created by jb030 on 12/05/2023.
//
#include "tokenizer.h"
#include <vector>

extern char CurTok;
extern std::string IdentifierStr; // Filled in if tok_identifier
extern std::string NumVal;        // Filled in if tok_number

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
  static char LastChar = ' ';

  // Skip any whitespace.
  while (isspace(LastChar))
    LastChar = advance();

  CurLoc = LexLoc;

  // keywords and identifiers
  if (isalpha(LastChar)) { // identifier: [a-zA-Z_][a-zA-Z0-9_]*
    IdentifierStr = LastChar;
    while (isalnum((LastChar = advance())) || LastChar == '_')
      IdentifierStr += LastChar;

    if (IdentifierStr == "const")
      return tok_const;
    if (IdentifierStr == "if")
      return tok_if;
    if (IdentifierStr == "else")
      return tok_else;
    if (IdentifierStr == "for")
      return tok_for;
    if (IdentifierStr == "return")
      return tok_return;
    if (IdentifierStr == "uniform")
      return tok_uniform;
    if (IdentifierStr == "in")
      return tok_layout_in;
    if (IdentifierStr == "out")
      return tok_layout_out;
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
    if (IdentifierStr == "mat2")
      return tok_mat2;
    if (IdentifierStr == "mat3")
      return tok_mat3;
    if (IdentifierStr == "mat4")
      return tok_mat4;
    if (IdentifierStr == "layout")
      return tok_layout;
    if (IdentifierStr == "in")
      return tok_layout_in;
    if (IdentifierStr == "out")
      return tok_layout_out;
    if (IdentifierStr == "location")
      return tok_location;
    if (IdentifierStr == "binding")
      return tok_binding;
    return tok_identifier;
  }

  // numbers
  if (isdigit(LastChar)) { // Number: [0-9.]+
    std::string NumStr;
    do {
      NumStr += std::string(1, LastChar);
      if (LastChar == 'f') {
        LastChar = advance();
        NumVal = NumStr;
        return tok_number;
      }
      LastChar = advance();
    } while (isdigit(LastChar) || LastChar == '.' || LastChar == 'f');
    // check is a number
    if (NumStr.find_first_not_of("0123456789.") != std::string::npos) {
      return tok_eof;
    }

    NumVal = NumStr;
    return tok_number;
  }

  if (LastChar == '.') {
    LastChar = advance();
    return tok_dot;
  }

  // version
  if (LastChar == '#') {
    // parsing #version
    char version[8];
    int i = 0;
    while (i < 8) {
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

  if (LastChar == '(') {
    LastChar = advance();
    return tok_left_paren;
  }

  if (LastChar == ')') {
    LastChar = advance();
    return tok_right_paren;
  }

  if (LastChar == '{') {
    LastChar = advance();
    return tok_left_brace;
  }

  if (LastChar == '}') {
    LastChar = advance();
    return tok_right_brace;
  }

  if (LastChar == '[') {
    LastChar = advance();
    return tok_left_bracket;
  }

  if (LastChar == ']') {
    LastChar = advance();
    return tok_right_bracket;
  }

  if (LastChar == ';') {
    LastChar = advance();
    return tok_semicolon;
  }

  if (LastChar == ',') {
    LastChar = advance();
    return tok_comma;
  }

  if (LastChar == '+') {
    LastChar = advance();
    if (LastChar == '+') {
      LastChar = advance();
      return tok_plus_p;
    }
    if (LastChar == '=') {
      LastChar = advance();
      return tok_plus_assign;
    }
    return tok_plus;
  }

  if (LastChar == '-') {
    LastChar = advance();
    if (LastChar == '-') {
      LastChar = advance();
      return tok_minus_m;
    }
    if (LastChar == '=') {
      LastChar = advance();
      return tok_minus_assign;
    }
    return tok_minus;
  }

  if (LastChar == '*') {
    LastChar = advance();
    if (LastChar == '=') {
      LastChar = advance();
      return tok_times_assign;
    }
    return tok_times;
  }

  if (LastChar == '/') {
    LastChar = advance();
    if (LastChar == '=') {
      LastChar = advance();
      return tok_divide_assign;
    }
    return tok_divide;
  }

  if (LastChar == '%') {
    LastChar = advance();
    if (LastChar == '=') {
      LastChar = advance();
      return tok_mod_assign;
    }
    return tok_mod;
  }

  if (LastChar == '^') {
    LastChar = advance();
    if (LastChar == '^') {
      LastChar = advance();
      return tok_xor;
    }
    if (LastChar == '=') {
      LastChar = advance();
      return tok_xor_assign;
    }
    return tok_bit_xor;
  }

  if (LastChar == '=') {
    LastChar = advance();
    if (LastChar == '=') {
      LastChar = advance();
      return tok_equal;
    }
    return tok_assign;
  }

  if (LastChar == '!') {
    LastChar = advance();
    if (LastChar == '=') {
      LastChar = advance();
      return tok_not_equal;
    }
    return tok_exclamation;
  }

  if (LastChar == '<') {
    LastChar = advance();
    if (LastChar == '=') {
      LastChar = advance();
      return tok_less_equal;
    }
    if (LastChar == '<') {
      LastChar = advance();
      if (LastChar == '=') {
        LastChar = advance();
        return tok_left_shift_assign;
      }
      return tok_left_shift;
    }
    return tok_less;
  }

  if (LastChar == '>') {
    LastChar = advance();
    if (LastChar == '=') {
      LastChar = advance();
      return tok_greater_equal;
    }
    if (LastChar == '>') {
      LastChar = advance();
      if (LastChar == '=') {
        LastChar = advance();
        return tok_right_shift_assign;
      }
      return tok_right_shift;
    }
    return tok_greater;
  }

  if (LastChar == '&') {
    LastChar = advance();
    if (LastChar == '&') {
      LastChar = advance();
      return tok_and;
    }
    if (LastChar == '=') {
      LastChar = advance();
      return tok_and_assign;
    }
    return tok_bit_and;
  }

  if (LastChar == '|') {
    LastChar = advance();
    if (LastChar == '|') {
      LastChar = advance();
      return tok_or;
    }
    if (LastChar == '=') {
      LastChar = advance();
      return tok_or_assign;
    }
    return tok_bit_or;
  }

  if (LastChar == '~') {
    LastChar = advance();
    return tok_tilde;
  }

  if (LastChar == '?') {
    LastChar = advance();
    return tok_unary;
  }

  if (LastChar == ':') {
    LastChar = advance();
    return tok_colon;
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

void redirectInput(std::string filePath) {
  //  freopen("filePath","r",stdin);
  freopen(filePath.c_str(), "r", stdin);
}

void redirectOutput(std::string filePath) {
  // clear the file
  freopen(filePath.c_str(), "w", stdout);
}

void rediectOutput(std::string filePath) {
  // clear the file
  freopen(filePath.c_str(), "w", stdout);
}

std::vector<Token> tokens;

void Tokenize() {
  while ((CurTok = getNextToken())) {
    switch (CurTok) {
    case tok_layout:
      tokens.emplace_back(tok_layout, "layout");
      break;
    case tok_uniform:
      tokens.emplace_back(tok_uniform, "uniform");
      break;
    case tok_layout_in:
      tokens.emplace_back(tok_layout_in, "in");
      break;
    case tok_layout_out:
      tokens.emplace_back(tok_layout_out, "out");
      break;
    case tok_location:
      tokens.emplace_back(tok_location, "location");
      break;
    case tok_binding:
      tokens.emplace_back(tok_binding, "binding");
      break;
    case tok_version:
      tokens.emplace_back(tok_version, "#version");
      break;
    case tok_const:
      tokens.emplace_back(tok_const, "const");
      break;
    case tok_if:
      tokens.emplace_back(tok_if, "if");
      break;
    case tok_for:
      tokens.emplace_back(tok_for, "for");
      break;
    case tok_return:
      tokens.emplace_back(tok_return, "return");
      break;
    case tok_else:
      tokens.emplace_back(tok_else, "else");
      break;
    case tok_identifier:
      tokens.emplace_back(tok_identifier, IdentifierStr.c_str());
      break;
    case tok_number:
      tokens.emplace_back(tok_number, NumVal.c_str());
      break;
    case tok_void:
      tokens.emplace_back(tok_void, "void");
      break;
    case tok_int:
      tokens.emplace_back(tok_int, "int");
      break;
    case tok_bool:
      tokens.emplace_back(tok_bool, "bool");
      break;
    case tok_float:
      tokens.emplace_back(tok_float, "float");
      break;
    case tok_double:
      tokens.emplace_back(tok_double, "double");
      break;
    case tok_vec2:
      tokens.emplace_back(tok_vec2, "vec2");
      break;
    case tok_vec3:
      tokens.emplace_back(tok_vec3, "vec3");
      break;
    case tok_vec4:
      tokens.emplace_back(tok_vec4, "vec4");
      break;
    case tok_mat2:
      tokens.emplace_back(tok_mat2, "mat2");
      break;
    case tok_mat3:
      tokens.emplace_back(tok_mat3, "mat3");
      break;
    case tok_mat4:
      tokens.emplace_back(tok_mat4, "mat4");
      break;
      // binary operator
    case tok_plus:
      tokens.emplace_back(tok_plus, "+");
      break;
    case tok_minus:
      tokens.emplace_back(tok_minus, "-");
      break;
    case tok_times:
      tokens.emplace_back(tok_times, "*");
      break;
    case tok_divide:
      tokens.emplace_back(tok_divide, "/");
      break;
    case tok_mod:
      tokens.emplace_back(tok_mod, "%");
      break;
    case tok_assign:
      tokens.emplace_back(tok_assign, "=");
      break;
    case tok_equal:
      tokens.emplace_back(tok_equal, "==");
      break;
    case tok_not_equal:
      tokens.emplace_back(tok_not_equal, "!=");
      break;
    case tok_less:
      tokens.emplace_back(tok_less, "<");
      break;
    case tok_less_equal:
      tokens.emplace_back(tok_less_equal, "<=");
      break;
    case tok_greater:
      tokens.emplace_back(tok_greater, ">");
      break;
    case tok_greater_equal:
      tokens.emplace_back(tok_greater_equal, ">=");
      break;
    case tok_and:
      tokens.emplace_back(tok_and, "&&");
      break;
    case tok_or:
      tokens.emplace_back(tok_or, "||");
      break;
    case tok_xor:
      tokens.emplace_back(tok_xor, "^^");
      break;
    case tok_left_paren:
      tokens.emplace_back(tok_left_paren, "(");
      break;
    case tok_right_paren:
      tokens.emplace_back(tok_right_paren, ")");
      break;
    case tok_left_brace:
      tokens.emplace_back(tok_left_brace, "{");
      break;
    case tok_right_brace:
      tokens.emplace_back(tok_right_brace, "}");
      break;
    case tok_left_bracket:
      tokens.emplace_back(tok_left_bracket, "[");
      break;
    case tok_right_bracket:
      tokens.emplace_back(tok_right_bracket, "]");
      break;
    case tok_tilde:
      tokens.emplace_back(tok_tilde, "~");
      break;
    case tok_exclamation:
      tokens.emplace_back(tok_exclamation, "!");
      break;
    case tok_plus_p:
      tokens.emplace_back(tok_plus_p, "++");
      break;
    case tok_minus_m:
      tokens.emplace_back(tok_minus_m, "--");
      break;
    case tok_bit_and:
      tokens.emplace_back(tok_bit_and, "&");
      break;
    case tok_bit_or:
      tokens.emplace_back(tok_bit_or, "|");
      break;
    case tok_left_shift:
      tokens.emplace_back(tok_left_shift, "<<");
      break;
    case tok_right_shift:
      tokens.emplace_back(tok_right_shift, ">>");
      break;
    case tok_plus_assign:
      tokens.emplace_back(tok_plus_assign, "+=");
      break;
    case tok_minus_assign:
      tokens.emplace_back(tok_minus_assign, "-=");
      break;
    case tok_times_assign:
      tokens.emplace_back(tok_times_assign, "*=");
      break;
    case tok_divide_assign:
      tokens.emplace_back(tok_divide_assign, "/=");
      break;
    case tok_unary:
      tokens.emplace_back(tok_unary, "?");
      break;
    case tok_mod_assign:
      tokens.emplace_back(tok_mod_assign, "%=");
      break;
    case tok_and_assign:
      tokens.emplace_back(tok_and_assign, "&=");
      break;
    case tok_or_assign:
      tokens.emplace_back(tok_or_assign, "|=");
      break;
    case tok_xor_assign:
      tokens.emplace_back(tok_xor_assign, "^=");
      break;
    case tok_left_shift_assign:
      tokens.emplace_back(tok_left_shift_assign, "<<=");
      break;
    case tok_right_shift_assign:
      tokens.emplace_back(tok_right_shift_assign, ">>=");
      break;
    case tok_semicolon:
      tokens.emplace_back(tok_semicolon, ";");
      break;
    case tok_comma:
      tokens.emplace_back(tok_comma, ",");
      break;
    case tok_colon:
      tokens.emplace_back(tok_colon, ":");
      break;
    case tok_dot:
      tokens.emplace_back(tok_dot, ".");
      break;
    case tok_eof:
      tokens.emplace_back(tok_eof, "EOF");
      return;
    case tok_unkown:
      // change char CurTok into string
      tokens.emplace_back(tok_unkown, std::string(1, CurTok).c_str());
      return;
    }
  }
}

std::string Token::toString() {
  return std::string(std::string(this->value->c_str()));
}

void printTokens() {
  rediectOutput(TOKENS_FILE);
  for (auto &token : tokens) {
    std::cout << token.toString() << std::endl;
  }
  rediectOutput(JSON_FILE);
}
void consolePrint(std::string str) {
  // use std::cerr to print to console
  std::cerr << str << std::endl;
}
