//
// Created by jb030 on 13/05/2023.
//

#ifndef LLVM_AST_H

#include <utility>

#include "global.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

using namespace llvm;

namespace ast {

class FunctionArgumentAST {
  AstType type;
  std::string name;

public:
  FunctionArgumentAST(AstType type, std::string name)
      : type(type), name(std::move(name)) {}

  const std::string &getName() const { return name; }
  AstType getType() const { return type; }
  // toString
  std::string toString() const;
};

class FunctionPrototypeAST {
  AstType returnType;
  std::string Name;
  std::vector<std::unique_ptr<FunctionArgumentAST>> Args;

public:
  FunctionPrototypeAST(AstType returnType, std::string Name,
                       std::vector<std::unique_ptr<FunctionArgumentAST>> Args)
      : returnType(returnType), Name(std::move(Name)), Args(std::move(Args)) {}

  Function *codegen();
  const std::string &getName() const { return Name; }
  std::string toString() const;
};

class SentenceAST {
public:
  virtual ~SentenceAST() = default;

  virtual Value *codegen() = 0;
  virtual std::string toString() const;
};

class SentencesAST : public SentenceAST {
  std::vector<std::unique_ptr<SentenceAST>> sentences;

public:
  explicit SentencesAST(std::vector<std::unique_ptr<SentenceAST>> sentences)
      : sentences(std::move(sentences)) {}

  Value *codegen() override;
  std::string toString() const override;
};

class ExpressionAST : public SentenceAST {
public:
  ~ExpressionAST() override = default;

  Value *codegen() override = 0;
};

class ConditionalExpressionAST : public ExpressionAST {
  std::unique_ptr<ExpressionAST> condition;
  std::unique_ptr<ExpressionAST> then;
  std::unique_ptr<ExpressionAST> else_;

public:
  ConditionalExpressionAST(std::unique_ptr<ExpressionAST> condition,
                           std::unique_ptr<ExpressionAST> then,
                           std::unique_ptr<ExpressionAST> else_)
      : condition(std::move(condition)), then(std::move(then)),
        else_(std::move(else_)) {}

  Value *codegen() override;

  std::string toString() const override;

  ~ConditionalExpressionAST() override = default;
};

class BinaryExpressionAST : public ExpressionAST {
  ExprType type;
  std::unique_ptr<ExpressionAST> LHS, RHS;

public:
  BinaryExpressionAST(ExprType type, std::unique_ptr<ExpressionAST> LHS,
                      std::unique_ptr<ExpressionAST> RHS)
      : type(type), LHS(std::move(LHS)), RHS(std::move(RHS)) {}

  Value *codegen() override;

  std::string toString() const override;

  ~BinaryExpressionAST() override = default;
};

class PrefixExpressionAST : public ExpressionAST {
  ExprType type;
  std::unique_ptr<ExpressionAST> RHS;

public:
  PrefixExpressionAST(ExprType type, std::unique_ptr<ExpressionAST> RHS)
      : type(type), RHS(std::move(RHS)) {}

  Value *codegen() override;

  std::string toString() const override;

  ~PrefixExpressionAST() override = default;
};

class PostfixExpressionAST : public ExpressionAST {
  ExprType type;
  std::unique_ptr<ExpressionAST> LHS;
  std::string identifier;

public:
  PostfixExpressionAST(ExprType type, std::unique_ptr<ExpressionAST> LHS)
      : type(type), LHS(std::move(LHS)) {}

  void setIdentifier(const std::string &identifier) {
    this->identifier = identifier;
  }
  Value *codegen() override;

  std::string toString() const override;

  ~PostfixExpressionAST() override = default;
};

class ExprListAST : public ExpressionAST {
  std::vector<std::unique_ptr<ExpressionAST>> expressions;

public:
  explicit ExprListAST(std::vector<std::unique_ptr<ExpressionAST>> expressions)
      : expressions(std::move(expressions)) {}

  Value *codegen() override;

  std::string toString() const override;

  ~ExprListAST() override = default;
};

class FunctionCallAST : public ExpressionAST {
  std::string callee;
  std::unique_ptr<ExprListAST> args;

public:
  FunctionCallAST(std::string callee, std::unique_ptr<ExprListAST> args)
      : callee(std::move(callee)), args(std::move(args)) {}

  Value *codegen() override;

  std::string toString() const override;

  ~FunctionCallAST() override = default;
};

class ArrayAccessAST : public ExpressionAST {
  std::string identifier;
  std::unique_ptr<ExpressionAST> index;

public:
  ArrayAccessAST(std::string identifier, std::unique_ptr<ExpressionAST> index)
      : identifier(std::move(identifier)), index(std::move(index)) {}

  Value *codegen() override;

  std::string toString() const override;

  ~ArrayAccessAST() override = default;
};

class IfStatementAST : public SentenceAST {
  std::unique_ptr<ExpressionAST> condition;
  std::unique_ptr<SentenceAST> then;
  std::unique_ptr<SentenceAST> else_;

public:
  IfStatementAST(std::unique_ptr<ExpressionAST> condition,
                 std::unique_ptr<SentenceAST> then,
                 std::unique_ptr<SentenceAST> else_)
      : condition(std::move(condition)), then(std::move(then)),
        else_(std::move(else_)) {}

  Value *codegen() override;

  std::string toString() const override;

  ~IfStatementAST() override = default;
};

class TypeConstructorAST : public ExpressionAST {
  AstType type;
  std::unique_ptr<ExprListAST> args;

public:
  TypeConstructorAST(AstType type, std::unique_ptr<ExprListAST> args)
      : type(type), args(std::move(args)) {}

  Value *codegen() override;

  std::string toString() const override;

  ~TypeConstructorAST() override = default;
};

class WhileStatementAST : public SentenceAST {
  std::unique_ptr<ExpressionAST> condition;
  std::unique_ptr<SentenceAST> body;

public:
  WhileStatementAST(std::unique_ptr<ExpressionAST> condition,
                    std::unique_ptr<SentenceAST> body)
      : condition(std::move(condition)), body(std::move(body)) {}

  Value *codegen() override;

  std::string toString() const override;

  ~WhileStatementAST() override = default;
};

class DoWhileStatementAST : public SentenceAST {
  std::unique_ptr<ExpressionAST> condition;
  std::unique_ptr<SentenceAST> body;

public:
  DoWhileStatementAST(std::unique_ptr<ExpressionAST> condition,
                      std::unique_ptr<SentenceAST> body)
      : condition(std::move(condition)), body(std::move(body)) {}

  Value *codegen() override;

  std::string toString() const override;

  ~DoWhileStatementAST() override = default;
};

class ForStatementAST : public SentenceAST {
  std::unique_ptr<ExpressionAST> init;
  std::unique_ptr<ExpressionAST> condition;
  std::unique_ptr<ExpressionAST> step;
  std::unique_ptr<SentenceAST> body;

public:
  ForStatementAST(std::unique_ptr<ExpressionAST> init,
                  std::unique_ptr<ExpressionAST> condition,
                  std::unique_ptr<ExpressionAST> step,
                  std::unique_ptr<SentenceAST> body)
      : init(std::move(init)), condition(std::move(condition)),
        step(std::move(step)), body(std::move(body)) {}

  Value *codegen() override;

  std::string toString() const override;

  ~ForStatementAST() override = default;
};

class BreakStatementAST : public SentenceAST {
public:
  BreakStatementAST() = default;

  Value *codegen() override;

  std::string toString() const override;

  ~BreakStatementAST() override = default;
};

class ContinueStatementAST : public SentenceAST {
public:
  ContinueStatementAST() = default;

  Value *codegen() override;

  std::string toString() const override;

  ~ContinueStatementAST() override = default;
};

class ReturnStatementAST : public SentenceAST {
  std::unique_ptr<ExpressionAST> expr;

public:
  explicit ReturnStatementAST(std::unique_ptr<ExpressionAST> expr)
      : expr(std::move(expr)) {}

  // void return
  ReturnStatementAST() : expr(nullptr) {}

  Value *codegen() override;

  std::string toString() const override;

  ~ReturnStatementAST() override = default;
};

class NumberExprAST : public ExpressionAST {
  std::string value;
  AstType type;

public:
  NumberExprAST(std::string value, AstType type)
      : value(std::move(value)), type(type) {}
  AstType getType() const { return type; }
  std::string getValue() const { return value; }
  Value *codegen() override;

  std::string toString() const override;

  ~NumberExprAST() override = default;
};

class VariableExprAST : public ExpressionAST {
  std::string name;

public:
  explicit VariableExprAST(std::string name) : name(std::move(name)) {}
  Value *codegen() override;

  std::string toString() const override;

  ~VariableExprAST() override = default;
};

class VariableIndexExprAST : public ExpressionAST {
  std::string name;
  std::unique_ptr<ExpressionAST> index;

public:
  VariableIndexExprAST(std::string name, std::unique_ptr<ExpressionAST> index)
      : name(std::move(name)), index(std::move(index)) {}
  Value *codegen() override;

  std::string toString() const override;

  ~VariableIndexExprAST() override = default;
};

class DefinitionAST {
public:
  virtual ~DefinitionAST() = default;

  virtual Value *codegen() = 0;

  virtual std::string toString() const;
};

class FunctionDefinitionAST : public DefinitionAST {
  std::unique_ptr<FunctionPrototypeAST> Proto;
  // vector of sentences
  std::unique_ptr<SentencesAST> Body;

public:
  FunctionDefinitionAST(std::unique_ptr<FunctionPrototypeAST> Proto,
                        std::unique_ptr<SentencesAST> Body)
      : Proto(std::move(Proto)), Body(std::move(Body)) {}

  FunctionDefinitionAST(AstType type, const std::string &name,
                        std::vector<std::unique_ptr<FunctionArgumentAST>> args,
                        std::unique_ptr<SentencesAST> Body)
      : Proto(std::make_unique<FunctionPrototypeAST>(type, name,
                                                     std::move(args))),
        Body(std::move(Body)) {}

  Function *codegen() override;

  std::string toString() const override;

  ~FunctionDefinitionAST() override = default;
};

class VariableDefinitionAST : public DefinitionAST, SentenceAST {

protected:
  AstType type;
  bool isConst = false;
  std::string name;
  std::unique_ptr<ExpressionAST> init;

public:
  VariableDefinitionAST(AstType type, bool isConst, std::string name,
                        std::unique_ptr<ExpressionAST> init)
      : type(type), isConst(isConst), name(std::move(name)),
        init(std::move(init)) {}
  Value *codegen() override;

  std::string toString() const override;

  ~VariableDefinitionAST() override = default;
};

enum LayoutType {
  uniform = -1,
  in = -2,
  out = -3,
  empty = -4,
};

std::string layoutTypeToString(LayoutType type);

enum LayoutIdentifier {
  location = -1,
  binding = -2,
};

std::string layoutIdentifierToString(LayoutIdentifier id);

class LayoutQualifierIdAST {
  LayoutIdentifier id;
  int value = -1;

public:
  LayoutQualifierIdAST(LayoutIdentifier id, int value) : id(id), value(value) {}
  explicit LayoutQualifierIdAST(LayoutIdentifier id) : id(id) {}

  std::string toString() const;
};

class LayoutQualifierAst {
  std::unique_ptr<std::vector<std::unique_ptr<LayoutQualifierIdAST>>> ids;

public:
  explicit LayoutQualifierAst(
      std::unique_ptr<std::vector<std::unique_ptr<LayoutQualifierIdAST>>> ids)
      : ids(std::move(ids)) {}

  std::string toString() const;
};

class LayoutAst {
  LayoutType type;
  std::unique_ptr<LayoutQualifierAst> layoutQualifier;

public:
  LayoutAst(LayoutType type,
            std::unique_ptr<LayoutQualifierAst> layoutQualifier)
      : type(type), layoutQualifier(std::move(layoutQualifier)) {}
  ~LayoutAst() = default;
  Value *codegen();
  std::string toString() const;
};

class GlobalVariableDefinitionAST : public VariableDefinitionAST {
  std::unique_ptr<LayoutAst> layout;

public:
  GlobalVariableDefinitionAST(AstType type, bool isConst, std::string name,
                              std::unique_ptr<ExpressionAST> init,
                              std::unique_ptr<LayoutAst> layout)
      : VariableDefinitionAST(type, isConst, std::move(name), std::move(init)),
        layout(std::move(layout)) {}

  std::string toString() const override;
};

class TopLevelAST {
  uint64_t version;
  std::unique_ptr<std::vector<std::unique_ptr<DefinitionAST>>> definitions;

public:
  TopLevelAST(
      uint64_t version,
      std::unique_ptr<std::vector<std::unique_ptr<DefinitionAST>>> definitions)
      : version(version), definitions(std::move(definitions)) {}
  Value *codegen();

  std::string toString() const;
};
} // namespace ast

#define LLVM_AST_H

#endif // LLVM_AST_H
