//
// Created by jb030 on 04/06/2023.
//

#include "generator.h"
#include "ast.h"
#include "scope.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

#include <fstream>
#include <memory>

using namespace llvm;
using namespace ast;

std::unique_ptr<LLVMContext> TheContext;
std::unique_ptr<Module> TheModule;
std::unique_ptr<IRBuilder<>> Builder;
std::shared_ptr<Scope> topScope = std::make_shared<Scope>();
std::set<std::shared_ptr<Scope>> scopeSet = {topScope};
std::shared_ptr<Scope> currentScope = topScope;

void codeGen(const char *filename) {
  std::string filenameStr(filename);
  std::string code;
  raw_string_ostream os(code);
  TheModule->print(os, nullptr);
  os.flush();
  // override file
  std::ofstream out(filenameStr, std::ios::out | std::ios::trunc);
  out << code;
  out.close();
}

Type *getTypeFromAstType(AstType type) {
  switch (type) {
  case type_int:
    return Type::getInt32Ty(*TheContext);
  case type_bool:
    return Type::getInt1Ty(*TheContext);
  case type_uint:
    return Type::getInt32Ty(*TheContext);
  case type_float:
    return Type::getFloatTy(*TheContext);
  case type_double:
    return Type::getDoubleTy(*TheContext);
  case type_vec2:
    return VectorType::get(Type::getFloatTy(*TheContext), 2, false);
  case type_vec3:
    return VectorType::get(Type::getFloatTy(*TheContext), 3, false);
  case type_vec4:
    return VectorType::get(Type::getFloatTy(*TheContext), 4, false);
  case type_mat2:
    return VectorType::get(Type::getFloatTy(*TheContext), 4, false);
  case type_mat3:
    return VectorType::get(Type::getFloatTy(*TheContext), 9, false);
  case type_mat4:
    return VectorType::get(Type::getFloatTy(*TheContext), 16, false);
  case type_void:
    return Type::getVoidTy(*TheContext);
  default:
    printf("Error: unknown type\n");
    break;
  }
}

Type *getPtrTypeFromAstType(AstType type) {
  switch (type) {
  case type_int:
    return PointerType::getUnqual(Type::getInt32Ty(*TheContext));
  case type_bool:
    return PointerType::getUnqual(Type::getInt1Ty(*TheContext));
  case type_uint:
    return PointerType::getUnqual(Type::getInt32Ty(*TheContext));
  case type_float:
    return PointerType::getUnqual(Type::getFloatTy(*TheContext));
  case type_double:
    return PointerType::getUnqual(Type::getDoubleTy(*TheContext));
  case type_vec2:
  case type_vec3:
  case type_vec4:
  case type_mat2:
  case type_mat3:
  case type_mat4:
    return PointerType::getUnqual(Type::getFloatTy(*TheContext));
  case type_void:
    return Type::getVoidTy(*TheContext);
  default:
    printf("Error: unknown type\n");
    break;
  }
}

bool isIncrementable(Type *type) {
  if (type->isPointerTy())
    //  if (type->isIntegerTy() || type->isFloatingPointTy() ||
    //  type->isPointerTy() ||
    //      type->isDoubleTy())
    return true;
  else
    return false;
}

Value *ConditionalExpressionAST::codegen() {
  Value *cond = condition->codegen();
  if (!cond)
    return nullptr;

  // Load int from pointer
  if (cond->getType()->isPointerTy()) {
    cond = Builder->CreateLoad(
        getTypeFromAstType(
            currentScope
                ->getIndentifier(
                    ((VariableExprAST *)condition.get())->getName())
                ->first),
        cond, "tmp");
  }

  cond = Builder->CreateICmpNE(
      cond, ConstantInt::get(Type::getInt32Ty(*TheContext), 0), "ifcond");

  Value *trueValue = then->codegen();
  Value *falseValue = else_->codegen();

  if (!trueValue || !falseValue)
    return nullptr;

  return Builder->CreateSelect(cond, trueValue, falseValue, "ifresult");
}

Value *BinaryExpressionAST::codegen() {
  Value *zero;
  Value *one;
  Value *cond;
  Value *cond2;
  Value *left;
  Value *right;
  Value *is_left_zero;
  Value *is_right_zero;
  Value *temp;
  Type *tempType;
  std::string tempName;
  VariableExprAST * tempVar;
  switch (type) {
  case plus_expr: //TOD: handle type conversion eg, int + float
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    if (left->getType()->isPointerTy()) {
      // get element from pointer
      tempName = ((VariableExprAST *)LHS.get())->getName();
      left = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          left, "tmp");
    }
    if (right->getType()->isPointerTy()) {
      tempName = ((VariableExprAST *)RHS.get())->getName();
      right = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          right, "tmp");
    }
    return Builder->CreateAdd(left, right, "addtmp");
  case minus_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    if (left->getType()->isPointerTy()) {
      // get element from pointer
      tempName = ((VariableExprAST *)LHS.get())->getName();
      left = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          left, "tmp");
    }
    if (right->getType()->isPointerTy()) {
      tempName = ((VariableExprAST *)RHS.get())->getName();
      right = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          right, "tmp");
    }
    return Builder->CreateSub(left, right, "subtmp");
  case times_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    if (left->getType()->isPointerTy()) {
      // get element from pointer
      tempName = ((VariableExprAST *)LHS.get())->getName();
      left = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          left, "tmp");
    }
    if (right->getType()->isPointerTy()) {
      tempName = ((VariableExprAST *)RHS.get())->getName();
      right = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          right, "tmp");
    }
    return Builder->CreateMul(left, right, "multmp");
  case divide_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    if (left->getType()->isPointerTy()) {
      // get element from pointer
      tempName = ((VariableExprAST *)LHS.get())->getName();
      left = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          left, "tmp");
    }
    if (right->getType()->isPointerTy()) {
      tempName = ((VariableExprAST *)RHS.get())->getName();
      right = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          right, "tmp");
    }
    return Builder->CreateSDiv(left, right, "divtmp");
  case mod_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    if (left->getType()->isPointerTy()) {
      // get element from pointer
      tempName = ((VariableExprAST *)LHS.get())->getName();
      left = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          left, "tmp");
    }
    if (right->getType()->isPointerTy()) {
      tempName = ((VariableExprAST *)RHS.get())->getName();
      right = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          right, "tmp");
    }
    return Builder->CreateSRem(left, right, "modtmp");
  case and_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    if (left->getType()->isPointerTy()) {
      // get element from pointer
      tempName = ((VariableExprAST *)LHS.get())->getName();
      left = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          left, "tmp");
    }
    if (right->getType()->isPointerTy()) {
      tempName = ((VariableExprAST *)RHS.get())->getName();
      right = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          right, "tmp");
    }
    zero = ConstantInt::get(Type::getInt32Ty(*TheContext), 0);
    is_left_zero = Builder->CreateICmpEQ(left, zero);
    is_right_zero = Builder->CreateICmpEQ(right, zero);
    cond = Builder->CreateAnd(is_left_zero, is_right_zero, "andtmp");
    return Builder->CreateSelect(
        cond, zero, ConstantInt::get(Type::getInt32Ty(*TheContext), 1),
        "andtmp");
  case or_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    if (left->getType()->isPointerTy()) {
      // get element from pointer
      tempName = ((VariableExprAST *)LHS.get())->getName();
      left = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          left, "tmp");
    }
    if (right->getType()->isPointerTy()) {
      tempName = ((VariableExprAST *)RHS.get())->getName();
      right = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          right, "tmp");
    }
    zero = ConstantInt::get(Type::getInt32Ty(*TheContext), 0);
    is_left_zero = Builder->CreateICmpEQ(left, zero);
    is_right_zero = Builder->CreateICmpEQ(right, zero);
    cond = Builder->CreateOr(is_left_zero, is_right_zero, "ortmp");
    return Builder->CreateSelect(
        cond, zero, ConstantInt::get(Type::getInt32Ty(*TheContext), 1),
        "ortmp");
  case xor_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    if (left->getType()->isPointerTy()) {
      // get element from pointer
      tempName = ((VariableExprAST *)LHS.get())->getName();
      left = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          left, "tmp");
    }
    if (right->getType()->isPointerTy()) {
      tempName = ((VariableExprAST *)RHS.get())->getName();
      right = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          right, "tmp");
    }
    zero = ConstantInt::get(Type::getInt32Ty(*TheContext), 0);
    is_left_zero = Builder->CreateICmpEQ(left, zero);
    is_right_zero = Builder->CreateICmpEQ(right, zero);
    cond = Builder->CreateOr(is_left_zero, is_right_zero, "ortmp");
    cond2 = Builder->CreateAnd(Builder->CreateNot(is_left_zero),
                               Builder->CreateNot(is_right_zero), "andtmp");
    one = ConstantInt::get(Type::getInt32Ty(*TheContext), 1);
    return Builder->CreateSelect(
        cond, Builder->CreateSelect(cond2, zero, one, "xortmp1"), one,
        "xortmp2");
  case bit_and_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    if (left->getType()->isPointerTy()) {
      // get element from pointer
      tempName = ((VariableExprAST *)LHS.get())->getName();
      left = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          left, "tmp");
    }
    if (right->getType()->isPointerTy()) {
      tempName = ((VariableExprAST *)RHS.get())->getName();
      right = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          right, "tmp");
    }
    return Builder->CreateAnd(left, right, "bandtmp");
  case bit_or_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    if (left->getType()->isPointerTy()) {
      tempName = ((VariableExprAST *)LHS.get())->getName();
      left = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          left, "tmp");
    }
    if (right->getType()->isPointerTy()) {
      tempName = ((VariableExprAST *)RHS.get())->getName();
      right = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          right, "tmp");
    }
    return Builder->CreateOr(left, right, "bortmp");
  case bit_xor_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    if (left->getType()->isPointerTy()) {
      tempName = ((VariableExprAST *)LHS.get())->getName();
      left = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          left, "tmp");
    }
    if (right->getType()->isPointerTy()) {
      tempName = ((VariableExprAST *)RHS.get())->getName();
      right = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          right, "tmp");
    }
    return Builder->CreateXor(left, right, "bxortmp");
  case left_shift_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    if (left->getType()->isPointerTy()) {
      tempName = ((VariableExprAST *)LHS.get())->getName();
      left = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          left, "tmp");
    }
    if (right->getType()->isPointerTy()) {
      tempName = ((VariableExprAST *)RHS.get())->getName();
      right = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          right, "tmp");
    }
    return Builder->CreateShl(left, right, "lshifttmp");
  case right_shift_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    if (left->getType()->isPointerTy()) {
      tempName = ((VariableExprAST *)LHS.get())->getName();
      left = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          left, "tmp");
    }
    if (right->getType()->isPointerTy()) {
      tempName = ((VariableExprAST *)RHS.get())->getName();
      right = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          right, "tmp");
    }
    return Builder->CreateLShr(left, right, "rshifttmp");
  case less_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    if (left->getType()->isPointerTy()) {
      tempName = ((VariableExprAST *)LHS.get())->getName();
      left = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          left, "tmp");
    }
    if (right->getType()->isPointerTy()) {
      tempName = ((VariableExprAST *)RHS.get())->getName();
      right = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          right, "tmp");
    }
    return Builder->CreateICmpSLT(left, right, "lesstmp");
  case greater_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    if (left->getType()->isPointerTy()) {
      tempName = ((VariableExprAST *)LHS.get())->getName();
      left = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          left, "tmp");
    }
    if (right->getType()->isPointerTy()) {
      tempName = ((VariableExprAST *)RHS.get())->getName();
      right = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          right, "tmp");
    }
    return Builder->CreateICmpSGT(left, right, "greatertmp");
  case less_equal_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    if (left->getType()->isPointerTy()) {
      tempName = ((VariableExprAST *)LHS.get())->getName();
      left = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          left, "tmp");
    }
    if (right->getType()->isPointerTy()) {
      tempName = ((VariableExprAST *)RHS.get())->getName();
      right = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          right, "tmp");
    }
    return Builder->CreateICmpSLE(left, right, "lessequaltmp");
  case greater_equal_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    if (left->getType()->isPointerTy()) {
      tempName = ((VariableExprAST *)LHS.get())->getName();
      left = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          left, "tmp");
    }
    if (right->getType()->isPointerTy()) {
      tempName = ((VariableExprAST *)RHS.get())->getName();
      right = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          right, "tmp");
    }
    return Builder->CreateICmpSGE(left, right, "greaterequaltmp");
  case equal_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    if (left->getType()->isPointerTy()) {
      tempName = ((VariableExprAST *)LHS.get())->getName();
      left = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          left, "tmp");
    }
    if (right->getType()->isPointerTy()) {
      tempName = ((VariableExprAST *)RHS.get())->getName();
      right = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          right, "tmp");
    }
    return Builder->CreateICmpEQ(left, right, "equaltmp");
  case not_equal_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    if (left->getType()->isPointerTy()) {
      tempName = ((VariableExprAST *)LHS.get())->getName();
      left = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          left, "tmp");
    }
    if (right->getType()->isPointerTy()) {
      tempName = ((VariableExprAST *)RHS.get())->getName();
      right = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          right, "tmp");
    }
    return Builder->CreateICmpNE(left, right, "notequaltmp");
  case assign_expr:
    left = LHS->codegen();
    if (!left)
      return nullptr;
    if (left->getType()->isPointerTy()) {
      //      tempName = ((VariableExprAST *)LHS.get())->getName();
      //      tempType =
      //          getTypeFromAstType(currentScope->getIndentifier(tempName)->first);
      right = RHS->codegen();
      if (!right && RHS != nullptr) { // TypeConstructor
        std::unique_ptr<TypeConstructorAST> typeConstructor =
            std::unique_ptr<TypeConstructorAST>(
                (TypeConstructorAST *)RHS.release());
        auto &exprList = typeConstructor->getArgs();
        for (int i = 0; i < exprList->getExpressions().size(); i++) {
          auto &expr = exprList->getExpressions()[i];
          auto currentIndex = ConstantInt::get(*TheContext, APInt(32, i, true));
          auto elementPtr = Builder->CreateGEP(Type::getFloatTy(*TheContext),
                                               left, currentIndex);
          Value *value = expr->codegen();
          // change double or int to float
          if (value->getType()->isDoubleTy()) {
            value =
                Builder->CreateFPTrunc(value, Type::getFloatTy(*TheContext));
          } else if (value->getType()->isIntegerTy()) {
            value = ConstantExpr::getSIToFP((ConstantInt *)value,
                                            Type::getFloatTy(*TheContext));
          }
          Builder->CreateStore(value, elementPtr);
        }
        return left;
      } else if (!right) {
        return nullptr;
      }
      if (right->getType()->isPointerTy()) {
        tempName = ((VariableExprAST *)RHS.get())->getName();
        right = Builder->CreateLoad(
            getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
            right, "tmp");
      }
      return Builder->CreateStore(right, left);
    } else {
      printf("Error: left side of assignment is not a pointer\n");
      return nullptr;
    }
  case plus_assign_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    tempType = right->getType();
    if (right->getType()->isPointerTy()) {
      tempName = ((VariableExprAST *)RHS.get())->getName();
      tempType =
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first);
      right = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          right, "tmp");
    }
    temp = Builder->CreateAdd(Builder->CreateLoad(tempType, left), right);
    Builder->CreateStore(temp, left);
    return temp;
  case minus_assign_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    tempType = right->getType();
    if (right->getType()->isPointerTy()) {
      tempName = ((VariableExprAST *)RHS.get())->getName();
      tempType =
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first);
      right = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          right, "tmp");
    }
    temp = Builder->CreateSub(Builder->CreateLoad(tempType, left), right);
    Builder->CreateStore(temp, left);
    return temp;
  case times_assign_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    tempType = right->getType();
    if (right->getType()->isPointerTy()) {
      tempName = ((VariableExprAST *)RHS.get())->getName();
      tempType =
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first);
      right = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          right, "tmp");
    }
    temp = Builder->CreateMul(Builder->CreateLoad(tempType, left), right);
    Builder->CreateStore(temp, left);
    return temp;
  case divide_assign_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    tempType = right->getType();
    if (right->getType()->isPointerTy()) {
      tempName = ((VariableExprAST *)RHS.get())->getName();
      tempType =
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first);
      right = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          right, "tmp");
    }
    temp = Builder->CreateSDiv(Builder->CreateLoad(tempType, left), right);
    Builder->CreateStore(temp, left);
    return temp;
  case mod_assign_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    tempType = right->getType();
    if (right->getType()->isPointerTy()) {
      tempName = ((VariableExprAST *)RHS.get())->getName();
      tempType =
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first);
      right = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          right, "tmp");
    }
    temp = Builder->CreateSRem(Builder->CreateLoad(tempType, left), right);
    Builder->CreateStore(temp, left);
    return temp;
  case bit_and_assign_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    tempType = right->getType();
    if (right->getType()->isPointerTy()) {
      tempName = ((VariableExprAST *)RHS.get())->getName();
      tempType =
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first);
      right = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          right, "tmp");
    }
    temp = Builder->CreateAnd(Builder->CreateLoad(tempType, left), right);
    Builder->CreateStore(temp, left);
    return temp;
  case bit_or_assign_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    tempType = right->getType();
    if (right->getType()->isPointerTy()) {
      tempName = ((VariableExprAST *)RHS.get())->getName();
      tempType =
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first);
      right = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          right, "tmp");
    }
    temp = Builder->CreateOr(Builder->CreateLoad(tempType, left), right);
    Builder->CreateStore(temp, left);
    return temp;
  case left_shift_assign_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    tempType = right->getType();
    if (right->getType()->isPointerTy()) {
      tempName = ((VariableExprAST *)RHS.get())->getName();
      tempType =
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first);
      right = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          right, "tmp");
    }
    temp = Builder->CreateShl(Builder->CreateLoad(tempType, left), right);
    Builder->CreateStore(temp, left);
    return temp;
  case right_shift_assign_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    tempType = right->getType();
    if (right->getType()->isPointerTy()) {
      tempName = ((VariableExprAST *)RHS.get())->getName();
      tempType =
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first);
      right = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          right, "tmp");
    }
    temp = Builder->CreateLShr(Builder->CreateLoad(tempType, left), right);
    Builder->CreateStore(temp, left);
    return temp;
  case or_assign_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    tempType = right->getType();
    if (right->getType()->isPointerTy()) {
      tempName = ((VariableExprAST *)RHS.get())->getName();
      tempType =
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first);
      right = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          right, "tmp");
    }
    temp = left;
    if (left->getType()->isPointerTy()) {
      tempName = ((VariableExprAST *)LHS.get())->getName();
      tempType =
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first);
      temp = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          left, "tmp");
    }
    zero = ConstantInt::get(Type::getInt32Ty(*TheContext), 0);
    is_left_zero = Builder->CreateICmpEQ(temp, zero);
    is_right_zero = Builder->CreateICmpEQ(right, zero);
    cond = Builder->CreateOr(is_left_zero, is_right_zero, "ortmp");
    temp = Builder->CreateSelect(
        cond, zero, ConstantInt::get(Type::getInt32Ty(*TheContext), 1));
    Builder->CreateStore(temp, left);
    return temp;
  case and_assign_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    tempType = right->getType();
    if (right->getType()->isPointerTy()) {
      tempName = ((VariableExprAST *)RHS.get())->getName();
      tempType =
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first);
      right = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          right, "tmp");
    }
    temp = left;
    if (left->getType()->isPointerTy()) {
      tempName = ((VariableExprAST *)LHS.get())->getName();
      tempType =
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first);
      temp = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          left, "tmp");
    }
    zero = ConstantInt::get(Type::getInt32Ty(*TheContext), 0);
    is_left_zero = Builder->CreateICmpEQ(temp, zero);
    is_right_zero = Builder->CreateICmpEQ(right, zero);
    cond = Builder->CreateAnd(is_left_zero, is_right_zero, "andtmp");
    temp = Builder->CreateSelect(
        cond, zero, ConstantInt::get(Type::getInt32Ty(*TheContext), 1));
    Builder->CreateStore(temp, left);
    return temp;
  case sequence_expr:
    LHS->codegen();
    return RHS->codegen();
  default:
    printf("Error: unknown binary expression\n");
    break;
  }
  return nullptr;
}

Value *PrefixExpressionAST::codegen() { // TODO: 06.21 finish this and after
  Value *var;
  Value *oldValue;
  Value *newValue;
  Type *varType;
  Value *zero;
  Value *cond;
  std::string tempName;
  Value *temp;
  switch (type) {
  case plus_p_expr: // TODO: check the RHS can be assigned, eg, ++vec2.x
    var = RHS->codegen();
    varType = var->getType();
    // check
    if (isIncrementable(varType)) {
      // get element from pointer
      tempName = ((VariableExprAST *)RHS.get())->getName();
      oldValue = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          var, "tmp");
      newValue = Builder->CreateAdd(
          oldValue, ConstantInt::get(llvm::Type::getInt32Ty(*TheContext), 1),
          "newvalue");
      Builder->CreateStore(newValue, var);
      return newValue;
    } else {
      printf("Error: cannot increment this type\n");
      return nullptr;
    }
  case minus_m_expr:
    var = RHS->codegen();
    varType = var->getType();
    // check
    if (isIncrementable(varType)) {
      tempName = ((VariableExprAST *)RHS.get())->getName();
      oldValue = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          var, "tmp");
      newValue = Builder->CreateSub(
          oldValue, ConstantInt::get(llvm::Type::getInt32Ty(*TheContext), 1),
          "newvalue");
      Builder->CreateStore(newValue, var);
      return newValue;
    } else {
      printf("Error: cannot decrement this type\n");
      return nullptr;
    }
  case minus_expr:
    temp = RHS->codegen();
    if (!temp)
      return nullptr;
    if (temp->getType()->isPointerTy()) {
      tempName = ((VariableExprAST *)RHS.get())->getName();
      var = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          temp, "tmp");
    } else {
      var = temp;
    }
    return Builder->CreateNeg(var, "neg");
  case plus_expr:
    temp = RHS->codegen();
    if (!temp)
      return nullptr;
    if (temp->getType()->isPointerTy()) {
      tempName = ((VariableExprAST *)RHS.get())->getName();
      var = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          temp, "tmp");
    } else {
      var = temp;
    }
    ConstantVector
    return var;
  case not_expr: // !, logical not
    zero = ConstantInt::get(Type::getInt32Ty(*TheContext), 0);
    temp = RHS->codegen();
    if (!temp)
      return nullptr;
    if (temp->getType()->isPointerTy()) {
      tempName = ((VariableExprAST *)RHS.get())->getName();
      var = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          temp, "tmp");
    } else {
      var = temp;
    }
    cond = Builder->CreateICmpEQ(var, zero, "ifcond");
    return Builder->CreateSelect(
        cond, zero, ConstantInt::get(Type::getInt32Ty(*TheContext), 1),
        "selecttmp");
  case tilde_expr: // ~, bitwise not
    temp = RHS->codegen();
    if (!temp)
      return nullptr;
    if (temp->getType()->isPointerTy()) {
      tempName = ((VariableExprAST *)RHS.get())->getName();
      var = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
          temp, "tmp");
    } else {
      var = temp;
    }
    return Builder->CreateNot(var, "not");
  default:
    printf("Error: unknown prefix expression\n");
    return nullptr;
  }
}

Value *PostfixExpressionAST::codegen() {
  Value *var;
  Value *oldValue;
  Value *newValue;
  Value *temp;
  std::string varName;
  VariableExprAST *varExpr;
  int index = -1;
  switch (type) {
  case plus_p_expr: // TODO: check the RHS can be assigned, eg, ++vec2.x | also
    temp = LHS->codegen();
    if (isIncrementable(temp->getType())) {
      varName = ((VariableExprAST *)LHS.get())->getName();
      oldValue = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(varName)->first),
          temp, "oldvalue");
      newValue = Builder->CreateAdd(
          oldValue, ConstantInt::get(llvm::Type::getInt32Ty(*TheContext), 1),
          "newvalue");
      Builder->CreateStore(newValue, temp);
      return oldValue;
    } else {
      printf("Error: cannot decrement this type\n");
      return nullptr;
    }
  case minus_m_expr:
    temp = LHS->codegen();
    if (isIncrementable(temp->getType())) {
      varName = ((VariableExprAST *)LHS.get())->getName();
      oldValue = Builder->CreateLoad(
          getTypeFromAstType(currentScope->getIndentifier(varName)->first),
          temp, "oldvalue");
      newValue = Builder->CreateSub(
          oldValue, ConstantInt::get(llvm::Type::getInt32Ty(*TheContext), 1),
          "newvalue");
      Builder->CreateStore(newValue, temp);
      return oldValue;
    } else {
      printf("Error: cannot decrement this type\n");
      return nullptr;
    }
  case dot_expr:
    varExpr = (VariableExprAST *)LHS.get();
    varName = varExpr->getName();
    if (identifier == "x") {
      index = 0;
    }
    if (identifier == "y") {
      index = 1;
    }
    if (identifier == "z") {
      index = 2;
    }
    if (identifier == "w") {
      index = 3;
    }
    if (index == -1) {
      printf("Error: unknown identifier\n");
      return nullptr;
    }
    var = currentScope->getIndentifier(varName)->second;
    // create Vector type
    oldValue = Builder->CreateLoad(
        getTypeFromAstType(currentScope->getIndentifier(varName)->first), var,
        "oldvalue");
    // using index to get the value
    newValue = Builder->CreateExtractElement(oldValue, index, "newvalue");
    return newValue;
  default:
    printf("Error: unknown type\n");
    break;
  }
  return nullptr;
}

// std::vector<ExpressionAST *> getValueFromSequenceAST(BinaryExpressionAST
// *expr) {
//   if () {
//     return {};
//   } else {
//     std::vector<ExpressionAST *> leftValues =
//         getValueFromSequenceAST((BinaryExpressionAST
//         *)(expr->getLHS().get()));
//   }
//   std::vector<ExpressionAST *> leftValues;
//   leftValues.push_back(expr->getRHS().get());
//   return leftValues;
// }

Value *SequenceExpressionAST::codegen() {
  for (int i = 0; i < expressions.size() - 1; i++) {
    expressions[i]->codegen();
  }
  return expressions[expressions.size() - 1]->codegen();
}

Value *SequenceExpressionAST::getArgs() {
  Value *first = expressions[0]->codegen();
  Type *firstType = first->getType();
  Type *vecType = VectorType::get(firstType, expressions.size(), false);
  Value *vecValue = UndefValue::get(vecType);
  for (int i = 0; i < expressions.size(); i++) {
    Value *newValue = expressions[i]->codegen();
    // type cast
    if (newValue->getType() != firstType) {
      // check whether the type is castable
      if (newValue->getType()->isIntegerTy() &&
          firstType->isIntegerTy()) { // int to int
        newValue =
            Builder->CreateIntCast(newValue, firstType, false, "intcast");
      } else if (newValue->getType()->isIntegerTy() &&
                 firstType->isFloatingPointTy()) { // int to float
        newValue = Builder->CreateSIToFP(newValue, firstType, "intcast");
      } else if (newValue->getType()->isFloatingPointTy() &&
                 firstType->isIntegerTy()) { // float to int
        newValue = Builder->CreateFPToSI(newValue, firstType, "intcast");
      } else if (newValue->getType()->isFloatingPointTy() &&
                 firstType->isFloatingPointTy()) { // float to float
        newValue = Builder->CreateFPCast(newValue, firstType, "intcast");
      } else {
        printf("Error: cannot cast type\n");
        return nullptr;
      }
    }
    vecValue = Builder->CreateInsertElement(vecValue, newValue, i);
  }

  Value *vecAlloc = Builder->CreateAlloca(vecType);
  Builder->CreateStore(vecValue, vecAlloc);
  return Builder->CreateLoad(vecType, vecAlloc, "vec");
}

Value *FunctionCallAST::codegen() {
  Function *function = TheModule->getFunction(callee);
  if (!function) {
    printf("Error: unknown function referenced\n");
    return nullptr;
  }

  std::vector<Value *> funcArgs;
  for (const auto &arg : args->getExpressions()) {
    funcArgs.push_back(arg->codegen());
  }

  return Builder->CreateCall(function, funcArgs, "calltmp");
}

Function *FunctionPrototypeAST::codegen() {
  std::vector<Type *> functionArgs;
  functionArgs.reserve(args.size());
  for (auto &arg : args) {
    functionArgs.push_back(getTypeFromAstType(arg->getType()));
  }
  FunctionType *FT =
      FunctionType::get(getTypeFromAstType(returnType), functionArgs, false);
  Function *F =
      Function::Create(FT, Function::ExternalLinkage, name, TheModule.get());
  unsigned Idx = 0;
  for (auto &Arg : F->args()) {
    Arg.setName(args[Idx]->getName());
    //    Value *Alloca =
    //        Builder->CreateAlloca(Arg.getType(), nullptr,
    //        Arg.getName().str());
    //    Builder->CreateStore(&Arg, Alloca);
    currentScope->addIndentifier(args[Idx]->getName(), args[Idx]->getType(),
                                 &Arg);
    ++Idx;
  }
  return F;
}

Value *SentencesAST::codegen() {
  // create a new scope
  currentScope = std::make_shared<Scope>(currentScope);
  scopeSet.insert(currentScope);
  Value *lastValue = nullptr;
  for (auto &sentence : sentences) {
    lastValue = sentence->codegen();
  }
  // restore the scope
  if (currentScope->getParent() != nullptr) {
    currentScope = currentScope->getParent();
  } else {
    currentScope = topScope;
  }
  if (lastValue == nullptr) {
    Builder->CreateRetVoid();
  }
  return lastValue;
}

Value *IfStatementAST::codegen() {
  Value *condValue = condition->codegen();
  if (!condValue)
    return nullptr;
  // int to float
  if (condValue->getType()->isIntegerTy()) {
    condValue = Builder->CreateSIToFP(condValue, Type::getDoubleTy(*TheContext),
                                      "intcast");
  }
  condValue = Builder->CreateFCmpONE(
      condValue, ConstantFP::get(*TheContext, APFloat(0.0)), "ifcond");
  Function *TheFunction = Builder->GetInsertBlock()->getParent();

  BasicBlock *ThenBB = BasicBlock::Create(*TheContext, "then", TheFunction);
  BasicBlock *MergeBB = BasicBlock::Create(*TheContext, "ifcont");

  if (else_) {
    BasicBlock *ElseBB = BasicBlock::Create(*TheContext, "else");
    Builder->CreateCondBr(condValue, ThenBB, ElseBB);

    Builder->SetInsertPoint(ThenBB);
    Value *ThenV = then->codegen();
    if (!ThenV)
      return nullptr;
    Builder->CreateBr(MergeBB);
    ThenBB = Builder->GetInsertBlock();

    TheFunction->insert(TheFunction->end(), ElseBB);
    Builder->SetInsertPoint(ElseBB);
    Value *ElseV = else_->codegen();
    if (!ElseV)
      return nullptr;
    Builder->CreateBr(MergeBB);
    ElseBB = Builder->GetInsertBlock();

    TheFunction->insert(TheFunction->end(), MergeBB);
    Builder->SetInsertPoint(MergeBB);
    PHINode *PN =
        Builder->CreatePHI(ThenV->getType(), 2, "iftmp");
    PN->addIncoming(ThenV, ThenBB);
    PN->addIncoming(ElseV, ElseBB);
    return PN;
  } else {
    Builder->CreateCondBr(condValue, ThenBB, MergeBB);
    Builder->SetInsertPoint(ThenBB);
    Value *ThenV = then->codegen();
    if (!ThenV)
      return nullptr;
    Builder->CreateBr(MergeBB);
    TheFunction->insert(TheFunction->end(), MergeBB);
    Builder->SetInsertPoint(MergeBB);
    return ThenV;
  }
}

Value *ReturnStatementAST::codegen() { // TODO: return type
  // If the return value is not null, generate code for the expression
  Value *retVal = nullptr;
  if (expr != nullptr) {
    retVal = expr->codegen();
  }

  return Builder->CreateRet(retVal);

  //  // Get the current function and insert point
  //  Function *func = Builder->GetInsertBlock()->getParent();
  //  BasicBlock *currBlock = Builder->GetInsertBlock();
  //
  //  // Create a new basic block for the return statement
  //  BasicBlock *retBlock = BasicBlock::Create(*TheContext, "return", func);
  //
  //  // Set the insert point to the new block and create the return
  //  // instruction
  //  Builder->SetInsertPoint(retBlock);
  //  if (retVal) {
  //    Builder->CreateRet(retVal);
  //  } else {
  //    Builder->CreateRetVoid();
  //  }
  //
  //  // Set the insert point back to the current block
  //  Builder->SetInsertPoint(currBlock);
  //
  //  return nullptr;
}

Value *ForStatementAST::codegen() {
  //  std::unique_ptr<SentenceAST> init;
  //  std::unique_ptr<ExpressionAST> condition;
  //  std::unique_ptr<ExpressionAST> step;
  //  std::unique_ptr<SentenceAST> body;
  // Create the basic blocks for the loop.
  BasicBlock *loopBB = BasicBlock::Create(
      *TheContext, "loop", Builder->GetInsertBlock()->getParent());
  BasicBlock *afterBB = BasicBlock::Create(
      *TheContext, "afterloop", Builder->GetInsertBlock()->getParent());

  // Generate LLVM code for the initialization statement.
  init->codegen();

  // Jump to the loop condition.
  Builder->CreateBr(loopBB);

  // Set the insertion point to the loop block.
  Builder->SetInsertPoint(loopBB);

  // Generate LLVM code for the loop condition.
  Value *conditionValue = condition->codegen();

  // Create the loop body block and generate LLVM code for the body statements.
  BasicBlock *bodyBB = BasicBlock::Create(
      *TheContext, "loopbody", Builder->GetInsertBlock()->getParent(), afterBB);
  Builder->CreateCondBr(conditionValue, bodyBB, afterBB);
  Builder->SetInsertPoint(bodyBB);
  body->codegen();

  // Generate LLVM code for the loop step expression and jump back to the loop
  // condition.
  step->codegen();
  Builder->CreateBr(loopBB);

  // Set the insertion point to the after-loop block.
  Builder->SetInsertPoint(afterBB);

  // Return a null value.
  return Constant::getNullValue(Type::getInt32Ty(*TheContext));
}

Value *NumberExprAST::codegen() {
  // depends on the type of the number
  double d;
  float f;
  switch (type) {
  case type_int:
    return ConstantInt::get(*TheContext, APInt(32, stoi(value), true));
  case type_float:
    f = std::stof(value);
    return ConstantFP::get(*TheContext, APFloat(f));
  case type_double:
    d = std::stod(value);
    return ConstantFP::get(*TheContext, APFloat(d));
  default:
    return nullptr;
  }
}

Value *GlobalVariableDefinitionAST::codegen() {
  Type *llvmType = getTypeFromAstType(type);

  auto *gvar = new GlobalVariable(*TheModule, llvmType, false,
                                  GlobalValue::ExternalLinkage, nullptr, name);

  topScope->addIndentifier(name, type, gvar);

  return gvar;
}

Value *VariableExprAST::codegen() {
  // Look up the variable in the symbol table
  Value *varValue = currentScope->getIndentifier(name)->second;
  if (!varValue) {
    printf("Unknown variable name %s\n", name.c_str());
    return nullptr;
  }
  return varValue;
}

Value *VariableIndexExprAST::codegen() {
  // Look up the variable in the symbol table
  Value *varValue = currentScope->getIndentifier(name)->second;
  if (!varValue) {
    printf("Unknown variable name %s\n", name.c_str());
    return nullptr;
  }

  // Generate code for the index expression
  Value *indexValue = index->codegen();
  Type *indexType = indexValue->getType();

  if (indexValue->getType()->isPointerTy()) {
    indexValue = Builder->CreateLoad(indexType, indexValue);
  }

  // Convert the index value to 64-bit integer type
  indexValue =
      Builder->CreateIntCast(indexValue, Type::getInt32Ty(*TheContext), true);

  // Calculate the element pointer using GEP (GetElementPtr) instruction
  Value *zero = ConstantInt::get(*TheContext, llvm::APInt(32, 0, true));
  std::vector<llvm::Value *> indices;
  indices.push_back(zero);
  indices.push_back(indexValue);
  Type *elementType;

  // get type
  switch (currentScope->getIndentifier(name)->first) {
  case type_mat2:
    elementType = VectorType::get(Type::getFloatTy(*TheContext), 2, false);
    break;
  case type_mat3:
    elementType = VectorType::get(Type::getFloatTy(*TheContext), 3, false);
    break;
  case type_mat4:
    elementType = VectorType::get(Type::getFloatTy(*TheContext), 4, false);
    break;
  default:
    printf("Unknown variable type %s\n", name.c_str());
    return nullptr;
  }

  if (elementType == nullptr) {
    printf("Unknown variable type %s\n", name.c_str());
    return nullptr;
  }

  Value *elementPtr = Builder->CreateGEP(elementType, varValue, indices);

  return elementPtr;
}

Function *FunctionDefinitionAST::codegen() {
  // Create scope
  std::shared_ptr<Scope> scope = std::make_shared<Scope>(currentScope);
  scopeSet.insert(scope);
  currentScope = scope;

  // Create the function
  Function *TheFunction = Proto->codegen();

  if (!TheFunction) {
    printf("Error: function definition failed\n");
    return nullptr;
  }

  // Create a new basic block to start insertion into.
  BasicBlock *BB = BasicBlock::Create(*TheContext, "entry", TheFunction);
  Builder->SetInsertPoint(BB);
  Body->codegen();
  // Validate the generated code, checking for consistency.
  verifyFunction(*TheFunction);
  // recover scope
  if (currentScope->getParent() != nullptr) {
    // move shared_ptr to parent
    currentScope = currentScope->getParent();
  } else {
    currentScope = topScope;
  }
  return TheFunction;
}

Value *VariableDefinitionAST::codegen() { // TODO: float i = 1; handle type conversion
  // Create the alloca instruction for the variable
  AllocaInst *allocaInst =
      Builder->CreateAlloca(getTypeFromAstType(type), nullptr, name.c_str());

  // Store the variable in the symbol table
  currentScope->addIndentifier(name, type, allocaInst);

  // If the variable has an initial value, generate code for the
  // expression and store the value in the alloca instruction
  if (init == nullptr) {
    return allocaInst;
  }

  switch (type) {
  case type_int:
  case type_double:
  case type_float:
  case type_bool:
  case type_uint:
    Builder->CreateStore(init->codegen(), allocaInst);
    break;
  default:
    std::unique_ptr<TypeConstructorAST> typeConstructor =
        std::unique_ptr<TypeConstructorAST>(
            (TypeConstructorAST *)init.release());
    auto &exprList = typeConstructor->getArgs();
    for (int i = 0; i < exprList->getExpressions().size(); i++) {
      auto &expr = exprList->getExpressions()[i];
      auto currentIndex = ConstantInt::get(*TheContext, APInt(32, i, true));
      auto elementPtr = Builder->CreateGEP(Type::getFloatTy(*TheContext),
                                           allocaInst, currentIndex);
      Value *value = expr->codegen();
      // change double or int to float
      if (value->getType()->isDoubleTy()) {
        value = Builder->CreateFPTrunc(value, Type::getFloatTy(*TheContext));
      } else if (value->getType()->isIntegerTy()) {
        value = ConstantExpr::getSIToFP((ConstantInt *)value,
                                        Type::getFloatTy(*TheContext));
      }
      Builder->CreateStore(value, elementPtr);
    }
  }
  return allocaInst;
}

Value *LayoutAst::codegen() { return nullptr; }

Value *TopLevelAST::codegen() {
  // Generate code for each statement in the top level
  for (auto &stmt : *definitions) {
    stmt->codegen();
  }
  return nullptr;
}

Value *EmptySentenceAST::codegen() {
  // do nothing
  return nullptr;
}

Value *ExprListAST::codegen() { return nullptr; }

Value *TypeConstructorAST::codegen() { return nullptr; }