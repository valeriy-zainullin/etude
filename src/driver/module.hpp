#pragma once

#include <types/constraints/generate/algorithm_w.hpp>
#include <types/constraints/expand/expand.hpp>
#include <types/instantiate/instantiator.hpp>

#include <ast/elaboration/mark_intrinsics.hpp>
#include <ast/scope/context_builder.hpp>

#include <ast/visitors/visitor.hpp>
#include <ast/declarations.hpp>

#include <qbe/ir_emitter.hpp>

#include <lex/location.hpp>

#include <filesystem>

//////////////////////////////////////////////////////////////////////

class CompilationDriver;

class Module {
 public:
  friend class Parser;

  Module()
    : global_context(std::make_unique<ast::scope::Context>(ast::scope::Context{
      .name = std::string_view(),
      .unit = *this,
      .location = lex::Location{0, 0}
    }))
  {}

  Module(Module&&) = default;
  Module(const Module&) = delete;

  void SetName(std::string_view name) {
    name_ = name;

    namespace fs = std::filesystem;
    full_path = std::string(fs::absolute(fs::path(name_))) + ".et";
  }

  void BuildContext(CompilationDriver* driver) {
    global_context->driver = driver;

    ast::scope::ContextBuilder ctx_builder{*this, *global_context.get()};
    types::constraints::ExpandTypeVariables expand;

    for (auto item : items_) {
      item->Accept(&ctx_builder);
    }

    for (auto item : items_) {
      item->Accept(&expand);
    }
  }

  void MarkIntrinsics() {
    ast::elaboration::MarkIntrinsics mark;
    for (auto& r : items_) r = mark.Eval(r)->as<Declaration>();
  }

  void InferTypes(types::constraints::ConstraintSolver& solver) {
    solver.CollectAndSolve(items_);
  }

  auto CompileMain(Declaration* main) {
    types::instantiate::TemplateInstantiator inst(main);
    return inst.Flush();
  }

  auto CompileTests() {
    types::instantiate::TemplateInstantiator inst(tests_);
    return inst.Flush();
  }

  void Compile(Declaration* main) {
    auto [funs, gen_ty_list] = [&]() {
      return main ? CompileMain(main) : CompileTests();
    }();

    qbe::IrEmitter ir;
    ir.EmitTypes(std::move(gen_ty_list));

    for (auto f : funs) f->Accept(&ir);
  }

  std::string_view GetName() const {
    return name_;
  }

  std::vector<lex::Token> imports_;
  std::vector<std::string_view> exported_;

  ast::scope::Symbol* GetExportedSymbol(std::string_view name) {
    return global_context->FindLocalSymbol(name);
  }

  void RunTooling(Visitor* visitor) {
    for (Declaration* decl: items_) {
      decl->Accept(visitor);
    }
  }

  std::string GetPath() {
    return full_path;
  }

 private:
  std::string_view name_;
  std::string full_path;

  std::unique_ptr<ast::scope::Context> global_context;

  // Actual code item from this module
  std::vector<Declaration*> items_;

  // Functions that are marked @test
  std::vector<FunDeclStatement*> tests_;
};

//////////////////////////////////////////////////////////////////////
