//
// Created by jb030 on 13/05/2023.
//

#include "ast.h"
#include "log.h"
#include "parser.h"

using namespace ast;

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

Value *ConditionalExpressionAST::codegen() { return nullptr; }
std::string ConditionalExpressionAST::toString() const {
  // toJson
  return R"({"type":"ConditionalExpressionAST","condition":)" +
         condition->toString() + ",\"then\":" + then->toString() +
         ",\"else\":" + else_->toString() + "}";
}
Value *BinaryExpressionAST::codegen() { return nullptr; }
std::string BinaryExpressionAST::toString() const {
  return R"({"type":"BinaryExpressionAST","operator":")" +
         exprTypeToString(type) + R"(","left":)" + LHS->toString() +
         ",\"right\":" + RHS->toString() + "}";
}
Value *PrefixExpressionAST::codegen() { return nullptr; }
std::string PrefixExpressionAST::toString() const {
  return R"({"type":"PrefixExpressionAST","operator":")" +
         exprTypeToString(type) + R"(","operand":)" + RHS->toString() + "}";
}
Value *PostfixExpressionAST::codegen() { return nullptr; }
std::string PostfixExpressionAST::toString() const {
  return R"({"type":"PostfixExpressionAST","operator":")" +
         exprTypeToString(type) + R"(","identifier":")" + identifier +
         R"(","LHS":)" + LHS->toString() + "}";
}
Value *ExprListAST::codegen() { return nullptr; }
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
Value *FunctionCallAST::codegen() { return nullptr; }
std::string FunctionCallAST::toString() const {
  // toJson: callee ,args
  return R"({"type":"FunctionCallAST","callee":)" + callee +
         ",\"args\":" + args->toString() + "}";
}
Function *FunctionPrototypeAST::codegen() { return nullptr; }
std::string FunctionPrototypeAST::toString() const {
  // toJson: returnType, Name, Args
  //  change args vector to string
  std::string argsString;
  for (auto &arg : Args) {
    argsString += arg->toString();
    argsString += ",";
  }
  if (!argsString.empty()) {
    argsString.pop_back();
  }

  return R"({"type":"FunctionPrototypeAST","returnType":")" +
         astTypeToString(returnType) + R"(","name":")" + Name +
         R"(","args":[)" + argsString + "]}";
}
Value *SentencesAST::codegen() { return nullptr; }
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
Value *IfStatementAST::codegen() { return nullptr; }
std::string IfStatementAST::toString() const {
  return R"({"type":"IfStatementAST","condition":)" + condition->toString() +
         ",\"then\":" + then->toString() + ",\"else\":" + else_->toString() +
         "}";
}
Value *ArrayAccessAST::codegen() { return nullptr; }
std::string ArrayAccessAST::toString() const {
  return R"({"type":"ArrayAccessAST","identifier":")" + identifier +
         R"(","index":)" + index->toString() + "}";
}
Value *TypeConstructorAST::codegen() { return nullptr; }
std::string TypeConstructorAST::toString() const {
  return R"({"type":"TypeConstructorAST","type":")" + astTypeToString(type) +
         R"(","args":)" + args->toString() + "}";
}
Value *WhileStatementAST::codegen() { return nullptr; }
std::string WhileStatementAST::toString() const {
  // condition, body
  return R"({"type":"WhileStatementAST","condition":)" + condition->toString() +
         ",\"body\":" + body->toString() + "}";
}
Value *DoWhileStatementAST::codegen() { return nullptr; }
std::string DoWhileStatementAST::toString() const {
  return R"({"type":"DoWhileStatementAST","condition":)" +
         condition->toString() + ",\"body\":" + body->toString() + "}";
}
Value *ForStatementAST::codegen() { return nullptr; }
std::string ForStatementAST::toString() const {
  return R"({"type":"ForStatementAST","init":)" + init->toString() +
         ",\"condition\":" + condition->toString() +
         ",\"step\":" + step->toString() + ",\"body\":" + body->toString() +
         "}";
}
Value *BreakStatementAST::codegen() { return nullptr; }
std::string BreakStatementAST::toString() const {
  // only type
  return R"({"type":"BreakStatementAST"})";
}
Value *ContinueStatementAST::codegen() { return nullptr; }
std::string ContinueStatementAST::toString() const {
  // only type
  return R"({"type":"ContinueStatementAST"})";
}
Value *ReturnStatementAST::codegen() { return nullptr; }
std::string ReturnStatementAST::toString() const {
  if (expr == nullptr) {
    return R"({"type":"ReturnStatementAST"})";
  }
  return R"({"type":"ReturnStatementAST","expr":)" + expr->toString() + "}";
}
Value *NumberExprAST::codegen() { return nullptr; }
std::string NumberExprAST::toString() const {
  // valueType, value
  return R"({"type":"NumberExprAST","valueType":")" + astTypeToString(type) +
         R"(","value":)" + value + "}";
}
Value *VariableExprAST::codegen() { return nullptr; }
std::string VariableExprAST::toString() const {
  // identifier
  return R"({"type":"VariableExprAST","identifier":")" + name + "\"}";
}
Value *VariableIndexExprAST::codegen() { return nullptr; }
std::string VariableIndexExprAST::toString() const {
  // identifier, index
  return R"({"type":"VariableIndexExprAST","identifier":")" + name +
         R"(","index":)" + index->toString() + "}";
}
Function *FunctionDefinitionAST::codegen() { return nullptr; }
std::string FunctionDefinitionAST::toString() const {
  // prototype, body
  return R"({"type":"FunctionDefinitionAST","prototype":)" + Proto->toString() +
         ",\"body\":" + Body->toString() + "}";
}
Value *VariableDefinitionAST::codegen() { return nullptr; }
std::string VariableDefinitionAST::toString() const {
  if (init == nullptr) {
    return R"({"type":"VariableDefinitionAST","type":")" +
           astTypeToString(type) + R"(","isConst":)" +
           (isConst ? "true" : "false") + R"(,"name":")" + name + "\"}";
  }
  // type, isConst, name, init
  return R"({"type":"VariableDefinitionAST","type":")" + astTypeToString(type) +
         R"(","isConst":)" + (isConst ? "true" : "false") + R"(,"name":")" +
         name + R"(","init":)" + init->toString() + "}";
}
Value *LayoutAst::codegen() { return nullptr; }
std::string LayoutAst::toString() const {
  // toJson: type, layoutQualifier
  return R"({"type":"LayoutAst","type":")" + layoutTypeToString(type) +
         R"(","layoutQualifier":)" + layoutQualifier->toString() + "}";
}
Value *TopLevelAST::codegen() { return nullptr; }
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
  return R"({"type":"FunctionArgumentAST","type":")" + astTypeToString(type) +
         R"(","name":")" + name + "\"}";
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
    return R"({"type":"GlobalVariableDefinitionAST","type":")" +
           astTypeToString(type) + R"(","isConst":)" +
           (isConst ? "true" : "false") + R"(,"name":")" + name +
           R"(","init":null,"layout":null})";
  }
  if (init == nullptr) {
    return R"({"type":"GlobalVariableDefinitionAST","type":")" +
           astTypeToString(type) + R"(","isConst":)" +
           (isConst ? "true" : "false") + R"(,"name":")" + name +
           R"(","init":null,"layout":)" + layout->toString() + "}";
  }
  if (layout == nullptr) {
    return R"({"type":"GlobalVariableDefinitionAST","type":")" +
           astTypeToString(type) + R"(","isConst":)" +
           (isConst ? "true" : "false") + R"(,"name":")" + name +
           R"(","init":)" + init->toString() + ",\"layout\":null}";
  }
  return R"({"type":"GlobalVariableDefinitionAST","type":")" +
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
