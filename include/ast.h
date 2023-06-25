#ifndef LLVM_AST_H

#include <utility>

#include "global.h"
#include "scope.h"
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

extern std::shared_ptr<Scope> currentScope;

namespace ast {

class AST {
public:
  virtual ~AST() = default;

  virtual Value *codegen() = 0;
  virtual std::string toString() const = 0;
};

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

class FunctionPrototypeAST : public AST {
  AstType returnType;
  std::string name;
  std::vector<std::unique_ptr<FunctionArgumentAST>> args;

public:
  FunctionPrototypeAST(AstType returnType, std::string Name,
                       std::vector<std::unique_ptr<FunctionArgumentAST>> Args)
      : returnType(returnType), name(std::move(Name)), args(std::move(Args)) {}

  Function *codegen() override;
  const std::string &getName() const { return name; }
  AstType getReturnType() const { return returnType; }
  const std::vector<std::unique_ptr<FunctionArgumentAST>> &getArgs() const {
    return args;
  }
  std::string toString() const override;
};

class SentenceAST : public AST {
public:
  virtual ~SentenceAST() = default;

  Value *codegen() override = 0;
  std::string toString() const override;
  virtual bool isReturn() const { return false; }
};

class EmptySentenceAST : public SentenceAST {
public:
  EmptySentenceAST() = default;

  Value *codegen() override;
  std::string toString() const override;
  bool isReturn() const override { return false; }
};

class SentencesAST : public SentenceAST {
  std::vector<std::unique_ptr<SentenceAST>> sentences;

public:
  explicit SentencesAST(std::vector<std::unique_ptr<SentenceAST>> sentences)
      : sentences(std::move(sentences)) {}

  Value *codegen() override;
  std::string toString() const override;

  std::vector<std::unique_ptr<SentenceAST>> &getSentences() {
    return sentences;
  }
  bool isReturn() const override {
    for (auto &sentence : sentences) {
      if (sentence->isReturn())
        return true;
    }
    return false;
  }
};

class ExpressionAST : public SentenceAST {
protected:
  AstType returnType = type_error;

public:
  ~ExpressionAST() override = default;

  void setReturnType(AstType type) { returnType = type; }
  virtual AstType getReturnType() const { return returnType; }

  Value *codegen() override = 0;
  bool isReturn() const override { return false; }
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

  AstType getReturnType() const override {
    if (returnType != type_error)
      return returnType;
    if (then != nullptr && else_ != nullptr)
      return then->getReturnType();
  }

  Value *codegen() override;

  std::string toString() const override;

  ~ConditionalExpressionAST() override = default;
  bool isReturn() const override {
    return then->isReturn() && else_->isReturn();
  }
};

class BinaryExpressionAST : public ExpressionAST {
  ExprType type;
  std::unique_ptr<ExpressionAST> LHS, RHS;

public:
  BinaryExpressionAST(ExprType type, std::unique_ptr<ExpressionAST> LHS,
                      std::unique_ptr<ExpressionAST> RHS)
      : type(type), LHS(std::move(LHS)), RHS(std::move(RHS)) {}

  AstType getReturnType() const override {
    if (returnType != type_error)
      return returnType;
    if (LHS != nullptr && RHS != nullptr)
      return LHS->getReturnType();
  }

  Value *codegen() override;

  std::string toString() const override;

  ExprType getType() const { return type; }
  std::unique_ptr<ExpressionAST> &getLHS() { return LHS; }
  std::unique_ptr<ExpressionAST> &getRHS() { return RHS; }

  ~BinaryExpressionAST() override = default;

  bool isReturn() const override { return LHS->isReturn() && RHS->isReturn(); }
};

class PrefixExpressionAST : public ExpressionAST {
  ExprType type;
  std::unique_ptr<ExpressionAST> RHS;

public:
  PrefixExpressionAST(ExprType type, std::unique_ptr<ExpressionAST> RHS)
      : type(type), RHS(std::move(RHS)) {}

  AstType getReturnType() const override {
    switch (type) {
    case plus_p_expr:
    case minus_m_expr:
    case plus_expr:
    case minus_expr:
    case tilde_expr:
      return RHS->getReturnType();
      break;
    case not_expr:
      return type_int;
    default:
      return type_error;
    }
  }

  Value *codegen() override;

  std::string toString() const override;

  ~PrefixExpressionAST() override = default;

  bool isReturn() const override { return RHS->isReturn(); }
};

class PostfixExpressionAST : public ExpressionAST {
  ExprType type;
  std::unique_ptr<ExpressionAST> LHS;
  std::string identifier;

public:
  PostfixExpressionAST(ExprType type, std::unique_ptr<ExpressionAST> LHS)
      : type(type), LHS(std::move(LHS)) {}

  AstType getReturnType() const override {
    switch (type) {
    case plus_p_expr:
    case minus_m_expr:
      return type_int;
    case dot_expr:
      return type_float;
    default:
      return type_error;
    }
  }

  void setIdentifier(const std::string &identifier) {
    this->identifier = identifier;
  }
  Value *codegen() override;

  std::string toString() const override;

  bool isReturn() const override { return LHS->isReturn(); }

  ~PostfixExpressionAST() override = default;
};

class SequenceExpressionAST : public ExpressionAST {
  std::vector<std::unique_ptr<ExpressionAST>> expressions;

public:
  explicit SequenceExpressionAST(
      std::vector<std::unique_ptr<ExpressionAST>> expressions)
      : expressions(std::move(expressions)) {}

  AstType getReturnType() const override {
    if (returnType != type_error)
      return returnType;
    if (expressions.size() > 0)
      return expressions.back()->getReturnType();
    return type_error;
  }

  bool isReturn() const override {
    for (auto &expression : expressions) {
      if (expression->isReturn())
        return true;
    }
    return false;
  }

  explicit SequenceExpressionAST() {
    expressions = std::vector<std::unique_ptr<ExpressionAST>>();
  }

  Value *codegen() override;

  Value *getArgs();

  std::vector<std::unique_ptr<ExpressionAST>> &getExpressions() {
    return expressions;
  }

  std::string toString() const override;

  ~SequenceExpressionAST() override = default;
};

class ExprListAST : public ExpressionAST {
  std::vector<std::unique_ptr<ExpressionAST>> expressions;

public:
  explicit ExprListAST(std::vector<std::unique_ptr<ExpressionAST>> expressions)
      : expressions(std::move(expressions)) {}

  AstType getReturnType() const override {
    if (returnType != type_error)
      return returnType;
    if (expressions.size() > 0)
      return expressions.back()->getReturnType();
  }

  bool isReturn() const override {
    for (auto &expression : expressions) {
      if (expression->isReturn())
        return true;
    }
    return false;
  }

  explicit ExprListAST() {
    expressions = std::vector<std::unique_ptr<ExpressionAST>>();
  }

  std::vector<std::unique_ptr<ExpressionAST>> &getExpressions() {
    return expressions;
  }

  Value *codegen() override;

  std::string toString() const;

  ~ExprListAST() = default;
};

class FunctionCallAST : public ExpressionAST {
  std::string callee;
  std::unique_ptr<ExprListAST> args;

public:
  FunctionCallAST(std::string callee, std::unique_ptr<ExprListAST> args)
      : callee(std::move(callee)), args(std::move(args)) {}

  AstType getReturnType() const override {
    if (returnType != type_error)
      return returnType;
    if (args != nullptr)
      return args->getReturnType();
  }

  bool isReturn() const override {
    if (args != nullptr)
      return args->isReturn();
    return false;
  }

  Value *codegen() override;

  std::string toString() const override;

  ~FunctionCallAST() override = default;
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

  bool isReturn() const override {
    if (then != nullptr && else_ != nullptr)
      return then->isReturn() && else_->isReturn();
    return false;
  }

  ~IfStatementAST() override = default;
};

class TypeConstructorAST : public ExpressionAST {
  AstType type;
  std::unique_ptr<ExprListAST> args;

public:
  TypeConstructorAST(AstType type, std::unique_ptr<ExprListAST> args)
      : type(type), args(std::move(args)) {
    setReturnType(type);
  }

  std::unique_ptr<ExprListAST> &getArgs() { return args; }

  bool isReturn() const override {
    if (args != nullptr)
      return args->isReturn();
    return false;
  }

  Value *codegen() override;

  std::string toString() const;

  ~TypeConstructorAST() = default;
};

class ForStatementAST : public SentenceAST {
  std::unique_ptr<SentenceAST> init;
  std::unique_ptr<ExpressionAST> condition;
  std::unique_ptr<ExpressionAST> step;
  std::unique_ptr<SentenceAST> body;

public:
  ForStatementAST(std::unique_ptr<SentenceAST> init,
                  std::unique_ptr<ExpressionAST> condition,
                  std::unique_ptr<ExpressionAST> step,
                  std::unique_ptr<SentenceAST> body)
      : init(std::move(init)), condition(std::move(condition)),
        step(std::move(step)), body(std::move(body)) {}

  Value *codegen() override;

  bool isReturn() const override {
    if (body != nullptr)
      return body->isReturn();
    return false;
  }

  std::string toString() const override;

  ~ForStatementAST() override = default;
};

class ReturnStatementAST : public SentenceAST {
  std::unique_ptr<ExpressionAST> expr;

public:
  explicit ReturnStatementAST(std::unique_ptr<ExpressionAST> expr)
      : expr(std::move(expr)) {}

  // void return
  ReturnStatementAST() : expr(nullptr) {}

  bool isReturn() const { return true; }

  Value *codegen() override;

  std::string toString() const override;

  ~ReturnStatementAST() override = default;
};

class NumberExprAST : public ExpressionAST {
  std::string value;
  AstType type;

public:
  NumberExprAST(std::string value, AstType type)
      : value(std::move(value)), type(type) {
    setReturnType(type);
  }

  AstType getReturnType() const override { return type; }

  AstType getType() const { return type; }
  std::string getValue() const { return value; }
  Value *codegen() override;

  bool isReturn() const override { return false; }

  std::string toString() const override;

  ~NumberExprAST() override = default;
};

class VariableExprAST : public ExpressionAST {
  std::string name;

public:
  explicit VariableExprAST(std::string name) : name(std::move(name)) {}

  AstType getReturnType() const {
    return currentScope->getIndentifier(name)->first;
  }

  Value *codegen() override;

  bool isReturn() const override { return false; }

  std::string toString() const override;

  std::string getName() const { return name; }

  ~VariableExprAST() override = default;
};

class VariableIndexExprAST : public ExpressionAST {
  std::string name;
  std::unique_ptr<ExpressionAST> index;

public:
  VariableIndexExprAST(std::string name, std::unique_ptr<ExpressionAST> index)
      : name(std::move(name)), index(std::move(index)) {}

  AstType getReturnType() const override {
    if (returnType != type_error) {
      return returnType;
    }
    switch (currentScope->getIndentifier(name)->first) {
    case type_mat2:
      return type_vec2;
    case type_mat3:
      return type_vec3;
    case type_mat4:
      return type_vec4;
    default:
      return type_error;
    }
  }

  Value *codegen() override;

  bool isReturn() const override { return false; }

  std::string toString() const override;

  ~VariableIndexExprAST() override = default;
};

class DefinitionAST : public AST {
public:
  virtual ~DefinitionAST() = default;

  Value *codegen() override = 0;

  std::string toString() const override;
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
  void checkAndInsertVoidReturn(Function *);
};

class VariableDefinitionAST : public DefinitionAST, public SentenceAST {

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

  bool isReturn() const override { return false; }

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

class LayoutAst : public AST {
  LayoutType type;
  std::unique_ptr<LayoutQualifierAst> layoutQualifier;

public:
  LayoutAst(LayoutType type,
            std::unique_ptr<LayoutQualifierAst> layoutQualifier)
      : type(type), layoutQualifier(std::move(layoutQualifier)) {}
  ~LayoutAst() = default;
  Value *codegen() override;
  std::string toString() const override;
};

class GlobalVariableDefinitionAST : public VariableDefinitionAST {
  std::unique_ptr<LayoutAst> layout;

public:
  GlobalVariableDefinitionAST(AstType type, bool isConst, std::string name,
                              std::unique_ptr<ExpressionAST> init,
                              std::unique_ptr<LayoutAst> layout)
      : VariableDefinitionAST(type, isConst, std::move(name), std::move(init)),
        layout(std::move(layout)) {}

  Value *codegen() override;

  bool isReturn() const override { return false; }

  std::string toString() const override;
};

class TopLevelAST : public AST {
  uint64_t version;
  std::unique_ptr<std::vector<std::unique_ptr<DefinitionAST>>> definitions;

public:
  TopLevelAST(
      uint64_t version,
      std::unique_ptr<std::vector<std::unique_ptr<DefinitionAST>>> definitions)
      : version(version), definitions(std::move(definitions)) {}
  Value *codegen() override;

  std::string toString() const override;
};
} // namespace ast
#define LLVM_AST_H

#endif // LLVM_AST_H
