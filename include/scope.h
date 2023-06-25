#ifndef LLVM_SCOPE_H
#define LLVM_SCOPE_H

#include "global.h"
#include "llvm/IR/Constants.h"
#include <map>
#include <string>

using namespace llvm;

class Scope {
  std::map<std::string, std::shared_ptr<std::pair<AstType, Value *>>>
      namedValues = {};
  std::shared_ptr<Scope> parent;

public:
  ~Scope() = default;

  Scope() : parent(nullptr){};
  explicit Scope(Scope *parent) : parent(parent) {}
  explicit Scope(const std::shared_ptr<Scope> &parent) : parent(parent) {}

  void addIndentifier(const std::string &name, AstType type, Value *value) {
    namedValues[name] =
        std::make_shared<std::pair<AstType, Value *>>(type, value);
  }

  void setIndentifierValue(const std::string &name, Value *value) {
    if (namedValues.find(name) != namedValues.end()) {
      namedValues[name]->second = value;
    } else if (parent != nullptr) {
      parent->setIndentifierValue(name, value);
    }
  }

  // getParent
  std::shared_ptr<Scope> getParent() { return parent; }

  std::shared_ptr<std::pair<AstType, Value *>>
  getIndentifier(const std::string &name) {
    if (namedValues.find(name) != namedValues.end()) {
      return namedValues[name];
    } else if (parent != nullptr) {
      return parent->getIndentifier(name);
    } else {
      return nullptr;
    }
  }
};

#endif // LLVM_SCOPE_H
