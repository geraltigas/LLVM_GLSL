//
// Created by jb030 on 13/05/2023.
//

#include "global.h"
#include "tokenizer.h"
#include <vector>


std::map<char, int> BinopPrecedence;
extern std::vector<Token> tokens;

void initBinopPrecedence() {
  BinopPrecedence['<'] = 10;
  BinopPrecedence['+'] = 20;
  BinopPrecedence['-'] = 20;
  BinopPrecedence['*'] = 40;
}

int CurTok;
std::string IdentifierStr; // Filled in if tok_identifier
std::string NumVal;        // Filled in if tok_number

struct SourceLocation {
  int Line;
  int Col;
};

SourceLocation CurLoc;
SourceLocation LexLoc = {1, 0};

bool isAstType(TokenType token) {
  return token <= tok_void && token >= tok_mat4;
}

bool isAssignment(TokenType token) {
  return token == tok_assign || token == tok_plus_assign ||
         token == tok_minus_assign || token == tok_times_assign ||
         token == tok_divide_assign || token == tok_mod_assign ||
         token == tok_and_assign || token == tok_or_assign ||
         token == tok_left_shift_assign || token == tok_right_shift_assign ||
         token == tok_bit_or_assign || token == tok_bit_and_assign;
}

bool isRelational(TokenType token) {
  return token == tok_less || token == tok_less_equal || token == tok_greater ||
         token == tok_greater_equal;
}

bool isEuqality(TokenType token) {
  return token == tok_equal || token == tok_not_equal;
}

bool isShift(TokenType token) {
  return token == tok_left_shift || token == tok_right_shift;
}
bool isAdditive(TokenType token) {
  return token == tok_plus || token == tok_minus;
}
bool isMultiplicative(TokenType token) {
  return token == tok_times || token == tok_divide || token == tok_mod;
}
bool isPrefix(TokenType token) {
  return token == tok_plus_p || token == tok_minus_m ||
         token == tok_exclamation || token == tok_tilde;
}
bool isPostfix(TokenType token) {
  return token == tok_plus_p || token == tok_minus_m || token == tok_dot;
}

ExprType tokenToExprType(TokenType token) {
  switch (token) {
  case tok_assign:
    return assign_expr;
  case tok_plus_assign:
    return plus_assign_expr;
  case tok_minus_assign:
    return minus_assign_expr;
  case tok_times_assign:
    return times_assign_expr;
  case tok_divide_assign:
    return divide_assign_expr;
  case tok_mod_assign:
    return mod_assign_expr;
  case tok_and_assign:
    return and_assign_expr;
  case tok_or_assign:
    return or_assign_expr;
  case tok_left_shift_assign:
    return left_shift_assign_expr;
  case tok_right_shift_assign:
    return right_shift_assign_expr;
  case tok_bit_or_assign:
    return bit_or_assign_expr;
  case tok_bit_and_assign:
    return bit_and_assign_expr;
  case tok_equal:
    return equal_expr;
  case tok_not_equal:
    return not_equal_expr;
  case tok_less:
    return less_expr;
  case tok_less_equal:
    return less_equal_expr;
  case tok_greater:
    return greater_expr;
  case tok_greater_equal:
    return greater_equal_expr;
  case tok_left_shift:
    return left_shift_expr;
  case tok_right_shift:
    return right_shift_expr;
  case tok_plus:
    return plus_expr;
  case tok_minus:
    return minus_expr;
  case tok_times:
    return times_expr;
  case tok_divide:
    return divide_expr;
  case tok_mod:
    return mod_expr;
  case tok_plus_p:
    return plus_p_expr;
  case tok_minus_m:
    return minus_m_expr;
  case tok_exclamation:
    return not_expr;
  case tok_tilde:
    return tilde_expr;
  case tok_dot:
    return dot_expr;
  default:
    return unknown_expr;
  }
}
std::string exprTypeToString(ExprType exprType) {
  switch (exprType) {
  case assign_expr:
    return "assign_expr";
  case plus_assign_expr:
    return "plus_assign_expr";
  case minus_assign_expr:
    return "minus_assign_expr";
  case times_assign_expr:
    return "times_assign_expr";
  case divide_assign_expr:
    return "divide_assign_expr";
  case mod_assign_expr:
    return "mod_assign_expr";
  case and_assign_expr:
    return "and_assign_expr";
  case or_assign_expr:
    return "or_assign_expr";
  case left_shift_assign_expr:
    return "left_shift_assign_expr";
  case right_shift_assign_expr:
    return "right_shift_assign_expr";
  case bit_or_assign_expr:
    return "bit_or_assign_expr";
  case bit_and_assign_expr:
    return "bit_and_assign_expr";
  case equal_expr:
    return "equal_expr";
  case not_equal_expr:
    return "not_equal_expr";
  case less_expr:
    return "less_expr";
  case less_equal_expr:
    return "less_equal_expr";
  case greater_expr:
    return "greater_expr";
  case greater_equal_expr:
    return "greater_equal_expr";
  case left_shift_expr:
    return "left_shift_expr";
  case right_shift_expr:
    return "right_shift_expr";
  case plus_expr:
    return "plus_expr";
  case minus_expr:
    return "minus_expr";
  case times_expr:
    return "times_expr";
  case divide_expr:
    return "divide_expr";
  case mod_expr:
    return "mod_expr";
  case plus_p_expr:
    return "plus_p_expr";
  case minus_m_expr:
    return "minus_m_expr";
  case not_expr:
    return "not_expr";
  case tilde_expr:
    return "tilde_expr";
  case dot_expr:
    return "dot_expr";
  case sequence_expr:
    return "sequence_expr";
  default:
    return "none_expr";
  }
}
std::string astTypeToString(AstType astType) {
  switch (astType) {
  case type_void:
    return "void";
  case type_bool:
    return "bool";
  case type_int:
    return "int";
  case type_uint:
    return "uint";
  case type_float:
    return "float";
  case type_double:
    return "double";
  case type_vec2:
    return "vec2";
  case type_vec3:
    return "vec3";
  case type_vec4:
    return "vec4";
  case type_dvec2:
    return "dvec2";
  case type_dvec3:
    return "dvec3";
  case type_dvec4:
    return "dvec4";
  case type_bvec2:
    return "bvec2";
  case type_bvec3:
    return "bvec3";
  case type_bvec4:
    return "bvec4";
  case type_ivec2:
    return "ivec2";
  case type_ivec3:
    return "ivec3";
  case type_ivec4:
    return "ivec4";
  case type_uvec2:
    return "uvec2";
  case type_uvec3:
    return "uvec3";
  case type_uvec4:
    return "uvec4";
  case type_mat2:
    return "mat2";
  case type_mat3:
    return "mat3";
  case type_mat4:
    return "mat4";
  case type_error:
    return "error";
  default:
    return "unkown";
  }
}
std::string printTokens(size_t index) {
  std::string str;
  for (size_t i = index-5; i < index+5 && i < tokens.size(); i++) {
    str += " " + *tokens[i].value;
  }
  return str;
}

std::vector<char> buffer;
