#include <vm/codegen/compiler.hpp>
#include <vm/interpreter.hpp>

#include <parse/parser.hpp>
#include <lex/lexer.hpp>

// Finally,
#include <catch2/catch.hpp>

//////////////////////////////////////////////////////////////////////

TEST_CASE("vm:codegen: push constant", "[vm]") {
  char stream[] = "1";
  std::stringstream source{stream};
  Parser p{lex::Lexer{source}};

  auto pr = p.ParsePrimary();

  vm::ExecutableChunk chunk{
      .instructions =
          {
              vm::Instr{.type = vm::InstrType::PUSH_STACK,  //
                        .arg1 = 0}                          //
          },                                                //
      //
      .attached_vals{
          vm::rt::PrimitiveValue{.tag = vm::rt::ValueTag::Int,  //
                                 .as_int = 1},                  //
      },
  };

  vm::codegen::Compiler c;
  auto res = c.Compile(pr);

  CHECK(chunk.instructions[0].type == res.instructions[0].type);
  CHECK(chunk.attached_vals[0].tag == res.attached_vals[0].tag);
  CHECK(chunk.attached_vals[0].as_int == res.attached_vals[0].as_int);
  CHECK(vm::BytecodeInterpreter::InterpretStandalone({res}) == 1);
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("vm:codegen:add", "[vm]") {
  char stream[] = "1 + 2";
  std::stringstream source{stream};
  Parser p{lex::Lexer{source}};

  auto pr = p.ParseExpression();

  vm::codegen::Compiler c;
  auto res = c.Compile(pr);

  CHECK(vm::BytecodeInterpreter::InterpretStandalone({res}) == 3);
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("vm:codegen:if", "[vm:codegen]") {
  char stream[] = "if true { 2 }";
  std::stringstream source{stream};
  Parser p{lex::Lexer{source}};

  auto pr = p.ParseExpression();

  vm::codegen::Compiler c;
  auto res = c.Compile(pr);

  CHECK(vm::BytecodeInterpreter::InterpretStandalone({res}) == 2);
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("vm:codegen:if-else-true", "[vm:codegen]") {
  char stream[] = "if true { 2 } else { 3 }";
  std::stringstream source{stream};
  Parser p{lex::Lexer{source}};

  auto pr = p.ParseExpression();

  vm::codegen::Compiler c;
  auto res = c.Compile(pr);

  CHECK(vm::BytecodeInterpreter::InterpretStandalone({res}) == 2);
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("vm:codegen:if-else-false", "[vm:codegen]") {
  char stream[] = "if false { 2 } else { 3 }";
  std::stringstream source{stream};
  Parser p{lex::Lexer{source}};

  auto pr = p.ParseExpression();

  vm::codegen::Compiler c;
  auto res = c.Compile(pr);

  CHECK(vm::BytecodeInterpreter::InterpretStandalone({res}) == 3);
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("vm:codegen:local", "[vm:codegen]") {
  char stream[] =
      "                                                 "
      "fun f(b: Bool) {                                 "
      "    var res = if b { 100 } else { 101 };         "
      "    res + res                                    "
      "}                                                "
      "                                                 "
      "                                                 ";
  std::stringstream source{stream};
  Parser p{lex::Lexer{source}};

  auto pr = p.ParseStatement();

  vm::codegen::Compiler c;
  auto res = c.Compile(pr);

  CHECK(vm::BytecodeInterpreter::InterpretStandalone({res}) == 3);
}

//////////////////////////////////////////////////////////////////////
