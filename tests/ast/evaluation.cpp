#include <ast/visitors/evaluator.hpp>

#include <rt/base_object.hpp>

#include <parse/parser.hpp>

#include <lex/lexer.hpp>

#include <fmt/core.h>

// Finally,
#include <catch2/catch.hpp>

using rt::FromPrim;

//////////////////////////////////////////////////////////////////////

TEST_CASE("Grouping", "[ast]") {
  Evaluator e;

  SECTION("Grouping") {
    char stream[] = "1 - (2 - 3)";
    std::stringstream source{stream};
    Parser p{lex::Lexer{source}};

    CHECK(e.Eval(p.ParseExpression()) == FromPrim(2));
  }

  SECTION("Associativity") {
    char stream[] = "1 - 2 - 3";
    std::stringstream source{stream};
    Parser p{lex::Lexer{source}};

    CHECK(e.Eval(p.ParseExpression()) == FromPrim(-4));
  }
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Booleans", "[parser]") {
  char stream[] = "!true";
  std::stringstream source{stream};
  Parser p{lex::Lexer{source}};

  Evaluator e;
  CHECK(e.Eval(p.ParseExpression()) == FromPrim(false));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Variable decalration", "[ast]") {
  char stream[] = "var x = 5;";
  std::stringstream source{stream};
  Parser p{lex::Lexer{source}};

  Evaluator e;
  CHECK_NOTHROW(e.Eval(p.ParseStatement()));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Keep state", "[ast]") {
  char stream[] =
      "var x = 5;"  //
      "x + x";
  std::stringstream source{stream};
  Parser p{lex::Lexer{source}};

  Evaluator e;
  e.Eval(p.ParseStatement());
  CHECK(e.Eval(p.ParseExpression()) == FromPrim(10));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Multistate", "[ast]") {
  char stream[] =
      "var x = 5;"  //
      "var y = 3;"  //
      "x - y";
  std::stringstream source{stream};
  Parser p{lex::Lexer{source}};

  Evaluator e;
  e.Eval(p.ParseStatement());
  e.Eval(p.ParseStatement());
  CHECK(e.Eval(p.ParseExpression()) == FromPrim(2));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Initialize with variable", "[ast]") {
  char stream[] =
      "var x = 5;"  //
      "var y = x;"  //
      "x - y";
  std::stringstream source{stream};
  Parser p{lex::Lexer{source}};

  Evaluator e;
  e.Eval(p.ParseStatement());
  e.Eval(p.ParseStatement());
  CHECK(e.Eval(p.ParseExpression()) == FromPrim(0));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Overwrite", "[ast]") {
  char stream[] =
      "var x = 5;"  //
      "var x = 4;"  //
      "x";
  std::stringstream source{stream};
  Parser p{lex::Lexer{source}};

  Evaluator e;
  e.Eval(p.ParseStatement());
  e.Eval(p.ParseStatement());
  CHECK(e.Eval(p.ParseExpression()) == FromPrim(4));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Unknown variable", "[ast]") {
  char stream[] =
      "var x = 5;"  //
      "y + x";
  std::stringstream source{stream};
  Parser p{lex::Lexer{source}};

  Evaluator e;
  e.Eval(p.ParseStatement());
  CHECK_THROWS(e.Eval(p.ParseExpression()));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Eval string literal", "[ast]") {
  char stream[] = " \"abc\"";
  std::stringstream source{stream};
  Parser p{lex::Lexer{source}};

  Evaluator e;
  CHECK(e.Eval(p.ParseExpression()) == FromPrim("abc"));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Eval fn decl", "[ast]") {
  std::stringstream source("fun f  () Unit     { 123; }");
  //                        -----  --------  -------------
  //                        name   no args   block-expr
  Parser p{lex::Lexer{source}};

  Evaluator e;
  CHECK_NOTHROW(e.Eval(p.ParseStatement()));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Eval fn decl args", "[ast]") {
  std::stringstream source(
      "fun f"
      "(a1: Int, a2: Bool, a3: String) Unit"
      "{ 123; }");

  Parser p{lex::Lexer{source}};

  Evaluator e;
  CHECK_NOTHROW(e.Eval(p.ParseStatement()));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Bad scope access", "[ast]") {
  std::stringstream source(" { var x = 5; } x");
  Parser p{lex::Lexer{source}};

  Evaluator e;
  e.Eval(p.ParseExpression());
  CHECK_THROWS(e.Eval(p.ParseExpression()));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Fn call", "[ast]") {
  std::stringstream source(
      "var a = 3;"
      "fun f() Int { return a; }"
      "f()");
  Parser p{lex::Lexer{source}};

  Evaluator e;
  e.Eval(p.ParseStatement());
  e.Eval(p.ParseStatement());
  CHECK(e.Eval(p.ParseExpression()) == FromPrim(3));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Intrinsic print", "[ast][.]") {
  std::stringstream source("print(4, 3);");
  Parser p{lex::Lexer{source}};

  Evaluator e;
  e.Eval(p.ParseStatement());
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Return value", "[ast]") {
  std::stringstream source(          //
      "fun f() Int { return 123; }"  //
      "f()"                          //
  );
  Parser p{lex::Lexer{source}};

  Evaluator e;
  e.Eval(p.ParseStatement());
  CHECK(e.Eval(p.ParseExpression()) == FromPrim(123));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Yield as break", "[ast]") {
  std::stringstream source(  //
      "{ yield 5;    3 - 2  }");
  //              -----------
  //              not executed
  Parser p{lex::Lexer{source}};

  Evaluator e;
  CHECK(e.Eval(p.ParseExpression()) == FromPrim(5));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("If statement (I)", "[ast]") {
  std::stringstream source(  //
      "{                                         "
      "      fun retval() Int {                  "
      "        if true {                         "
      "          return 1;                       "
      "        } else {                          "
      "          return 0;                       "
      "        }                                 "
      "      }                                   "
      "                                          "
      "      var a = retval();                   "
      "                                          "
      "      a                                   "
      "}                                         ");
  Parser p{lex::Lexer{source}};

  Evaluator e;
  CHECK(e.Eval(p.ParseExpression()) == FromPrim(1));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("If statement (II)", "[ast]") {
  std::stringstream source(  //
      "{                                         "
      "      fun negate(val: Bool) Bool {        "
      "        if val {                          "
      "          false                           "
      "        } else {                          "
      "          true                            "
      "        }                                 "
      "      }                                   "
      "                                          "
      "    var a = negate(true);                 "
      "                                          "
      "    a                                     "
      "}                                         ");
  Parser p{lex::Lexer{source}};

  Evaluator e;
  CHECK(e.Eval(p.ParseExpression()) == FromPrim(false));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Recursive", "[ast]") {
  std::stringstream source(  //
      "{                                         "
      "      fun sum(n: Int) Int {               "
      "         if n == 0 {                      "
      "             1                            "
      "         } else {                         "
      "             n + sum(n-1)                 "
      "         }                                "
      "      }                                   "
      "                                          "
      "      sum(4)                              "
      "}                                         ");
  Parser p{lex::Lexer{source}};

  Evaluator e;
  CHECK(e.Eval(p.ParseExpression()) == FromPrim(11));
}

//////////////////////////////////////////////////////////////////////

TEST_CASE("Test evaluation from checker", "[ast]") {
  std::stringstream source(  //
      "{                                         "
      "      fun sum(n: Bool) Bool {             "
      "         if n { return true; } else       "
      "              { return false; };          "
      "                                          "
      "         false                            "
      "      }                                   "
      "                                          "
      "      sum(true)                           "
      "}                                         ");
  Parser p{lex::Lexer{source}};

  Evaluator e;
  CHECK(e.Eval(p.ParseExpression()) == FromPrim(true));
}

//////////////////////////////////////////////////////////////////////