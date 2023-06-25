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
    return Type::getInt32Ty(*TheContext);
  case type_bool:
    return Type::getInt1Ty(*TheContext);
  case type_float:
    return Type::getFloatTy(*TheContext);
  case type_double:
    return Type::getDoubleTy(*TheContext);
  case type_vec2:
  case type_vec3:
  case type_vec4:
  case type_mat2:
  case type_mat3:
  case type_mat4:
    return Type::getFloatPtrTy(*TheContext);
  case type_void:
    return Type::getVoidTy(*TheContext);
  default:
    printf("Error: unknown type\n");
    break;
  }
}

Value *getPtrFromPtrOrVector(Value *value) {
  if (value->getType()->isPointerTy())
    return value;
  else {
    // get ptr of the first element of the vector
    auto *vecType = ((VectorType *)value->getType())->getElementType();
    return Builder->CreateGEP(vecType, value, 0, "ptr");
  }
}

Value *getValueFromAllType(Value *value, AstType type) {
  if (value->getType()->isPointerTy()) {
    return Builder->CreateLoad(getTypeFromAstType(type), value, "tmp");
  } else {
    return value;
  }
}

Type *getTypeFromAssignableValue(Value *value) {
  if (value->getType()->isPointerTy()) {
    return getTypeFromAstType(
        currentScope->getIndentifier(((VariableExprAST *)value)->getName())
            ->first);
  } else if (value->getType()->isVectorTy()) {
    // get ptr of the first element of the vector
    Type *vecType = ((VectorType *)value->getType())->getElementType();
    return vecType;
  } else {
    return value->getType();
  }
}

Value *toDouble(Value *value) {
  if (value->getType()->isDoubleTy())
    return value;
  else if (value->getType()->isFloatTy())
    return Builder->CreateFPExt(value, Type::getDoubleTy(*TheContext), "tmp");
  else if (value->getType()->isIntegerTy())
    return Builder->CreateSIToFP(value, Type::getDoubleTy(*TheContext), "tmp");
  else if (value->getType()->isVectorTy())
    // change vector to double vector
    return Builder->CreateFPExt(
        value,
        VectorType::get(Type::getDoubleTy(*TheContext),
                        ((FixedVectorType *)value->getType())->getNumElements(),
                        false),
        "tmp");
  //    return value;
  else {
    printf("Error: unknown type\n");
    return nullptr;
  }
}

Value *doubleTo(Type *type, Value *value) {
  if (type->isDoubleTy())
    return value;
  else if (type->isFloatTy())
    return Builder->CreateFPTrunc(value, Type::getFloatTy(*TheContext), "tmp");
  else if (type->isIntegerTy())
    return Builder->CreateFPToSI(value, Type::getInt32Ty(*TheContext), "tmp");
  else if (type->isVectorTy())
    return Builder->CreateFPTrunc(
        value,
        VectorType::get(Type::getFloatTy(*TheContext),
                        ((FixedVectorType *)type)->getNumElements(), false),
        "tmp");
  else {
    printf("Error: unknown type\n");
    return nullptr;
  }
}

bool isIncrementable(Type *type) {
  if (type->isPointerTy() || type->isVectorTy())
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

  cond = getValueFromAllType(cond, condition->getReturnType());

  // change int 1 to int 32
  cond = Builder->CreateSExt(cond, Type::getInt32Ty(*TheContext), "ifcond");
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
  VariableExprAST *tempVar;
  Type *rightType;
  Type *leftType;
  Value *leftDoubleTemp;
  Value *rightDoubleTemp;
  switch (type) {
  case plus_expr: // TOD: handle type conversion eg, int + float
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    left = getValueFromAllType(left, LHS->getReturnType());
    right = getValueFromAllType(right, RHS->getReturnType());
    leftType = getTypeFromAstType(LHS->getReturnType());
    left = toDouble(left);
    right = toDouble(right);
    if (!right->getType()->isVectorTy() && left->getType()->isVectorTy()) {
      right = Builder->CreateVectorSplat(
          ((FixedVectorType *)left->getType())->getNumElements(), right);
    }
    temp = Builder->CreateFAdd(left, right, "addtmp");
    return doubleTo(leftType, temp);
  case minus_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    left = getValueFromAllType(left, LHS->getReturnType());
    right = getValueFromAllType(right, RHS->getReturnType());
    leftType = getTypeFromAstType(LHS->getReturnType());
    left = toDouble(left);
    right = toDouble(right);
    if (!right->getType()->isVectorTy() && left->getType()->isVectorTy()) {
      right = Builder->CreateVectorSplat(
          ((FixedVectorType *)left->getType())->getNumElements(), right);
    }
    temp = Builder->CreateFSub(left, right, "subtmp");
    return doubleTo(leftType, temp);
  case times_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    left = getValueFromAllType(left, LHS->getReturnType());
    right = getValueFromAllType(right, RHS->getReturnType());
    leftType = getTypeFromAstType(LHS->getReturnType());
    left = toDouble(left);
    right = toDouble(right);
    if (!right->getType()->isVectorTy() && left->getType()->isVectorTy()) {
      right = Builder->CreateVectorSplat(
          ((FixedVectorType *)left->getType())->getNumElements(), right);
    }
    temp = Builder->CreateFMul(left, right, "multmp");
    return doubleTo(leftType, temp);
  case divide_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    left = getValueFromAllType(left, LHS->getReturnType());
    right = getValueFromAllType(right, RHS->getReturnType());
    leftType = getTypeFromAstType(LHS->getReturnType());
    left = toDouble(left);
    right = toDouble(right);
    if (!right->getType()->isVectorTy() && left->getType()->isVectorTy()) {
      right = Builder->CreateVectorSplat(
          ((FixedVectorType *)left->getType())->getNumElements(), right);
    }
    temp = Builder->CreateFDiv(left, right, "divtmp");
    return doubleTo(leftType, temp);
  case mod_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    left = getValueFromAllType(left, LHS->getReturnType());
    right = getValueFromAllType(right, RHS->getReturnType());
    leftType = getTypeFromAstType(LHS->getReturnType());
    rightType = getTypeFromAstType(RHS->getReturnType());
    if (!leftType->isIntegerTy() || !rightType->isIntegerTy()) {
      printf("Error: mod operator only works on integer\n");
      return nullptr;
    }
    temp = Builder->CreateSRem(left, right, "modtmp");
    return doubleTo(leftType, temp);
  case and_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    left = getValueFromAllType(left, LHS->getReturnType());
    right = getValueFromAllType(right, RHS->getReturnType());
    if (!left->getType()->isIntegerTy() || !right->getType()->isIntegerTy()) {
      printf("Error: and operator only works on integer\n");
      return nullptr;
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
    left = getValueFromAllType(left, LHS->getReturnType());
    right = getValueFromAllType(right, RHS->getReturnType());
    if (!left->getType()->isIntegerTy() || !right->getType()->isIntegerTy()) {
      printf("Error: or operator only works on integer\n");
      return nullptr;
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
    left = getValueFromAllType(left, LHS->getReturnType());
    right = getValueFromAllType(right, RHS->getReturnType());
    if (!left->getType()->isIntegerTy() || !right->getType()->isIntegerTy()) {
      printf("Error: xor operator only works on integer\n");
      return nullptr;
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
    left = getValueFromAllType(left, LHS->getReturnType());
    right = getValueFromAllType(right, RHS->getReturnType());
    if (!left->getType()->isIntegerTy() || !right->getType()->isIntegerTy()) {
      printf("Error: bit_and operator only works on integer\n");
      return nullptr;
    }
    return Builder->CreateAnd(left, right, "bandtmp");
  case bit_or_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    left = getValueFromAllType(left, LHS->getReturnType());
    right = getValueFromAllType(right, RHS->getReturnType());
    if (!left->getType()->isIntegerTy() || !right->getType()->isIntegerTy()) {
      printf("Error: bit_or operator only works on integer\n");
      return nullptr;
    }
    return Builder->CreateOr(left, right, "bortmp");
  case bit_xor_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    left = getValueFromAllType(left, LHS->getReturnType());
    right = getValueFromAllType(right, RHS->getReturnType());
    if (!left->getType()->isIntegerTy() || !right->getType()->isIntegerTy()) {
      printf("Error: bit_xor operator only works on integer\n");
      return nullptr;
    }
    return Builder->CreateXor(left, right, "bxortmp");
  case left_shift_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    left = getValueFromAllType(left, LHS->getReturnType());
    right = getValueFromAllType(right, RHS->getReturnType());
    if (!left->getType()->isIntegerTy() || !right->getType()->isIntegerTy()) {
      printf("Error: left_shift operator only works on integer\n");
      return nullptr;
    }
    return Builder->CreateShl(left, right, "lshifttmp");
  case right_shift_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    left = getValueFromAllType(left, LHS->getReturnType());
    right = getValueFromAllType(right, RHS->getReturnType());
    if (!left->getType()->isIntegerTy() || !right->getType()->isIntegerTy()) {
      printf("Error: right_shift operator only works on integer\n");
      return nullptr;
    }
    return Builder->CreateLShr(left, right, "rshifttmp");
  case less_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    left = getValueFromAllType(left, LHS->getReturnType());
    right = getValueFromAllType(right, RHS->getReturnType());
    leftType = getTypeFromAstType(LHS->getReturnType());
    left = toDouble(left);
    right = toDouble(right);
    temp = Builder->CreateFCmpULT(left, right, "lesstmp");
    // return bool using i32
    return temp;
  case greater_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    left = getValueFromAllType(left, LHS->getReturnType());
    right = getValueFromAllType(right, RHS->getReturnType());
    leftType = getTypeFromAstType(LHS->getReturnType());
    left = toDouble(left);
    right = toDouble(right);
    temp = Builder->CreateFCmpUGT(left, right, "greatertmp");
    // change bool to 32 int
    temp = Builder->CreateSExt(temp, Type::getInt32Ty(*TheContext), "ifcond");
    return temp;
  case less_equal_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    left = getValueFromAllType(left, LHS->getReturnType());
    right = getValueFromAllType(right, RHS->getReturnType());
    leftType = getTypeFromAstType(LHS->getReturnType());
    left = toDouble(left);
    right = toDouble(right);
    temp = Builder->CreateFCmpULE(left, right, "lessequaltmp");
    temp = Builder->CreateSExt(temp, Type::getInt32Ty(*TheContext), "ifcond");

    return temp;
  case greater_equal_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    left = getValueFromAllType(left, LHS->getReturnType());
    right = getValueFromAllType(right, RHS->getReturnType());
    leftType = getTypeFromAstType(LHS->getReturnType());
    left = toDouble(left);
    right = toDouble(right);
    temp = Builder->CreateFCmpUGE(left, right, "greaterequaltmp");
    temp = Builder->CreateSExt(temp, Type::getInt32Ty(*TheContext), "ifcond");

    return temp;
  case equal_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    left = getValueFromAllType(left, LHS->getReturnType());
    right = getValueFromAllType(right, RHS->getReturnType());
    leftType = getTypeFromAstType(LHS->getReturnType());
    left = toDouble(left);
    right = toDouble(right);
    temp = Builder->CreateFCmpUEQ(left, right, "equaltmp");
    temp = Builder->CreateSExt(temp, Type::getInt32Ty(*TheContext), "ifcond");

    return temp;
  case not_equal_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    left = getValueFromAllType(left, LHS->getReturnType());
    right = getValueFromAllType(right, RHS->getReturnType());
    leftType = getTypeFromAstType(LHS->getReturnType());
    left = toDouble(left);
    right = toDouble(right);
    temp = Builder->CreateFCmpUNE(left, right, "notequaltmp");
    temp = Builder->CreateSExt(temp, Type::getInt32Ty(*TheContext), "ifcond");

    return temp;
  case assign_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    if (left->getType()->isPointerTy()) {
      right = getValueFromAllType(right, RHS->getReturnType());
      left = getPtrFromPtrOrVector(left);
      leftType = getTypeFromAstType(LHS->getReturnType());
      if (leftType->isVectorTy()) {
        // store to the vector
        Builder->CreateStore(right, left);
        return Builder->CreateLoad(leftType, left);
      } else {
        rightDoubleTemp = toDouble(right);
        temp = doubleTo(leftType, rightDoubleTemp);
        // store to the pointer
        Builder->CreateStore(temp, left);
        return temp;
      }
    } else {
      printf("Error: left side of assignment is not a pointer\n");
      return nullptr;
    }
  case plus_assign_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    if (left->getType()->isPointerTy()) {
      right = getValueFromAllType(right, RHS->getReturnType());
      left = getPtrFromPtrOrVector(left);
      leftType = getTypeFromAstType(LHS->getReturnType());
      if (leftType->isVectorTy()) {
        // store to the vector
        temp = Builder->CreateLoad(leftType, left);
        temp = Builder->CreateFAdd(temp, right, "addtmp");
        Builder->CreateStore(temp, left);
        return temp;
      } else {
        rightDoubleTemp = toDouble(right);
        temp = Builder->CreateLoad(leftType, left);
        temp = toDouble(temp);
        temp = Builder->CreateFAdd(temp, rightDoubleTemp, "addtmp");
        temp = doubleTo(leftType, temp);
        // store to the pointer
        Builder->CreateStore(temp, left);
        return temp;
      }
    } else {
      printf("Error: left side of assignment is not a pointer\n");
      return nullptr;
    }
    //    left = LHS->codegen();
    //    right = RHS->codegen();
    //    if (!left || !right)
    //      return nullptr;
    //    tempType = right->getType();
    //    if (right->getType()->isPointerTy()) {
    //      tempName = ((VariableExprAST *)RHS.get())->getName();
    //      tempType =
    //          getTypeFromAstType(currentScope->getIndentifier(tempName)->first);
    //      right = Builder->CreateLoad(
    //          getTypeFromAstType(currentScope->getIndentifier(tempName)->first),
    //          right, "tmp");
    //    }
    //    temp = Builder->CreateAdd(Builder->CreateLoad(tempType, left), right);
    //    Builder->CreateStore(temp, left);
    //    return temp;
  case minus_assign_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    if (left->getType()->isPointerTy()) {
      right = getValueFromAllType(right, RHS->getReturnType());
      left = getPtrFromPtrOrVector(left);
      leftType = getTypeFromAstType(LHS->getReturnType());
      if (leftType->isVectorTy()) {
        // store to the vector
        temp = Builder->CreateLoad(leftType, left);
        temp = Builder->CreateFSub(temp, right, "subtmp");
        Builder->CreateStore(temp, left);
        return temp;
      } else {
        rightDoubleTemp = toDouble(right);
        temp = Builder->CreateLoad(leftType, left);
        temp = toDouble(temp);
        temp = Builder->CreateFSub(temp, rightDoubleTemp, "subtmp");
        temp = doubleTo(leftType, temp);
        // store to the pointer
        Builder->CreateStore(temp, left);
        return temp;
      }
    } else {
      printf("Error: left side of assignment is not a pointer\n");
      return nullptr;
    }
  case times_assign_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    if (left->getType()->isPointerTy()) {
      right = getValueFromAllType(right, RHS->getReturnType());
      left = getPtrFromPtrOrVector(left);
      leftType = getTypeFromAstType(LHS->getReturnType());
      if (leftType->isVectorTy()) {
        // store to the vector
        temp = Builder->CreateLoad(leftType, left);
        temp = Builder->CreateFMul(temp, right, "multmp");
        Builder->CreateStore(temp, left);
        return temp;
      } else {
        rightDoubleTemp = toDouble(right);
        temp = Builder->CreateLoad(leftType, left);
        temp = toDouble(temp);
        temp = Builder->CreateFMul(temp, rightDoubleTemp, "multmp");
        temp = doubleTo(leftType, temp);
        // store to the pointer
        Builder->CreateStore(temp, left);
        return temp;
      }
    } else {
      printf("Error: left side of assignment is not a pointer\n");
      return nullptr;
    }
  case divide_assign_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    if (left->getType()->isPointerTy()) {
      right = getValueFromAllType(right, RHS->getReturnType());
      left = getPtrFromPtrOrVector(left);
      leftType = getTypeFromAstType(LHS->getReturnType());
      if (leftType->isVectorTy()) {
        // store to the vector
        temp = Builder->CreateLoad(leftType, left);
        temp = Builder->CreateFDiv(temp, right, "divtmp");
        Builder->CreateStore(temp, left);
        return temp;
      } else {
        rightDoubleTemp = toDouble(right);
        temp = Builder->CreateLoad(leftType, left);
        temp = toDouble(temp);
        temp = Builder->CreateFDiv(temp, rightDoubleTemp, "divtmp");
        temp = doubleTo(leftType, temp);
        // store to the pointer
        Builder->CreateStore(temp, left);
        return temp;
      }
    } else {
      printf("Error: left side of assignment is not a pointer\n");
      return nullptr;
    }
  case mod_assign_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    if (left->getType()->isPointerTy()) {
      right = getValueFromAllType(right, RHS->getReturnType());
      left = getPtrFromPtrOrVector(left);
      leftType = getTypeFromAstType(LHS->getReturnType());
      if (leftType->isVectorTy()) {
        // store to the vector
        temp = Builder->CreateLoad(leftType, left);
        temp = Builder->CreateFRem(temp, right, "modtmp");
        Builder->CreateStore(temp, left);
        return temp;
      } else {
        rightDoubleTemp = toDouble(right);
        temp = Builder->CreateLoad(leftType, left);
        temp = toDouble(temp);
        temp = Builder->CreateFRem(temp, rightDoubleTemp, "modtmp");
        temp = doubleTo(leftType, temp);
        // store to the pointer
        Builder->CreateStore(temp, left);
        return temp;
      }
    } else {
      printf("Error: left side of assignment is not a pointer\n");
      return nullptr;
    }
  case bit_and_assign_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    if (left->getType()->isPointerTy() || left->getType()->isVectorTy()) {
      rightType = getTypeFromAssignableValue(right);
      right = getValueFromAllType(right, RHS->getReturnType());
      left = getPtrFromPtrOrVector(left);
      leftType = left->getType();
      // load int from left pointer
      if (!rightType->isIntegerTy() || !leftType->isIntegerTy()) {
        printf("Error: don't support bit and assign for non-integer type\n");
      }
      temp = Builder->CreateLoad(leftType, left);
      temp = Builder->CreateAnd(temp, right);
      // store to the pointer
      Builder->CreateStore(temp, left);
      return temp;
    } else {
      printf("Error: left side of assignment is not a pointer\n");
      return nullptr;
    }
  case bit_or_assign_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    if (left->getType()->isPointerTy() || left->getType()->isVectorTy()) {
      rightType = getTypeFromAssignableValue(right);
      right = getValueFromAllType(right, RHS->getReturnType());
      left = getPtrFromPtrOrVector(left);
      leftType = left->getType();
      // load int from left pointer
      if (!rightType->isIntegerTy() || !leftType->isIntegerTy()) {
        printf("Error: don't support bit or assign for non-integer type\n");
      }
      temp = Builder->CreateLoad(leftType, left);
      temp = Builder->CreateOr(temp, right);
      // store to the pointer
      Builder->CreateStore(temp, left);
      return temp;
    } else {
      printf("Error: left side of assignment is not a pointer\n");
      return nullptr;
    }
  case left_shift_assign_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    if (left->getType()->isPointerTy()) {
      right = getValueFromAllType(right, RHS->getReturnType());
      left = getPtrFromPtrOrVector(left);
      leftType = getTypeFromAstType(LHS->getReturnType());
      if (!getTypeFromAstType(LHS->getReturnType())->isIntegerTy() ||
          !right->getType()->isIntegerTy()) {
        printf("Error: don't support left shift assign for non-integer type\n");
      }
      temp = Builder->CreateLoad(leftType, left);
      temp = Builder->CreateShl(temp, right);
      // store to the pointer
      Builder->CreateStore(temp, left);
      return temp;
    } else {
      printf("Error: left side of assignment is not a pointer\n");
      return nullptr;
    }
  case right_shift_assign_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    if (left->getType()->isPointerTy()) {
      right = getValueFromAllType(right, RHS->getReturnType());
      left = getPtrFromPtrOrVector(left);
      leftType = getTypeFromAstType(LHS->getReturnType());
      if (!getTypeFromAstType(LHS->getReturnType())->isIntegerTy() ||
          !right->getType()->isIntegerTy()) {
        printf(
            "Error: don't support right shift assign for non-integer type\n");
      }
      temp = Builder->CreateLoad(leftType, left);
      temp = Builder->CreateAShr(temp, right);
      // store to the pointer
      Builder->CreateStore(temp, left);
      return temp;
    } else {
      printf("Error: left side of assignment is not a pointer\n");
      return nullptr;
    }
  case or_assign_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    if (left->getType()->isPointerTy()) {
      rightType = getTypeFromAstType(RHS->getReturnType());
      right = getValueFromAllType(right, RHS->getReturnType());
      left = getPtrFromPtrOrVector(left);
      leftType = getTypeFromAstType(LHS->getReturnType());
      // load int from left pointer
      if (!rightType->isIntegerTy() || !leftType->isIntegerTy()) {
        printf("Error: don't support or assign for non-integer type\n");
      }
      zero = ConstantInt::get(Type::getInt32Ty(*TheContext), 0);
      temp = Builder->CreateLoad(leftType, left);
      is_left_zero = Builder->CreateICmpEQ(temp, zero);
      is_right_zero = Builder->CreateICmpEQ(right, zero);
      cond = Builder->CreateOr(is_left_zero, is_right_zero, "ortmp");
      temp = Builder->CreateSelect(
          cond, zero, ConstantInt::get(Type::getInt32Ty(*TheContext), 1));
      Builder->CreateStore(temp, left);
      return temp;
    } else {
      printf("Error: left side of assignment is not a pointer\n");
      return nullptr;
    }
  case and_assign_expr:
    left = LHS->codegen();
    right = RHS->codegen();
    if (!left || !right)
      return nullptr;
    if (left->getType()->isPointerTy() || left->getType()->isVectorTy()) {
      rightType = getTypeFromAstType(RHS->getReturnType());
      right = getValueFromAllType(right, RHS->getReturnType());
      left = getPtrFromPtrOrVector(left);
      leftType = getTypeFromAstType(LHS->getReturnType());
      // load int from left pointer
      if (!rightType->isIntegerTy() || !leftType->isIntegerTy()) {
        printf("Error: don't support and assign for non-integer type\n");
      }
      zero = ConstantInt::get(Type::getInt32Ty(*TheContext), 0);
      temp = Builder->CreateLoad(leftType, left);
      is_left_zero = Builder->CreateICmpEQ(temp, zero);
      is_right_zero = Builder->CreateICmpEQ(right, zero);
      cond = Builder->CreateAnd(is_left_zero, is_right_zero, "andtmp");
      temp = Builder->CreateSelect(
          cond, zero, ConstantInt::get(Type::getInt32Ty(*TheContext), 1));
      Builder->CreateStore(temp, left);
      return temp;
    } else {
      printf("Error: left side of assignment is not a pointer\n");
      return nullptr;
    }
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
  Value *left;
  Value *right;
  Type *leftType;
  Type *rightType;
  switch (type) {
  case plus_p_expr: // TODO: check the RHS can be assigned, eg, ++vec2.x
    var = RHS->codegen();
    varType = var->getType();
    // check
    if (isIncrementable(varType)) {
      // get element from pointer
      right = getValueFromAllType(var, RHS->getReturnType());
      rightType = getTypeFromAstType(RHS->getReturnType());
      // load int from left pointer
      if (!rightType->isIntegerTy()) {
        printf("Error: don't support plus plus for non-integer type\n");
      }
      temp = Builder->CreateAdd(
          right, ConstantInt::get(llvm::Type::getInt32Ty(*TheContext), 1));
      // store to the pointer
      Builder->CreateStore(temp, getPtrFromPtrOrVector(var));
      return temp;
    } else {
      printf("Error: cannot increment this type\n");
      return nullptr;
    }
  case minus_m_expr:
    var = RHS->codegen();
    varType = var->getType();
    // check
    if (isIncrementable(varType)) {
      // get element from pointer
      right = getValueFromAllType(var, RHS->getReturnType());
      rightType = getTypeFromAstType(RHS->getReturnType());
      // load int from left pointer
      if (!rightType->isIntegerTy()) {
        printf("Error: don't support plus plus for non-integer type\n");
      }
      temp = Builder->CreateSub(
          right, ConstantInt::get(llvm::Type::getInt32Ty(*TheContext), 1));
      // store to the pointer
      Builder->CreateStore(temp, getPtrFromPtrOrVector(var));
      return temp;
    } else {
      printf("Error: cannot increment this type\n");
      return nullptr;
    }
  case minus_expr:
    temp = RHS->codegen();
    if (!temp)
      return nullptr;
    if (isIncrementable(temp->getType())) {
      temp = getValueFromAllType(temp, RHS->getReturnType());
    }
    right = toDouble(temp);
    temp = Builder->CreateNeg(right, "negtmp");
    temp = doubleTo(getTypeFromAstType(RHS->getReturnType()), temp);
    return temp;
  case plus_expr:
    temp = RHS->codegen();
    if (!temp)
      return nullptr;
    if (isIncrementable(temp->getType())) {
      temp = getValueFromAllType(temp, RHS->getReturnType());
    }
    return temp;
  case not_expr: // !, logical not
    zero = ConstantInt::get(Type::getInt32Ty(*TheContext), 0);
    temp = RHS->codegen();
    if (!temp)
      return nullptr;
    if (isIncrementable(temp->getType())) {
      temp = getValueFromAllType(temp, RHS->getReturnType());
    }
    cond = Builder->CreateICmpEQ(temp, zero, "ifcond");
    return Builder->CreateSelect(
        cond, zero, ConstantInt::get(Type::getInt32Ty(*TheContext), 1),
        "selecttmp");
  case tilde_expr: // ~, bitwise not
    temp = RHS->codegen();
    if (!temp)
      return nullptr;
    if (isIncrementable(temp->getType())) {
      temp = getValueFromAllType(temp, RHS->getReturnType());
    }
    return Builder->CreateNot(temp, "nottmp");
  default:
    printf("Error: unknown prefix expression\n");
    return nullptr;
  }
}

Value *PostfixExpressionAST::codegen() {
  Value *var;
  AllocaInst *left;
  Value *oldValue;
  Value *newValue;
  Value *temp;
  std::string varName;
  VariableExprAST *varExpr;
  Type *varType;
  int index = -1;
  switch (type) {
  case plus_p_expr: // TODO: check the RHS can be assigned, eg, ++vec2.x | also
    temp = LHS->codegen();
    if (isIncrementable(temp->getType())) {
      var = getValueFromAllType(temp, LHS->getReturnType());
      newValue = Builder->CreateAdd(
          var, ConstantInt::get(llvm::Type::getInt32Ty(*TheContext), 1),
          "newvalue");
      Builder->CreateStore(newValue, getPtrFromPtrOrVector(temp));
      return var;
    } else {
      printf("Error: cannot decrement this type\n");
      return nullptr;
    }
  case minus_m_expr:
    temp = LHS->codegen();
    if (isIncrementable(temp->getType())) {
      var = getValueFromAllType(temp, LHS->getReturnType());
      newValue = Builder->CreateSub(
          var, ConstantInt::get(llvm::Type::getInt32Ty(*TheContext), 1),
          "newvalue");
      Builder->CreateStore(newValue, getPtrFromPtrOrVector(temp));
      return var;
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
    var = LHS->codegen();
    varType = getTypeFromAstType(LHS->getReturnType());
    // return the pointer to the element
    if (varType->isVectorTy() || varType->isPointerTy()) {
      temp = ConstantInt::get(Type::getInt32Ty(*TheContext), index);
      std::vector<Value *> indices;
      // sub one from index
      indices.push_back(ConstantInt::get(Type::getInt32Ty(*TheContext), 0));
      indices.push_back(temp);
      // create one element vector using indices
      temp = Builder->CreateGEP(
          VectorType::get(Type::getFloatTy(*TheContext), 1, false), var,
          indices); // TODO: dont know how to use GEP
      return temp;
    } else {
      printf("Error: cannot extract element from this type\n");
      return nullptr;
    }
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
  Value *last = expressions[expressions.size() - 1]->codegen();
  if (last->getType()->isPointerTy()) {
    return Builder->CreateLoad(
        getTypeFromAstType(
            expressions[expressions.size() - 1]->getReturnType()),
        last);
  } else {
    return last;
  }
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
  //  if (lastValue == nullptr) {
  //    Builder->CreateRetVoid();
  //  }
  // return zero
  return ConstantFP::get(*TheContext, APFloat(0.0));
}

Value *IfStatementAST::codegen() {
  Value *condValue = condition->codegen();
  if (!condValue)
    return nullptr;
  // int to float
  if (condValue->getType()->isPointerTy()) {
    condValue = Builder->CreateLoad(
        getTypeFromAstType(condition->getReturnType()), condValue);
  }
  if (condValue->getType()->isIntegerTy()) {
    condValue = Builder->CreateSIToFP(condValue, Type::getDoubleTy(*TheContext),
                                      "intcast");
  } else if (condValue->getType()->isFloatingPointTy()) {
    condValue = Builder->CreateFPCast(condValue, Type::getDoubleTy(*TheContext),
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
    PHINode *PN = Builder->CreatePHI(ThenV->getType(), 2, "iftmp");
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
  } else {
    return Builder->CreateRetVoid();
  }

  if (retVal->getType()->isPointerTy()) {
    retVal = Builder->CreateLoad(getTypeFromAstType(expr->getReturnType()),
                                 retVal, "retVal");
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

  auto *gvar = TheModule->getOrInsertGlobal(name, llvmType);

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
  std::vector<Value *> indices;
  // sub one from index
  indices.push_back(ConstantInt::get(*TheContext, APInt(32, 0, true)));
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

  Value *elementPtr = Builder->CreateGEP(
      elementType, varValue,
      indices); //  TODO: I dont understand the meaning of indices

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

  checkAndInsertVoidReturn(TheFunction);

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
void FunctionDefinitionAST::checkAndInsertVoidReturn(Function *func) {
  for (Function::iterator b = func->begin(), be = func->end(); b != be; ++b) {
    BasicBlock *BB = &*b;
    if (BB->getTerminator() == nullptr) {
      Builder->SetInsertPoint(BB);
      if (func->getReturnType()->isVoidTy())
        Builder->CreateRetVoid();
      else
        Builder->CreateRet(Constant::getNullValue(func->getReturnType()));
    }
  }
}

Value *
VariableDefinitionAST::codegen() { // TODO: float i = 1; handle type conversion
  // Create the alloca instruction for the variable
  AllocaInst *allocaInst =
      Builder->CreateAlloca(getTypeFromAstType(type), nullptr, name.c_str());

  // If the variable has an initial value, generate code for the
  // expression and store the value in the alloca instruction
  if (init == nullptr) {
    return allocaInst;
  }

  Value *initValue = init->codegen();

  if (initValue->getType()->isPointerTy()) {
    initValue = Builder->CreateLoad(getTypeFromAstType(init->getReturnType()),
                                    initValue);
  }

  Type *type1 = getTypeFromAstType(type);

  // type is double
  if (!type1->isDoubleTy() && initValue->getType()->isDoubleTy()) {
    initValue = doubleTo(type1,initValue);
  }

  Builder->CreateStore(initValue, allocaInst);
  // Store the variable in the symbol table
  currentScope->addIndentifier(name, type, allocaInst);
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

Value *TypeConstructorAST::codegen() {
  Value *Vec;
  int index;
  switch (type) {
  case type_void:
    return nullptr;
  case type_bool:
  case type_int:
  case type_float:
  case type_double:
    return args->getExpressions()[0]->codegen();
  case type_vec2:
    Vec = ConstantAggregateZero::get(
        VectorType::get(Type::getFloatTy(*TheContext), 2, false));
    index = 0;
    for (auto &value : args->getExpressions()) {
      auto valueCode = value->codegen();
      if (valueCode->getType()->isDoubleTy()) {
        valueCode =
            Builder->CreateFPTrunc(valueCode, Type::getFloatTy(*TheContext));
      } else if (valueCode->getType()->isIntegerTy()) {
        valueCode = ConstantExpr::getSIToFP((ConstantInt *)valueCode,
                                            Type::getFloatTy(*TheContext));
      } else if (valueCode->getType()->isPointerTy()) {
        auto elementType = getTypeFromAstType(value->getReturnType());
        valueCode = Builder->CreateLoad(elementType, valueCode);
        if (elementType->isIntegerTy()) {
          valueCode =
              Builder->CreateSIToFP(valueCode, Type::getFloatTy(*TheContext));
        } else if (elementType->isDoubleTy()) {
          valueCode =
              Builder->CreateFPTrunc(valueCode, Type::getFloatTy(*TheContext));
        }
      }
      Vec = Builder->CreateInsertElement(Vec, valueCode, index);
      index++;
    }
    return Vec;
  case type_vec3:
    Vec = ConstantAggregateZero::get(
        VectorType::get(Type::getFloatTy(*TheContext), 3, false));
    index = 0;
    for (auto &value : args->getExpressions()) {
      auto valueCode = value->codegen();
      if (valueCode->getType()->isDoubleTy()) {
        valueCode =
            Builder->CreateFPTrunc(valueCode, Type::getFloatTy(*TheContext));
      } else if (valueCode->getType()->isIntegerTy()) {
        valueCode = ConstantExpr::getSIToFP((ConstantInt *)valueCode,
                                            Type::getFloatTy(*TheContext));
      } else if (valueCode->getType()->isPointerTy()) {
        auto elementType = getTypeFromAstType(value->getReturnType());
        valueCode = Builder->CreateLoad(elementType, valueCode);
        if (elementType->isIntegerTy()) {
          valueCode =
              Builder->CreateSIToFP(valueCode, Type::getFloatTy(*TheContext));
        } else if (elementType->isDoubleTy()) {
          valueCode =
              Builder->CreateFPTrunc(valueCode, Type::getFloatTy(*TheContext));
        }
      }
      Vec = Builder->CreateInsertElement(Vec, valueCode, index);
      index++;
    }
    return Vec;
  case type_vec4:
    Vec = ConstantAggregateZero::get(
        VectorType::get(Type::getFloatTy(*TheContext), 4, false));
    index = 0;
    for (auto &value : args->getExpressions()) {
      auto valueCode = value->codegen();
      if (valueCode->getType()->isDoubleTy()) {
        valueCode =
            Builder->CreateFPTrunc(valueCode, Type::getFloatTy(*TheContext));
      } else if (valueCode->getType()->isIntegerTy()) {
        valueCode = ConstantExpr::getSIToFP((ConstantInt *)valueCode,
                                            Type::getFloatTy(*TheContext));
      } else if (valueCode->getType()->isPointerTy()) {
        auto elementType = getTypeFromAstType(value->getReturnType());
        valueCode = Builder->CreateLoad(elementType, valueCode);
        if (elementType->isIntegerTy()) {
          valueCode =
              Builder->CreateSIToFP(valueCode, Type::getFloatTy(*TheContext));
        } else if (elementType->isDoubleTy()) {
          valueCode =
              Builder->CreateFPTrunc(valueCode, Type::getFloatTy(*TheContext));
        }
      }
      Vec = Builder->CreateInsertElement(Vec, valueCode, index);
      index++;
    }
    return Vec;
  case type_mat2:
    Vec = ConstantAggregateZero::get(
        VectorType::get(Type::getFloatTy(*TheContext), 4, false));
    index = 0;
    for (auto &value : args->getExpressions()) {
      auto valueCode = value->codegen();
      if (valueCode->getType()->isDoubleTy()) {
        valueCode =
            Builder->CreateFPTrunc(valueCode, Type::getFloatTy(*TheContext));
      } else if (valueCode->getType()->isIntegerTy()) {
        valueCode = ConstantExpr::getSIToFP((ConstantInt *)valueCode,
                                            Type::getFloatTy(*TheContext));
      } else if (valueCode->getType()->isPointerTy()) {
        auto elementType = getTypeFromAstType(value->getReturnType());
        valueCode = Builder->CreateLoad(elementType, valueCode);
        if (elementType->isIntegerTy()) {
          valueCode =
              Builder->CreateSIToFP(valueCode, Type::getFloatTy(*TheContext));
        } else if (elementType->isDoubleTy()) {
          valueCode =
              Builder->CreateFPTrunc(valueCode, Type::getFloatTy(*TheContext));
        }
      }
      Vec = Builder->CreateInsertElement(Vec, valueCode, index);
    }
    index++;
    return Vec;
  case type_mat3:
    Vec = ConstantAggregateZero::get(
        VectorType::get(Type::getFloatTy(*TheContext), 9, false));
    index = 0;
    for (auto &value : args->getExpressions()) {
      auto valueCode = value->codegen();
      if (valueCode->getType()->isDoubleTy()) {
        valueCode =
            Builder->CreateFPTrunc(valueCode, Type::getFloatTy(*TheContext));
      } else if (valueCode->getType()->isIntegerTy()) {
        valueCode = ConstantExpr::getSIToFP((ConstantInt *)valueCode,
                                            Type::getFloatTy(*TheContext));
      } else if (valueCode->getType()->isPointerTy()) {
        auto elementType = getTypeFromAstType(value->getReturnType());
        valueCode = Builder->CreateLoad(elementType, valueCode);
        if (elementType->isIntegerTy()) {
          valueCode =
              Builder->CreateSIToFP(valueCode, Type::getFloatTy(*TheContext));
        } else if (elementType->isDoubleTy()) {
          valueCode =
              Builder->CreateFPTrunc(valueCode, Type::getFloatTy(*TheContext));
        }
      }
      Vec = Builder->CreateInsertElement(Vec, valueCode, index);
      index++;
    }
    return Vec;
  case type_mat4:
    Vec = ConstantAggregateZero::get(
        VectorType::get(Type::getFloatTy(*TheContext), 16, false));
    index = 0;
    for (auto &value : args->getExpressions()) {
      auto valueCode = value->codegen();
      if (valueCode->getType()->isDoubleTy()) {
        valueCode =
            Builder->CreateFPTrunc(valueCode, Type::getFloatTy(*TheContext));
      } else if (valueCode->getType()->isIntegerTy()) {
        valueCode = ConstantExpr::getSIToFP((ConstantInt *)valueCode,
                                            Type::getFloatTy(*TheContext));
      } else if (valueCode->getType()->isPointerTy()) {
        // get the element type of the pointer
        auto elementType = getTypeFromAstType(value->getReturnType());
        valueCode = Builder->CreateLoad(elementType, valueCode);
        if (elementType->isIntegerTy()) {
          valueCode =
              Builder->CreateSIToFP(valueCode, Type::getFloatTy(*TheContext));
        } else if (elementType->isDoubleTy()) {
          valueCode =
              Builder->CreateFPTrunc(valueCode, Type::getFloatTy(*TheContext));
        }
      }
      Vec = Builder->CreateInsertElement(Vec, valueCode, index);
      index++;
    }
    return Vec;
  case type_error:
    break;
  }
}