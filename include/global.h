//
// Created by jb030 on 12/05/2023.
//

#ifndef LLVM_GLOBAL_H
#include <iostream>
#include <map>

void initBinopPrecedence();

enum Token {
  tok_eof = -1,

  // type
  tok_void = -27,
  tok_bool = -28,
  tok_int = -29,
  tok_uint = -30,
  tok_float = -31,
  tok_double = -32,
  tok_vec2 = -33,
  tok_vec3 = -34,
  tok_vec4 = -35,
  tok_dvec2 = -36,
  tok_dvec3 = -37,
  tok_dvec4 = -38,
  tok_bvec2 = -39,
  tok_bvec3 = -40,
  tok_bvec4 = -41,
  tok_ivec2 = -42,
  tok_ivec3 = -43,
  tok_ivec4 = -44,
  tok_uvec2 = -45,
  tok_uvec3 = -46,
  tok_uvec4 = -47,
  tok_mat2 = -48,
  tok_mat3 = -49,
  tok_mat4 = -50,

  // layout
  tok_layout = -24,
  tok_uniform = -21,
  tok_uniform_in = -22,
  tok_uniform_out = -23,
  tok_location = -25,
  tok_binding = -26,

  // commands
  tok_def = -2,
  tok_extern = -3,

  // primary
  tok_identifier = -4,
  tok_number = -5,

  // control
  tok_if = -6,
  tok_then = -7,
  tok_else = -8,
  tok_for = -9,
  tok_in = -10,
  tok_while = -16,
  tok_do = -17,
  tok_break = -18,
  tok_continue = -19,
  tok_return = -20,

  // operators
  tok_binary = -11,
  tok_unary = -12,

  // var definition
  tok_var = -13,
  tok_version = -14,
  tok_const = -15,
};

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

#define LLVM_GLOBAL_H

#endif // LLVM_GLOBAL_H
