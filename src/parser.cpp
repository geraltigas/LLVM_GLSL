#include "parser.h"
#include "ast.h"
#include "global.h"
#include "tokenizer.h"

std::unique_ptr<TopLevelAST> topLevelAst = nullptr;
uint64_t index_temp = 0;
extern std::vector<Token> tokens;
extern TokenType CurTok;
extern std::string IdentifierStr; // Filled in if tok_identifier
extern std::string NumVal;        // Filled in if tok_number

extern std::unique_ptr<LLVMContext> TheContext;
extern std::unique_ptr<Module> TheModule;
extern std::unique_ptr<IRBuilder<>> Builder;
extern std::map<std::string, Value *> NamedValues;

using namespace ast;

std::unique_ptr<ExpressionAST> ParseAssignmentExpression();

void InitializeModule() {
  // Open a new context and module.
  TheContext = std::make_unique<LLVMContext>();
  TheModule = std::make_unique<Module>("GLSL", *TheContext);

  // Create a new builder for the module.
  Builder = std::make_unique<IRBuilder<>>(*TheContext);
}

std::unique_ptr<NumberExprAST> ParseNumberExpr() {
  if (tokens[index_temp].type != tok_number) {
    return nullptr;
  }

  // record
  uint64_t index_record = index_temp;
  // parse
  if (tokens[index_temp].value->find(".") != std::string::npos) {
    // float
    auto result = std::make_unique<NumberExprAST>(
        std::string(tokens[index_temp].value->c_str()),
        tokens[index_temp].value->find("f") != std::string::npos ? type_float
                                                                 : type_double);
    index_temp++;
    return std::move(result);
  }
  // recover
  index_temp = index_record;

  auto result = std::make_unique<NumberExprAST>(
      std::string(tokens[index_temp].value->c_str()), type_int);
  index_temp++;
  return std::move(result);
}

std::unique_ptr<LayoutQualifierIdAST> ParseLayoutQualifierId() {
  // record
  uint64_t index_record = index_temp;
  LayoutIdentifier layoutType;
  // parse
  if (tokens[index_temp].type == tok_location ||
      tokens[index_temp].type == tok_binding) {
    layoutType = tokens[index_temp].type == tok_location ? location : binding;
    index_temp++;

    // record
    uint64_t index_record = index_temp;
    // parse
    if (tokens[index_temp].type == tok_assign) {
      index_temp++;

      // record
      uint64_t index_record = index_temp;
      // parse
      std::unique_ptr<NumberExprAST> numberExprAst = ParseNumberExpr();
      if (numberExprAst != nullptr && numberExprAst->getType() == type_int) {
        // success
        return std::make_unique<LayoutQualifierIdAST>(
            layoutType, std::stoi(numberExprAst->getValue()));
      } else {
        // recover
        index_temp = index_record;
        return nullptr;
      }
    } else {
      // recover
      index_temp = index_record;
      return std::make_unique<LayoutQualifierIdAST>(layoutType);
    }
  } else {
    // recover
    index_temp = index_record;
    return nullptr;
  }
}

std::unique_ptr<std::vector<std::unique_ptr<LayoutQualifierIdAST>>>
ParseLayoutQualifierIdList() {

  std::unique_ptr<std::vector<std::unique_ptr<LayoutQualifierIdAST>>>
      layout_qualifier_ids = std::make_unique<
          std::vector<std::unique_ptr<LayoutQualifierIdAST>>>();

  while (true) {
    // record
    uint64_t index_record = index_temp;
    // parse
    std::unique_ptr<LayoutQualifierIdAST> layout_qualifier_id =
        ParseLayoutQualifierId();
    if (layout_qualifier_id == nullptr) {
      // recover
      index_temp = index_record;
      break;
    }
    layout_qualifier_ids->push_back(std::move(layout_qualifier_id));
  }

  if (layout_qualifier_ids->empty()) {
    return nullptr;
  }

  return layout_qualifier_ids == nullptr ? nullptr
                                         : std::move(layout_qualifier_ids);
}

std::unique_ptr<LayoutQualifierAst> ParseLayoutQualifier() {

  // record
  uint64_t index_record = index_temp;
  // parse
  if (tokens[index_temp].type != tok_layout) {
    // recover
    index_temp = index_record;
    return nullptr; // no layout qualifier
  }
  index_temp++;

  // record
  index_record = index_temp;
  // parse
  if (tokens[index_temp].type != tok_left_paren) {
    // recover
    index_temp = index_record;
    return nullptr;
  }
  index_temp++;

  // record
  index_record = index_temp;

  // parse
  std::unique_ptr<std::vector<std::unique_ptr<LayoutQualifierIdAST>>>
      layout_qualifier_ids = ParseLayoutQualifierIdList();
  if (layout_qualifier_ids == nullptr) {
    // recover
    index_temp = index_record;
    return nullptr;
  }

  // record
  index_record = index_temp;
  // parse
  if (tokens[index_temp].type != tok_right_paren) {
    // recover
    index_temp = index_record;
    return nullptr;
  }
  index_temp++;

  return std::make_unique<LayoutQualifierAst>(std::move(layout_qualifier_ids));
}

std::unique_ptr<LayoutAst> ParseLayout() {

  // record
  uint64_t index_record = index_temp;
  // parse
  std::unique_ptr<LayoutQualifierAst> layout_qualifier = ParseLayoutQualifier();
  if (layout_qualifier == nullptr) {
    // recover
    index_temp = index_record;
    return nullptr;
  }

  // record
  index_record = index_temp;
  // parse
  if (tokens[index_temp].type == tok_uniform) {
    index_temp++;
    return std::make_unique<LayoutAst>(uniform, std::move(layout_qualifier));
  } else if (tokens[index_temp].type == tok_layout_in) {
    index_temp++;
    return std::make_unique<LayoutAst>(in, std::move(layout_qualifier));
  } else if (tokens[index_temp].type == tok_layout_out) {
    index_temp++;
    return std::make_unique<LayoutAst>(out, std::move(layout_qualifier));
  } else {
    // recover
    index_temp = index_record;
    return nullptr;
  }
}

AstType ParseType() {
  // record
  uint64_t index_record = index_temp;
  // parse
  switch (tokens[index_temp].type) {
  case tok_int:
    index_temp++;
    return type_int;
  case tok_float:
    index_temp++;
    return type_float;
  case tok_double:
    index_temp++;
    return type_double;
  case tok_void:
    index_temp++;
    return type_void;
  case tok_bool:
    index_temp++;
    return type_bool;
  case tok_mat2:
    index_temp++;
    return type_mat2;
  case tok_mat3:
    index_temp++;
    return type_mat3;
  case tok_mat4:
    index_temp++;
    return type_mat4;
  case tok_vec2:
    index_temp++;
    return type_vec2;
  case tok_vec3:
    index_temp++;
    return type_vec3;
  case tok_vec4:
    index_temp++;
    return type_vec4;
  case tok_number:
    if (tokens[index_temp].value->find('.') != std::string::npos) {
      if (tokens[index_temp].value->find('f') != std::string::npos) {
        index_temp++;
        return type_float;
      } else {
        index_temp++;
        return type_double;
      }
    } else {
      index_temp++;
      return type_int;
    }
  default:
    // recover
    index_temp = index_record;
    return type_error;
  }
}

std::unique_ptr<std::string> ParseIdentifier() {
  // record
  uint64_t index_record = index_temp;
  // parse
  if (tokens[index_temp].type != tok_identifier) {
    // recover
    index_temp = index_record;
    return nullptr;
  }
  index_temp++;
  return std::make_unique<std::string>(tokens[index_temp - 1].value->c_str());
}

std::unique_ptr<GlobalVariableDefinitionAST> ParseLayoutVariableDefinition() {
  AstType type;
  std::unique_ptr<std::string> name;
  std::unique_ptr<LayoutAst> layout;

  // record
  uint64_t index_record = index_temp;
  // parse
  layout = ParseLayout();

  if (layout == nullptr) {
    // recover
    index_temp = index_record;
    // return nullptr;
  }

  // record
  index_record = index_temp;
  // parse
  type = ParseType();
  if (type == type_void || type == type_error) {
    // recover
    index_temp = index_record;
    return nullptr;
  }

  // record
  index_record = index_temp;
  // parse
  name = ParseIdentifier();
  if (name == nullptr) {
    // recover
    index_temp = index_record;
    return nullptr;
  }

  // record
  index_record = index_temp;
  // parse
  if (tokens[index_temp].type != tok_semicolon) {
    // recover
    index_temp = index_record;
    return nullptr;
  }
  index_temp++;

  return std::make_unique<GlobalVariableDefinitionAST>(
      type, false, std::string(name->c_str()), nullptr, std::move(layout));
};

std::unique_ptr<GlobalVariableDefinitionAST> ParseGlobalVariableDefinition() {
  AstType type;
  std::unique_ptr<std::string> name;

  // record
  uint64_t index_record = index_temp;
  // parse
  std::unique_ptr<LayoutAst> layout = ParseLayout();

  if (layout == nullptr) {
    // recover
    index_temp = index_record;
  }

  // record
  index_record = index_temp;
  // parse
  type = ParseType();
  if (type == type_void || type == type_error) {
    // recover
    index_temp = index_record;
    return nullptr;
  }

  // record
  index_record = index_temp;
  // parse
  name = ParseIdentifier();
  if (name == nullptr) {
    // recover
    index_temp = index_record;
    return nullptr;
  }

  // record
  index_record = index_temp;
  // parse
  if (tokens[index_temp].type == tok_semicolon) {
    index_temp++;
    return std::make_unique<GlobalVariableDefinitionAST>(
        type, false, std::string(name->c_str()), nullptr, std::move(layout));
  }

  // recover
  index_temp = index_record;
  return nullptr;
};

std::unique_ptr<VariableDefinitionAST> ParseVariableDefinition() {
  AstType type;
  std::unique_ptr<std::string> name;
  std::unique_ptr<ExpressionAST> expression;
  bool is_const = false;

  // record
  uint64_t index_record = index_temp;
  // parse
  if (tokens[index_temp].type == tok_const) {
    // set const
    is_const = true;
    index_temp++;
  }

  // record
  index_record = index_temp;
  // parse
  type = ParseType();
  if (type == type_void || type == type_error) {
    // recover
    index_temp = index_record;
    return nullptr;
  }

  // record
  index_record = index_temp;
  // parse
  name = ParseIdentifier();
  if (name == nullptr) {
    // recover
    index_temp = index_record;
    return nullptr;
  }

  // record
  index_record = index_temp;
  // parse
  if (tokens[index_temp].type != tok_assign) {
    // recover
    index_temp = index_record;
    return nullptr;
  }
  index_temp++;

  // record
  index_record = index_temp;
  // parse
  expression = ParseExpression();
  if (expression == nullptr) {
    // recover
    index_temp = index_record;
    return nullptr;
  }

  // record
  index_record = index_temp;
  // parse
  if (tokens[index_temp].type != tok_semicolon) {
    // recover
    index_temp = index_record;
    return nullptr;
  }
  // index_temp++;

  return std::make_unique<VariableDefinitionAST>(
      type, is_const, std::string(name->c_str()), std::move(expression));
};

std::unique_ptr<ExprListAST> ParseExprList() {
  std::vector<std::unique_ptr<ExpressionAST>> expr_list;

  // record
  uint64_t index_record = index_temp;
  // parse
  std::unique_ptr<ExpressionAST> expression = ParseAssignmentExpression();
  if (expression == nullptr) {
    // recover
    index_temp = index_record;
    return nullptr;
  }
  expr_list.push_back(std::move(expression));

  // record
  index_record = index_temp;
  // parse
  while (tokens[index_temp].type == tok_comma) {
    // record
    index_record = index_temp;
    // parse
    index_temp++;
    expression = ParseAssignmentExpression();
    if (expression == nullptr) {
      // recover
      index_temp = index_record;
      return nullptr;
    }
    expr_list.push_back(std::move(expression));
  }

  //  if (expr_list.size() == 1) {
  //    return std::make_unique<ExprListAST>(std::move(
  //        ((SequenceExpressionAST
  //        *)(expr_list[0].release()))->getExpressions()));
  //  }

  return std::make_unique<ExprListAST>(std::move(expr_list));
}

std::unique_ptr<ExpressionAST> ParsePrimaryExpression() {
  // record
  uint64_t index_record = index_temp;
  // parse
  if (tokens[index_temp].type == tok_identifier) {
    // record
    index_record = index_temp;
    // parse
    std::unique_ptr<std::string> name = ParseIdentifier();
    if (name == nullptr) {
      // recover
      index_temp = index_record;
      return nullptr;
    }

    if (tokens[index_temp].type == tok_left_paren) {
      index_temp++;
      // record
      index_record = index_temp;
      // parse
      std::unique_ptr<ExprListAST> expr_list = ParseExprList();
      if (expr_list == nullptr) {
        // recover
        expr_list = std::make_unique<ExprListAST>();
      }
      if (tokens[index_temp].type != tok_right_paren) {
        // recover
        index_temp = index_record;
        return nullptr;
      }
      index_temp++;
      return std::make_unique<FunctionCallAST>(std::string(name->c_str()),
                                               std::move(expr_list));
    } else if (tokens[index_temp].type == tok_left_bracket) {
      index_temp++;
      // record
      index_record = index_temp;
      // parse
      std::unique_ptr<ExpressionAST> expression = ParseExpression();
      if (expression == nullptr) {
        // recover
        index_temp = index_record;
        return nullptr;
      }
      if (tokens[index_temp].type != tok_right_bracket) {
        // recover
        index_temp = index_record;
        return nullptr;
      }
      index_temp++;
      return std::make_unique<VariableIndexExprAST>(std::string(name->c_str()),
                                                    std::move(expression));
    } else {
      return std::make_unique<VariableExprAST>(std::string(name->c_str()));
    }
  } else if (isAstType(tokens[index_temp].type)) {
    // record
    index_record = index_temp;
    // parse
    AstType type = ParseType();
    if (type == type_void || type == type_error) {
      // recover
      index_temp = index_record;
      return nullptr;
    }

    // record
    index_record = index_temp;
    if (tokens[index_temp].type == tok_left_paren) {
      index_temp++;
      // record
      index_record = index_temp;
      // parse
      std::unique_ptr<ExprListAST> expr_list = ParseExprList();
      if (expr_list == nullptr) {
        // recover
        index_temp = index_record;
        return nullptr;
      }
      if (tokens[index_temp].type != tok_right_paren) {
        // recover
        index_temp = index_record;
        return nullptr;
      }
      index_temp++;
      return std::make_unique<TypeConstructorAST>(type, std::move(expr_list));
    }
  } else if (tokens[index_temp].type == tok_number ||
             tokens[index_temp].type == tok_float ||
             tokens[index_temp].type == tok_double ||
             tokens[index_temp].type == tok_int) {
    // record
    index_record = index_temp;
    // parse
    std::unique_ptr<NumberExprAST> number = ParseNumberExpr();
    if (number == nullptr) {
      // recover
      index_temp = index_record;
      return nullptr;
    }
    return std::move(number);
  } else if (tokens[index_temp].type == tok_left_paren) {
    // record
    index_record = index_temp;
    // parse
    index_temp++;
    std::unique_ptr<ExpressionAST> expression = ParseExpression();
    if (expression == nullptr) {
      // recover
      index_temp = index_record;
      return nullptr;
    }
    if (tokens[index_temp].type != tok_right_paren) {
      // recover
      index_temp = index_record;
      return nullptr;
    }
    index_temp++;
    return std::move(expression);
  } else {
    // recover
    index_temp = index_record;
    return nullptr;
  }

  // recover
  index_temp = index_record;
  return nullptr;
}

std::unique_ptr<ExpressionAST> ParsePostfixExpression() {
  // record
  uint64_t index_record = index_temp;
  // parse
  std::unique_ptr<ExpressionAST> expression = ParsePrimaryExpression();
  // LOG(expression);
  if (expression == nullptr) {
    // recover
    index_temp = index_record;
    return nullptr;
  }

  // record
  index_record = index_temp;
  TokenType tokenType;
  // parse
  while (isPostfix(tokens[index_temp].type)) {
    tokenType = tokens[index_temp].type;
    if (tokenType == tok_dot) {
      // record
      index_record = index_temp;
      // parse
      index_temp++;
      if (tokens[index_temp].type == tok_identifier) {
        // record
        index_record = index_temp;
        // parse
        std::unique_ptr<std::string> name = ParseIdentifier();
        if (name == nullptr) {
          // recover
          index_temp = index_record;
          return nullptr;
        }
        expression = std::make_unique<PostfixExpressionAST>(
            tokenToExprType(tokenType), std::move(expression));
        ((PostfixExpressionAST *)expression.get())
            ->setIdentifier(std::string(name->c_str()));
      } else {
        // recover
        index_temp = index_record;
        return nullptr;
      }
    } else {
      // record
      index_record = index_temp;
      // parse
      index_temp++;
      return std::make_unique<PostfixExpressionAST>(
          tokenType == tok_plus_p ? plus_p_expr : minus_m_expr,
          std::move(expression));
    }
  }
  return expression;
}

std::unique_ptr<ExpressionAST> ParsePrefixExpression() {
  // record
  uint64_t index_record = index_temp;
  // parse
  if (isPrefix(tokens[index_temp].type)) {
    TokenType tokenType = tokens[index_temp].type;
    index_temp++;
    // record
    index_record = index_temp;
    // parse
    std::unique_ptr<ExpressionAST> expression = ParsePrefixExpression();
    if (expression == nullptr) {
      // recover
      index_temp = index_record;
      return nullptr;
    }
    return std::make_unique<PrefixExpressionAST>(tokenToExprType(tokenType),
                                                 std::move(expression));
  } else {
    // record
    index_record = index_temp;
    // parse
    std::unique_ptr<ExpressionAST> expression = ParsePostfixExpression();
    if (expression == nullptr) {
      // recover
      index_temp = index_record;
      return nullptr;
    }
    return expression;
  }
}

std::unique_ptr<ExpressionAST> ParseMultiplicativeExpression() {
  // record
  uint64_t index_record = index_temp;
  // parse
  std::unique_ptr<ExpressionAST> expression = ParsePrefixExpression();

  if (expression == nullptr) {
    // recover
    index_temp = index_record;
    return nullptr;
  }

  // record
  index_record = index_temp;
  // parse
  TokenType tokenType;
  while (isMultiplicative(tokens[index_temp].type)) {
    tokenType = tokens[index_temp].type;
    index_temp++;
    // record
    index_record = index_temp;
    // parse
    std::unique_ptr<ExpressionAST> expression_ = ParsePrefixExpression();
    if (expression_ == nullptr) {
      // recover
      index_temp = index_record;
      return nullptr;
    }
    expression = std::make_unique<BinaryExpressionAST>(
        tokenToExprType(tokenType), std::move(expression),
        std::move(expression_));
  }
  return expression;
}

std::unique_ptr<ExpressionAST> ParseAdditiveExpression() {
  // record
  uint64_t index_record = index_temp;
  // parse
  std::unique_ptr<ExpressionAST> expression = ParseMultiplicativeExpression();

  if (expression == nullptr) {
    // recover
    index_temp = index_record;
    return nullptr;
  }

  // record
  index_record = index_temp;
  // parse
  TokenType tokenType;
  while (isAdditive(tokens[index_temp].type)) {
    tokenType = tokens[index_temp].type;
    index_temp++;
    // record
    index_record = index_temp;
    // parse
    std::unique_ptr<ExpressionAST> expression_ =
        ParseMultiplicativeExpression();
    if (expression_ == nullptr) {
      // recover
      index_temp = index_record;
      return nullptr;
    }
    expression = std::make_unique<BinaryExpressionAST>(
        tokenToExprType(tokenType), std::move(expression),
        std::move(expression_));
  }

  return expression;
}

std::unique_ptr<ExpressionAST> ParseShiftExpression() {
  // record
  uint64_t index_record = index_temp;
  // parse
  std::unique_ptr<ExpressionAST> expression = ParseAdditiveExpression();

  if (expression == nullptr) {
    // recover
    index_temp = index_record;
    return nullptr;
  }

  // record
  index_record = index_temp;
  // parse
  TokenType tokenType;
  while (isShift(tokens[index_temp].type)) {
    tokenType = tokens[index_temp].type;
    index_temp++;
    // record
    index_record = index_temp;
    // parse
    std::unique_ptr<ExpressionAST> expression_ = ParseAdditiveExpression();
    if (expression_ == nullptr) {
      // recover
      index_temp = index_record;
      return nullptr;
    }
    expression = std::make_unique<BinaryExpressionAST>(
        tokenToExprType(tokenType), std::move(expression),
        std::move(expression_));
  }
  return expression;
}

std::unique_ptr<ExpressionAST> ParseRelationalExpression() {
  // record
  uint64_t index_record = index_temp;
  // parse
  std::unique_ptr<ExpressionAST> expression = ParseShiftExpression();

  if (expression == nullptr) {
    // recover
    index_temp = index_record;
    return nullptr;
  }

  // record
  index_record = index_temp;
  // parse
  TokenType tokenType;
  while (isRelational(tokens[index_temp].type)) {
    tokenType = tokens[index_temp].type;
    index_temp++;
    // record
    index_record = index_temp;
    // parse
    std::unique_ptr<ExpressionAST> expression_ = ParseShiftExpression();
    if (expression_ == nullptr) {
      // recover
      index_temp = index_record;
      return nullptr;
    }
    expression = std::make_unique<BinaryExpressionAST>(
        tokenToExprType(tokenType), std::move(expression),
        std::move(expression_));
  }
  return expression;
}

std::unique_ptr<ExpressionAST> ParseEqualityExpression() {
  // record
  uint64_t index_record = index_temp;
  // parse
  std::unique_ptr<ExpressionAST> expression = ParseRelationalExpression();

  if (expression == nullptr) {
    // recover
    index_temp = index_record;
    return nullptr;
  }

  // record
  index_record = index_temp;
  // parse
  TokenType tokenType;
  while (isEuqality(tokens[index_temp].type)) {
    tokenType = tokens[index_temp].type;
    index_temp++;
    // record
    index_record = index_temp;
    // parse
    std::unique_ptr<ExpressionAST> expression_ = ParseRelationalExpression();
    if (expression_ == nullptr) {
      // recover
      index_temp = index_record;
      return nullptr;
    }
    expression = std::make_unique<BinaryExpressionAST>(
        tokenToExprType(tokenType), std::move(expression),
        std::move(expression_));
  }
  return expression;
}

std::unique_ptr<ExpressionAST> ParseBitwiseAndExpression() {
  // record
  uint64_t index_record = index_temp;
  // parse
  std::unique_ptr<ExpressionAST> expression = ParseEqualityExpression();

  if (expression == nullptr) {
    // recover
    index_temp = index_record;
    return nullptr;
  }

  // record
  index_record = index_temp;
  // parse
  while (tokens[index_temp].type == tok_bit_and) {
    index_temp++;
    // record
    index_record = index_temp;
    // parse
    std::unique_ptr<ExpressionAST> expression_ = ParseEqualityExpression();
    if (expression_ == nullptr) {
      // recover
      index_temp = index_record;
      return nullptr;
    }
    expression = std::make_unique<BinaryExpressionAST>(
        bit_and_expr, std::move(expression), std::move(expression_));
  }

  return expression;
}

std::unique_ptr<ExpressionAST> ParseBitwiseExclusiveOrExpression() {
  // record
  uint64_t index_record = index_temp;
  // parse
  std::unique_ptr<ExpressionAST> expression = ParseBitwiseAndExpression();

  if (expression == nullptr) {
    // recover
    index_temp = index_record;
    return nullptr;
  }

  // record
  index_record = index_temp;
  // parse
  while (tokens[index_temp].type == tok_bit_xor) {
    index_temp++;
    // record
    index_record = index_temp;
    // parse
    std::unique_ptr<ExpressionAST> expression_ = ParseBitwiseAndExpression();
    if (expression_ == nullptr) {
      // recover
      index_temp = index_record;
      return nullptr;
    }
    expression = std::make_unique<BinaryExpressionAST>(
        bit_xor_expr, std::move(expression), std::move(expression_));
  }

  return expression;
}

std::unique_ptr<ExpressionAST> ParseBitwiseInlusiveOrExpression() {
  // record
  uint64_t index_record = index_temp;
  // parse
  std::unique_ptr<ExpressionAST> expression =
      ParseBitwiseExclusiveOrExpression();

  if (expression == nullptr) {
    // recover
    index_temp = index_record;
    return nullptr;
  }

  // record
  index_record = index_temp;
  // parse
  while (tokens[index_temp].type == tok_bit_or) {
    index_temp++;
    // record
    index_record = index_temp;
    // parse
    std::unique_ptr<ExpressionAST> expression_ =
        ParseBitwiseExclusiveOrExpression();
    if (expression_ == nullptr) {
      // recover
      index_temp = index_record;
      return nullptr;
    }
    expression = std::make_unique<BinaryExpressionAST>(
        bit_or_expr, std::move(expression), std::move(expression_));
  }

  return expression;
}

std::unique_ptr<ExpressionAST> ParseLogicalAndExpression() {
  // record
  uint64_t index_record = index_temp;
  // parse
  std::unique_ptr<ExpressionAST> expression =
      ParseBitwiseInlusiveOrExpression();

  if (expression == nullptr) {
    // recover
    index_temp = index_record;
    return nullptr;
  }

  // record
  index_record = index_temp;
  // parse
  while (tokens[index_temp].type == tok_and) {
    index_temp++;
    // record
    index_record = index_temp;
    // parse
    std::unique_ptr<ExpressionAST> expression_ =
        ParseBitwiseInlusiveOrExpression();
    if (expression_ == nullptr) {
      // recover
      index_temp = index_record;
      return nullptr;
    }
    expression = std::make_unique<BinaryExpressionAST>(
        and_expr, std::move(expression), std::move(expression_));
  }

  return expression;
}

std::unique_ptr<ExpressionAST> ParseLogicalExclusiveOrExpression() {
  // record
  uint64_t index_record = index_temp;
  // parse
  std::unique_ptr<ExpressionAST> expression = ParseLogicalAndExpression();
  if (expression == nullptr) {
    // recover
    index_temp = index_record;
    return nullptr;
  }

  // record
  index_record = index_temp;
  // parse
  while (tokens[index_temp].type == tok_xor) {
    index_temp++;
    // record
    index_record = index_temp;
    // parse
    std::unique_ptr<ExpressionAST> expression_ = ParseLogicalAndExpression();
    if (expression_ == nullptr) {
      // recover
      index_temp = index_record;
      return nullptr;
    }
    expression = std::make_unique<BinaryExpressionAST>(
        xor_expr, std::move(expression), std::move(expression_));
  }

  return expression;
}

std::unique_ptr<ExpressionAST> ParseLogicalInclusiveOrExpression() {
  // record
  uint64_t index_record = index_temp;
  // parse
  std::unique_ptr<ExpressionAST> expression =
      ParseLogicalExclusiveOrExpression();
  if (expression == nullptr) {
    // recover
    index_temp = index_record;
    return nullptr;
  }

  // record
  index_record = index_temp;
  // parse
  while (tokens[index_temp].type == tok_or) {
    index_temp++;
    // record
    index_record = index_temp;
    // parse
    std::unique_ptr<ExpressionAST> expression_ =
        ParseLogicalExclusiveOrExpression();
    if (expression_ == nullptr) {
      // recover
      index_temp = index_record;
      return nullptr;
    }
    expression = std::make_unique<BinaryExpressionAST>(
        or_expr, std::move(expression), std::move(expression_));
  }

  return expression;
}

std::unique_ptr<ExpressionAST> ParseAssignmentExpression();

std::unique_ptr<ExpressionAST> ParseConditionalExpression() {
  // record
  uint64_t index_record = index_temp;
  // parse
  std::unique_ptr<ExpressionAST> expression =
      ParseLogicalInclusiveOrExpression();
  if (expression == nullptr) {
    // recover
    index_temp = index_record;
    return nullptr;
  }

  // record
  index_record = index_temp;
  // parse
  if (tokens[index_temp].type != tok_unary) {
    // recover
    index_temp = index_record;
    return expression;
  }

  index_temp++;
  // record
  index_record = index_temp;
  // parse
  std::unique_ptr<ExpressionAST> expression_ = ParseExpression();
  if (expression_ == nullptr) {
    // recover
    index_temp = index_record;
    return nullptr;
  }

  // record
  index_record = index_temp;
  // parse
  if (tokens[index_temp].type != tok_colon) {
    // recover
    index_temp = index_record;
    return nullptr;
  }
  index_temp++;

  // record
  index_record = index_temp;
  // parse
  std::unique_ptr<ExpressionAST> expression__ = ParseAssignmentExpression();
  if (expression__ == nullptr) {
    // recover
    index_temp = index_record;
    return nullptr;
  }

  return std::make_unique<ConditionalExpressionAST>(
      std::move(expression), std::move(expression_), std::move(expression__));
};

std::unique_ptr<ExpressionAST> ParseAssignmentExpression() {
  // record
  uint64_t index_record = index_temp;
  // parse
  std::unique_ptr<ExpressionAST> expression = ParseConditionalExpression();
  if (expression == nullptr) {
    // recover
    index_temp = index_record;
    return nullptr;
  }

  TokenType tokenType = tokens[index_temp].type;

  // record
  index_record = index_temp;
  // parse
  if (!isAssignment(tokens[index_temp].type)) {
    return expression;
  }

  index_temp++;

  // record
  index_record = index_temp;
  // parse
  std::unique_ptr<ExpressionAST> expression_ = ParseConditionalExpression();
  if (expression_ == nullptr) {
    // recover
    index_temp = index_record;
    return nullptr;
  }

  return std::unique_ptr<ExpressionAST>(
      (ExpressionAST *)new BinaryExpressionAST(tokenToExprType(tokenType),
                                               std::move(expression),
                                               std::move(expression_)));
}

std::unique_ptr<ExpressionAST> ParseSequenceExpression() {
  std::vector<std::unique_ptr<ExpressionAST>> sequence_expr;
  // record
  uint64_t index_record = index_temp;
  // parse
  std::unique_ptr<ExpressionAST> expression = ParseAssignmentExpression();
  if (expression == nullptr) {
    // recover
    index_temp = index_record;
    return nullptr;
  }

  sequence_expr.push_back(std::move(expression));

  // record
  index_record = index_temp;
  // parse
  while (tokens[index_temp].type == tok_comma) {
    index_temp++;
    // record
    index_record = index_temp;
    // parse
    std::unique_ptr<ExpressionAST> expression_ = ParseAssignmentExpression();
    if (expression_ == nullptr) {
      // recover
      index_temp = index_record;
      return nullptr;
    }

    // combine
    sequence_expr.push_back(std::move(expression_));
  }

  if (sequence_expr.size() == 1) {
    return std::move(sequence_expr[0]);
  }

  return std::make_unique<SequenceExpressionAST>(std::move(sequence_expr));
}

std::unique_ptr<ExpressionAST> ParseExpression() {
  // record
  uint64_t index_record = index_temp;
  // parse
  std::unique_ptr<ExpressionAST> expression = ParseSequenceExpression();

  if (expression == nullptr) {
    // recover
    index_temp = index_record;
    return nullptr;
  }

  // LOG(expression);

  return expression;
}

std::unique_ptr<SentenceAST> ParseSentence() {
  // record
  uint64_t index_record = index_temp;
  // parse
  if (tokens[index_temp].type == tok_left_brace) {
    index_temp++;
    std::unique_ptr<SentencesAST> sentence = ParseSentences();
    if (sentence == nullptr) {
      // recover
      index_temp = index_record;
      return nullptr;
    }
    if (tokens[index_temp].type != tok_right_brace) {
      // recover
      index_temp = index_record;
      return nullptr;
    }
    index_temp++;
    return sentence;
  } else if (tokens[index_temp].type == tok_semicolon) {
    // empty sentence
    index_temp++;
    return std::make_unique<EmptySentenceAST>();
  }

  // record
  index_record = index_temp;
  // parse
  std::unique_ptr<VariableDefinitionAST> variable_definition =
      ParseVariableDefinition();
  if (variable_definition != nullptr) {
    index_temp++;
    // change to sentence
    return std::unique_ptr<SentenceAST>(
        std::move(variable_definition)); // TODO: check whether it is right
  } else {
    // recover
    index_temp = index_record;
  }

  // if statement
  if (tokens[index_temp].type == tok_if) {
    index_temp++;
    if (tokens[index_temp].type != tok_left_paren) {
      // recover
      index_temp = index_record;
      return nullptr;
    }
    index_temp++;
    std::unique_ptr<ExpressionAST> condition = ParseExpression();
    if (condition == nullptr) {
      // recover
      index_temp = index_record;
      return nullptr;
    }
    if (tokens[index_temp].type != tok_right_paren) {
      // recover
      index_temp = index_record;
      return nullptr;
    }
    index_temp++;
    std::unique_ptr<SentenceAST> if_sentence = ParseSentence();
    if (if_sentence == nullptr) {
      // recover
      index_temp = index_record;
      return nullptr;
    }
    if (tokens[index_temp].type != tok_else) {
      // only if no else
      return std::make_unique<IfStatementAST>(std::move(condition),
                                              std::move(if_sentence), nullptr);
    }
    index_temp++;
    std::unique_ptr<SentenceAST> else_sentence = ParseSentence();
    if (else_sentence == nullptr) {
      // recover
      index_temp = index_record;
      return nullptr;
    }
    return std::make_unique<IfStatementAST>(
        std::move(condition), std::move(if_sentence), std::move(else_sentence));
  }

  // for statement
  if (tokens[index_temp].type == tok_for) {
    index_temp++;
    if (tokens[index_temp].type != tok_left_paren) {
      // recover
      index_temp = index_record;
      return nullptr;
    }
    index_temp++;
    // record
    index_record = index_temp;
    // parse
    std::unique_ptr<SentenceAST> init = ParseExpression();
    if (init == nullptr) {
      // recover
      index_temp = index_record;
      // record
      index_record = index_temp;
      // parse
      init = ParseVariableDefinition();
      if (init == nullptr) {
        // recover
        index_temp = index_record;
        return nullptr;
      }
    }
    if (tokens[index_temp].type != tok_semicolon) {
      // recover
      index_temp = index_record;
      return nullptr;
    }
    index_temp++;
    std::unique_ptr<ExpressionAST> condition = ParseExpression();
    if (condition == nullptr) {
      // recover
      index_temp = index_record;
      return nullptr;
    }
    if (tokens[index_temp].type != tok_semicolon) {
      // recover
      index_temp = index_record;
      return nullptr;
    }
    index_temp++;
    std::unique_ptr<ExpressionAST> update = ParseExpression();
    if (update == nullptr) {
      // recover
      index_temp = index_record;
      return nullptr;
    }
    if (tokens[index_temp].type != tok_right_paren) {
      // recover
      index_temp = index_record;
      return nullptr;
    }
    index_temp++;
    std::unique_ptr<SentenceAST> for_sentence = ParseSentence();
    if (for_sentence == nullptr) {
      // recover
      index_temp = index_record;
      return nullptr;
    }
    return std::make_unique<ForStatementAST>(
        std::move(init), std::move(condition), std::move(update),
        std::move(for_sentence));
  }

  // return statement
  if (tokens[index_temp].type == tok_return) {
    index_temp++;
    // return void
    if (tokens[index_temp].type == tok_semicolon) {
      index_temp++;
      return std::make_unique<ReturnStatementAST>();
    }
    // return expression
    std::unique_ptr<ExpressionAST> return_expression = ParseExpression();
    if (return_expression == nullptr) {
      // recover
      index_temp = index_record;
      return nullptr;
    }
    if (tokens[index_temp].type != tok_semicolon) {
      // recover
      index_temp = index_record;
      return nullptr;
    }
    index_temp++;
    return std::make_unique<ReturnStatementAST>(std::move(return_expression));
  }

  // expression statement
  std::unique_ptr<ExpressionAST> expression = ParseExpression();
  if (expression == nullptr) {
    // recover
    index_temp = index_record;
    return nullptr;
  }
  if (tokens[index_temp].type != tok_semicolon) {
    // recover
    index_temp = index_record;
    return nullptr;
  }
  index_temp++;
  // return
  return std::unique_ptr<ExpressionAST>(std::move(expression));
}

std::unique_ptr<SentencesAST> ParseSentences() { // TODO: syntax check here
  std::vector<std::unique_ptr<SentenceAST>> sentences = {};

  while (true) {
    // record
    uint64_t index_record = index_temp;
    // parse
    std::unique_ptr<SentenceAST> sentence = ParseSentence();
    if (sentence == nullptr) {
      // recover
      index_temp = index_record;
      break;
    }
    sentences.push_back(std::move(sentence));
  }

  return std::make_unique<SentencesAST>(std::move(sentences));
};

std::unique_ptr<FunctionDefinitionAST> ParseFunctionDefinition() {

  std::vector<std::unique_ptr<FunctionArgumentAST>> parameters = {};
  std::unique_ptr<std::string> name;
  AstType returnType;

  // record
  uint64_t index_record = index_temp;
  // parse
  returnType = ParseType();
  if (returnType == type_error) {
    // recover
    index_temp = index_record;
    return nullptr;
  }

  // record
  index_record = index_temp;
  // parse
  name = ParseIdentifier();
  if (name == nullptr) {
    // recover
    index_temp = index_record;
    return nullptr;
  }

  // record
  index_record = index_temp;
  // parse
  if (tokens[index_temp].type != tok_left_paren) {
    // recover
    index_temp = index_record;
    return nullptr;
  }

  index_temp++;

  // parse parameters
  while (true) {

    if (tokens[index_temp].type == tok_right_paren) {
      index_temp++;
      break;
    }
    // record
    index_record = index_temp;
    TokenType tokenType = tokens[index_temp].type;
    // parse
    AstType type = ParseType();
    if (type == type_error && tokenType != tok_right_paren) {
      // recover
      index_temp = index_record;
      return nullptr;
    }

    // record
    index_record = index_temp;
    // parse
    std::unique_ptr<std::string> name = ParseIdentifier();
    if (name == nullptr && tokenType != tok_right_paren) {
      // recover
      index_temp = index_record;
      return nullptr;
    }

    if (tokenType != tok_right_paren) {
      parameters.push_back(
          std::make_unique<FunctionArgumentAST>(type, std::string(*name)));
    }

    // record
    index_record = index_temp;
    // parse
    if (tokens[index_temp].type == tok_comma) {
      index_temp++;
    } else if (tokens[index_temp].type == tok_right_paren) {
      index_temp++;
      break;
    } else {
      // recover
      index_temp = index_record;
      return nullptr;
    }
  }

  // record
  index_record = index_temp;
  // parse
  if (tokens[index_temp].type != tok_left_brace) {
    // recover
    index_temp = index_record;
    return nullptr;
  }
  index_temp++;

  // parse body
  std::unique_ptr<SentencesAST> body = ParseSentences();
  if (body == nullptr) {
    // recover
    index_temp = index_record;
    return nullptr;
  }

  // record
  index_record = index_temp;
  // parse
  if (tokens[index_temp].type != tok_right_brace) {
    // recover
    index_temp = index_record;
    return nullptr;
  }
  index_temp++;

  // check return
//  bool hasReturn = false;
//  if (returnType == type_void) {
//    for (auto &sentence : body->getSentences()) {
//      if (sentence->isReturn()) {
//        hasReturn = true;
//        break;
//      }
//    }
//  }
//
//  if (!hasReturn) {
//    body->getSentences().push_back(std::make_unique<ReturnStatementAST>());
//    body->getSentences().push_back(std::make_unique<ReturnStatementAST>());
//  }

  return std::make_unique<FunctionDefinitionAST>(
      returnType, std::string(name->c_str()), std::move(parameters),
      std::move(body));
}

int ParseVersion() {
  int version = -1;
  uint64_t index_record = index_temp;
  if (tokens[index_temp].type == tok_version) { // version
    index_temp++;
    // find the '.' and 'f' in the string
    std::string version_string = *tokens[index_temp].value;
    std::size_t dot_index = version_string.find('.');
    std::size_t f_index = version_string.find('f');
    if (dot_index == std::string::npos && f_index == std::string::npos) {
      version = std::stoi(tokens[index_temp].value->c_str());
    }
  }
  index_temp++;
  return version;
}

std::unique_ptr<std::vector<std::unique_ptr<DefinitionAST>>>
ParseDefinitions() {
  std::unique_ptr<std::vector<std::unique_ptr<DefinitionAST>>> definitionASTs =
      std::make_unique<std::vector<std::unique_ptr<DefinitionAST>>>();
  definitionASTs->push_back(std::make_unique<GlobalVariableDefinitionAST>(
      type_vec4, false, "gl_Position", nullptr, nullptr));
  while (true) {
    if (tokens[index_temp].type == tok_eof) { // end
      return definitionASTs;
    }
    // record
    uint64_t index_record = index_temp;
    std::unique_ptr<FunctionDefinitionAST> functionAST =
        ParseFunctionDefinition();
    if (functionAST != nullptr) {
      definitionASTs->push_back(std::move(functionAST));
      continue;
    }
    // recover
    index_temp = index_record;

    // record
    index_record = index_temp;
    // parse
    std::unique_ptr<GlobalVariableDefinitionAST> layoutVariableDefinition =
        ParseGlobalVariableDefinition();
    if (layoutVariableDefinition == nullptr) {
      // recover
      index_temp = index_record;
      return nullptr;
    }
    definitionASTs->push_back(std::move(layoutVariableDefinition));
  }
}

int parseAST() {
  int version = 0;
  std::unique_ptr<std::vector<std::unique_ptr<DefinitionAST>>> definitionASTs =
      std::make_unique<std::vector<std::unique_ptr<DefinitionAST>>>();

  if (tokens[index_temp].type == tok_eof) { // end
    return 0;
  }

  version = ParseVersion();

  if (version == -1) {
    return -1;
  }

  definitionASTs = ParseDefinitions();

  if (definitionASTs == nullptr || definitionASTs->empty()) {
    return -1;
  }

  topLevelAst =
      std::make_unique<TopLevelAST>(version, std::move(definitionASTs));
  return 0;
};