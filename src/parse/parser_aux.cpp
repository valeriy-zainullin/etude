#include <parse/parse_error.hpp>
#include <parse/parser.hpp>

Parser::Parser(lex::Lexer& l) : lexer_{l} {
}

bool Parser::Matches(lex::TokenType type) {
  if (lexer_.Peek().type != type) {
    return false;
  }

  lexer_.Advance();
  return true;
}

bool Parser::MatchesComparisonSign(lex::TokenType type) {
  switch (type) {
    case lex::TokenType::GE:
    case lex::TokenType::GT:
    case lex::TokenType::LE:
    case lex::TokenType::LT:
      lexer_.Advance();
      return true;

    default:
      return false;
  }
}

void Parser::Consume(lex::TokenType type) {
  if (!Matches(type)) {
    throw parse::errors::ParseTokenError(
        lex::FormatTokenType(type),
        lexer_.GetPreviousToken().location
    );
  }
}

// Checks whether `.none` is a tag-only value
// as opposed to `.some <expr>`
bool Parser::TagOnly() {
  auto next_token = lexer_.Peek().type;
  switch (next_token) {
    case lex::TokenType::RIGHT_PAREN:
    case lex::TokenType::RIGHT_CBRACE:
    case lex::TokenType::RIGHT_SBRACE:
    case lex::TokenType::COMMA:
    case lex::TokenType::COLON:
    case lex::TokenType::BIT_OR:
    case lex::TokenType::ELSE:
    case lex::TokenType::SEMICOLON:
    case lex::TokenType::VAR:
      return true;

    default:
      return false;
  }
}

std::string Parser::FormatLocation() {
  return lexer_.GetPreviousToken().location.Format();
}
