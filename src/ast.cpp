//
// Created by jb030 on 13/05/2023.
//

#include "ast.h"

using namespace ast;

extern std::unique_ptr<LLVMContext> TheContext;
extern std::unique_ptr<Module> TheModule;
extern std::unique_ptr<IRBuilder<>> Builder;
extern std::map<std::string, Value *> NamedValues;

std::string ast::layoutTypeToString(LayoutType type) {
  switch (type) {
  case uniform:
    return "uniform";
  case in:
    return "in";
  case out:
    return "out";
  case empty:
    return "";
  }
  return "";
}

std::string ast::layoutIdentifierToString(LayoutIdentifier id) {
  switch (id) {
  case location:
    return "location";
  case binding:
    return "binding";
  }
  return "";
}

std::string ConditionalExpressionAST::toString() const {
  // toJson
  return R"({"type":"ConditionalExpressionAST","condition":)" +
         condition->toString() + ",\"then\":" + then->toString() +
         ",\"else\":" + else_->toString() + "}";
}

std::string BinaryExpressionAST::toString() const {
  return R"({"type":"BinaryExpressionAST","operator":")" +
         exprTypeToString(type) + R"(","left":)" + LHS->toString() +
         ",\"right\":" + RHS->toString() + "}";
}

std::string PrefixExpressionAST::toString() const {
  return R"({"type":"PrefixExpressionAST","operator":")" +
         exprTypeToString(type) + R"(","operand":)" + RHS->toString() + "}";
}

std::string PostfixExpressionAST::toString() const {
  return R"({"type":"PostfixExpressionAST","operator":")" +
         exprTypeToString(type) + R"(","identifier":")" + identifier +
         R"(","LHS":)" + LHS->toString() + "}";
}

std::string ExprListAST::toString() const {
  // change exprs vector to string
  std::string exprsString;
  for (auto &expr : expressions) {
    exprsString += expr->toString();
    exprsString += ",";
  }
  if (!exprsString.empty()) {
    exprsString.pop_back();
  }
  return R"({"type":"ExprListAST","exprs":[)" + exprsString + "]}";
}

std::string FunctionCallAST::toString() const {
  // toJson: callee ,args
  return R"({"type":"FunctionCallAST","callee":)" + callee +
         ",\"args\":" + args->toString() + "}";
}

std::string FunctionPrototypeAST::toString() const {
  // toJson: returnType, Name, Args
  //  change args vector to string
  std::string argsString;
  for (auto &arg : args) {
    argsString += arg->toString();
    argsString += ",";
  }
  if (!argsString.empty()) {
    argsString.pop_back();
  }

  return R"({"type":"FunctionPrototypeAST","returnAstType":")" +
         astTypeToString(returnType) + R"(","name":")" + name +
         R"(","args":[)" + argsString + "]}";
}

std::string SentencesAST::toString() const {
  // turn sentences vector to string
  std::string sentencesString;
  for (auto &sentence : sentences) {
    sentencesString += sentence->toString();
    sentencesString += ",";
  }
  if (!sentencesString.empty()) {
    sentencesString.pop_back();
  }
  return R"({"type":"SentencesAST","sentences":[)" + sentencesString + "]}";
}

std::string IfStatementAST::toString() const {
  if (else_ == nullptr) {
    return R"({"type":"IfStatementAST","condition":)" + condition->toString() +
           ",\"then\":" + then->toString() + "}";
  }
  return R"({"type":"IfStatementAST","condition":)" + condition->toString() +
         ",\"then\":" + then->toString() + ",\"else\":" + else_->toString() +
         "}";
}

std::string TypeConstructorAST::toString() const {
  return R"({"type":"TypeConstructorAST","astType":")" + astTypeToString(type) +
         R"(","args":)" + args->toString() + "}";
}

std::string WhileStatementAST::toString() const {
  // condition, body
  return R"({"type":"WhileStatementAST","condition":)" + condition->toString() +
         ",\"body\":" + body->toString() + "}";
}

std::string DoWhileStatementAST::toString() const {
  return R"({"type":"DoWhileStatementAST","condition":)" +
         condition->toString() + ",\"body\":" + body->toString() + "}";
}

std::string ForStatementAST::toString() const {
  return R"({"type":"ForStatementAST","init":)" + init->toString() +
         ",\"condition\":" + condition->toString() +
         ",\"step\":" + step->toString() + ",\"body\":" + body->toString() +
         "}";
}

std::string BreakStatementAST::toString() const {
  // only type
  return R"({"type":"BreakStatementAST"})";
}

std::string ContinueStatementAST::toString() const {
  // only type
  return R"({"type":"ContinueStatementAST"})";
}

std::string ReturnStatementAST::toString() const {
  if (expr == nullptr) {
    return R"({"type":"ReturnStatementAST"})";
  }
  return R"({"type":"ReturnStatementAST","expr":)" + expr->toString() + "}";
}

std::string NumberExprAST::toString() const {
  // valueType, value
  return R"({"type":"NumberExprAST","astType":")" + astTypeToString(type) +
         R"(","value":)" + value + "}";
}

std::string VariableExprAST::toString() const {
  // identifier
  return R"({"type":"VariableExprAST","identifier":")" + name + "\"}";
}

std::string VariableIndexExprAST::toString() const {
  // identifier, index
  return R"({"type":"VariableIndexExprAST","identifier":")" + name +
         R"(","index":)" + index->toString() + "}";
}

std::string FunctionDefinitionAST::toString() const {
  // prototype, body
  return R"({"type":"FunctionDefinitionAST","prototype":)" + Proto->toString() +
         ",\"body\":" + Body->toString() + "}";
}

std::string VariableDefinitionAST::toString() const {
  if (init == nullptr) {
    return R"({"type":"VariableDefinitionAST","astType":")" +
           astTypeToString(type) + R"(","isConst":)" +
           (isConst ? "true" : "false") + R"(,"name":")" + name + "\"}";
  }
  // type, isConst, name, init
  return R"({"type":"VariableDefinitionAST","astType":")" +
         astTypeToString(type) + R"(","isConst":)" +
         (isConst ? "true" : "false") + R"(,"name":")" + name + R"(","init":)" +
         init->toString() + "}";
}

std::string LayoutAst::toString() const {
  // toJson: type, layoutQualifier
  return R"({"type":"LayoutAst","astType":")" + layoutTypeToString(type) +
         R"(","layoutQualifier":)" + layoutQualifier->toString() + "}";
}

std::string TopLevelAST::toString() const {
  // version, definitions
  std::string definitionsString = "";
  for (auto &definition : *definitions) {
    definitionsString = definitionsString + definition->toString();
    definitionsString += ",";
  }
  if (!definitionsString.empty()) {
    definitionsString.pop_back();
  }
  // uint64_t version;
  return R"({"type":"TopLevelAST","version":)" + std::to_string(version) +
         ",\"definitions\":[" + definitionsString + "]}";
}
std::string FunctionArgumentAST::toString() const {
  // type, name
  return R"({"type":"FunctionArgumentAST","astType":")" +
         astTypeToString(type) + R"(","name":")" + name + "\"}";
}
std::string SentenceAST::toString() const { return std::string(); }
std::string DefinitionAST::toString() const { return std::string(); }
std::string LayoutQualifierIdAST::toString() const {
  // toJson: id, value
  // int value;
  return R"({"type":"LayoutQualifierIdAST","id":")" +
         layoutIdentifierToString(id) + R"(","value":)" +
         std::to_string(value) + "}";
}
std::string GlobalVariableDefinitionAST::toString() const {
  if (init == nullptr && layout == nullptr) {
    return R"({"type":"GlobalVariableDefinitionAST","astType":")" +
           astTypeToString(type) + R"(","isConst":)" +
           (isConst ? "true" : "false") + R"(,"name":")" + name +
           R"(","init":null,"layout":null})";
  }
  if (init == nullptr) {
    return R"({"type":"GlobalVariableDefinitionAST","astType":")" +
           astTypeToString(type) + R"(","isConst":)" +
           (isConst ? "true" : "false") + R"(,"name":")" + name +
           R"(","init":null,"layout":)" + layout->toString() + "}";
  }
  if (layout == nullptr) {
    return R"({"type":"GlobalVariableDefinitionAST","astType":")" +
           astTypeToString(type) + R"(","isConst":)" +
           (isConst ? "true" : "false") + R"(,"name":")" + name +
           R"(","init":)" + init->toString() + ",\"layout\":null}";
  }
  return R"({"type":"GlobalVariableDefinitionAST","astType":")" +
         astTypeToString(type) + R"(","isConst":)" +
         (isConst ? "true" : "false") + R"(,"name":")" + name + R"(","init":)" +
         init->toString() + ",\"layout\":" + layout->toString() + "}";
}

std::string LayoutQualifierAst::toString() const {
  // toJson: ids
  std::string idsString;
  for (auto &id : *ids) {
    idsString += id->toString();
    idsString += ",";
  }
  if (!idsString.empty()) {
    idsString.pop_back();
  }
  return R"({"type":"LayoutQualifierAst","ids":[)" + idsString + "]}";
}

std::string SequenceExpressionAST::toString() const {
  // toJson: expressions
  std::string expressionsString;
  for (auto &expression : expressions) {
    expressionsString += expression->toString();
    expressionsString += ",";
  }
  if (!expressionsString.empty()) {
    expressionsString.pop_back();
  }
  return R"({"type":"SequenceExpressionAST","expressions":[)" +
         expressionsString + "]}";
}
