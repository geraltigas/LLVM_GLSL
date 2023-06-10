//
// Created by jb030 on 12/05/2023.
//

#ifndef LLVM_GLOBAL_H
#include <iostream>
#include <map>

#define GLSL_FILE                                                              \
  "/home/geraltigas/compilerCourse/llvm16/llvm/GLSL/test/temp.glsl"
#define JSON_FILE                                                              \
  "/home/geraltigas/compilerCourse/llvm16/llvm/GLSL/tempAst.json"
#define TOKENS_FILE                                                            \
  "/home/geraltigas/compilerCourse/llvm16/llvm/GLSL/tokens.txt"
#define IR_FILE "/home/geraltigas/compilerCourse/llvm16/llvm/GLSL/ir.ll"

#define DEBUG

void initBinopPrecedence();
std::string printTokens(size_t index);

extern uint64_t index_temp;

// define a macro to print log , line number, file name, function name and the
// toString() of the object, also print the index_temp also make sure the log is
// controlled by a maroc named DEBUG
#ifdef DEBUG
#define LOG(x)                                                                 \
  std::cout << "// " << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__     \
            << "(): now at " << std::to_string(index_temp) << " "              \
            << printTokens(index_temp) << " \n"                                \
            << (x != nullptr ? x->toString() : "NULL") << std::endl;
#else
#define LOG(x)
#endif

#ifdef DEBUG // only print expression using toString() when DEBUG is defined
#define LOG_EXPR(x)                                                            \
  std::cout << "// " << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__     \
            << " " << (x != nullptr ? x->toString() : "NULL") << std::endl;
#else
#define LOG_EXPR(x)
#endif

enum TokenType {
  tok_eof = -1, //
  // type
  tok_void = -27,   //
  tok_bool = -28,   //
  tok_int = -29,    //
  tok_uint = -30,   //
  tok_float = -31,  //
  tok_double = -32, //
  tok_vec2 = -33,   //
  tok_vec3 = -34,   //
  tok_vec4 = -35,   //
  tok_dvec2 = -36,  //
  tok_dvec3 = -37,  //
  tok_dvec4 = -38,  //
  tok_bvec2 = -39,  //
  tok_bvec3 = -40,  //
  tok_bvec4 = -41,  //
  tok_ivec2 = -42,  //
  tok_ivec3 = -43,  //
  tok_ivec4 = -44,  //
  tok_uvec2 = -45,  //
  tok_uvec3 = -46,  //
  tok_uvec4 = -47,  //
  tok_mat2 = -48,   //
  tok_mat3 = -49,   //
  tok_mat4 = -50,   //

  // layout
  tok_layout = -24,     //
  tok_uniform = -21,    //
  tok_layout_in = -22,  //
  tok_layout_out = -23, //
  tok_location = -25,   //
  tok_binding = -26,    //

  // primary
  tok_identifier = -4, //
  tok_number = -5,     //

  // control
  tok_if = -6,        //
  tok_else = -8,      //
  tok_for = -9,       //
  tok_while = -16,    //
  tok_do = -17,       //
  tok_break = -18,    //
  tok_continue = -19, //
  tok_return = -20,   //

  // operators
  // tok_binary = -11,
  tok_plus_p = -11,             //
  tok_minus_m = -52,            //
  tok_plus = -53,               //
  tok_minus = -54,              //
  tok_tilde = -55,              //
  tok_exclamation = -56,        //
  tok_times = -57,              //
  tok_divide = -58,             //
  tok_mod = -59,                //
  tok_left_paren = -60,         //
  tok_right_paren = -61,        //
  tok_left_bracket = -62,       //
  tok_right_bracket = -63,      //
  tok_left_brace = -64,         //
  tok_right_brace = -65,        //
  tok_less = -66,               //
  tok_greater = -67,            //
  tok_less_equal = -68,         //
  tok_greater_equal = -69,      //
  tok_equal = -70,              //
  tok_not_equal = -71,          //
  tok_and = -72,                //
  tok_or = -73,                 //
  tok_xor = -74,                //
  tok_bit_and = -75,            //
  tok_bit_or = -76,             //
  tok_bit_xor = -77,            //
  tok_left_shift = -78,         //
  tok_right_shift = -79,        //
  tok_plus_assign = -80,        //
  tok_minus_assign = -81,       //
  tok_times_assign = -82,       //
  tok_divide_assign = -83,      //
  tok_mod_assign = -84,         //
  tok_and_assign = -85,         //
  tok_or_assign = -86,          //
  tok_xor_assign = -87,         //
  tok_left_shift_assign = -88,  //
  tok_right_shift_assign = -89, //
  tok_bit_and_assign = -93,     //
  tok_bit_or_assign = -92,      //
  tok_assign = -90,             //
  tok_unary = -12,              //
  tok_dot = -91,                //

  // var definition
  tok_version = -14, //
  tok_const = -15,   //

  tok_unkown = -51,   //
  tok_semicolon = -7, //
  tok_comma = -10,    //
  tok_colon = -13,    //

};

bool isAstType(TokenType token);
bool isAssignment(TokenType token);
bool isRelational(TokenType token);
bool isEuqality(TokenType token);
bool isShift(TokenType token);
bool isAdditive(TokenType token);
bool isMultiplicative(TokenType token);
bool isPrefix(TokenType token);
bool isPostfix(TokenType token);

enum ExprType {
  assign_expr = -1,
  plus_assign_expr = -2,
  minus_assign_expr = -3,
  times_assign_expr = -4,
  divide_assign_expr = -5,
  mod_assign_expr = -6,
  and_assign_expr = -7,
  or_assign_expr = -8,
  left_shift_assign_expr = -9,
  right_shift_assign_expr = -10,
  bit_and_assign_expr = -11,
  bit_or_assign_expr = -12,
  sequence_expr = -13,
  or_expr = -14,
  xor_expr = -15,
  and_expr = -16,
  bit_or_expr = -17,
  bit_xor_expr = -18,
  bit_and_expr = -19,
  equal_expr = -20,
  not_equal_expr = -21,
  greater_expr = -22,
  less_expr = -23,
  greater_equal_expr = -24,
  less_equal_expr = -25,
  left_shift_expr = -26,
  right_shift_expr = -27,
  plus_expr = -28,
  minus_expr = -29,
  times_expr = -30,
  divide_expr = -31,
  mod_expr = -32,
  plus_p_expr = -33,
  minus_m_expr = -34,
  not_expr = -35,
  tilde_expr = -36,
  post_plus_p_expr = -37,
  post_minus_m_expr = -38,
  dot_expr = -39,
  unknown_expr = -100
};

std::string exprTypeToString(ExprType exprType);
ExprType tokenToExprType(TokenType token);

enum AstType {
  type_void = -1,
  type_bool = -2,
  type_int = -3, // default
  type_uint = -4,
  type_float = -5, // default
  type_double = -6,
  type_vec2 = -7,
  type_vec3 = -8,
  type_vec4 = -9,
  type_dvec2 = -10,
  type_dvec3 = -11,
  type_dvec4 = -12,
  type_bvec2 = -13,
  type_bvec3 = -14,
  type_bvec4 = -15,
  type_ivec2 = -16,
  type_ivec3 = -17,
  type_ivec4 = -18,
  type_uvec2 = -19,
  type_uvec3 = -20,
  type_uvec4 = -25,
  type_mat2 = -21,
  type_mat3 = -22,
  type_mat4 = -23,
  type_error = -24,
};

std::string astTypeToString(AstType astType);

// std::string getTokName(int Tok) {
//   switch (Tok) {
//   case tok_eof:
//     return "eof";
//   case tok_def:
//     return "def";
//   case tok_extern:
//     return "extern";
//   case tok_identifier:
//     return "identifier";
//   case tok_number:
//     return "number";
//   case tok_if:
//     return "if";
//   case tok_then:
//     return "then";
//   case tok_else:
//     return "else";
//   case tok_for:
//     return "for";
//   case tok_in:
//     return "in";
//   case tok_binary:
//     return "binary";
//   case tok_unary:
//     return "unary";
//   case tok_var:
//     return "var";
//   default:
//     break;
//   }
//   return std::string(1, (char)Tok);
// }

template <typename Base, typename T> inline bool instanceof (const T *ptr) {
  return dynamic_cast<const Base *>(ptr) != nullptr;
}

#define LLVM_GLOBAL_H

#endif // LLVM_GLOBAL_H
