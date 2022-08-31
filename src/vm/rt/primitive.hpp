#pragma once

#include <lex/token.hpp>

#include <fmt/format.h>

#include <cstdint>
#include <variant>
#include <string>

namespace vm::rt {

//////////////////////////////////////////////////////////////////////

enum class ValueTag {
  Int,
  Bool,
  Char,
};

//////////////////////////////////////////////////////////////////////

struct PrimitiveValue {
  ValueTag tag;
  union {
    int as_int;
    bool as_bool;
    char as_char;
  };
};

//////////////////////////////////////////////////////////////////////

PrimitiveValue FromSemInfo(lex::Token::SemInfo sem_info);

//////////////////////////////////////////////////////////////////////

}  // namespace vm::rt