#include <types/instantiate/instantiator.hpp>
#include <types/constraints/solver.hpp>

#include <ast/patterns.hpp>

#include <lex/token.hpp>

namespace types::instantiate {

//////////////////////////////////////////////////////////////////////

bool TemplateInstantiator::TryFindInstantiation(FnCallExpression* i) {
  auto range = mono_items_.equal_range(i->GetFunctionName());

  for (auto it = range.first; it != range.second; ++it) {
    if (TypesEquivalent(it->second->type_, i->callable_type_)) {
      return true;
    }
  }
  return false;
}

//////////////////////////////////////////////////////////////////////

void TemplateInstantiator::StartUp(FunDeclStatement* main) {
  call_context_ = main->layer_;
  auto main_fn = Eval(main)->as<FunDeclStatement>();
  mono_items_.insert({main_fn->name_, main_fn});
}

//////////////////////////////////////////////////////////////////////

void TemplateInstantiator::ProcessQueueItem(FnCallExpression* i) {
  // 1) Check not already instantiated

  if (TryFindInstantiation(i)) {
    return;
  }

  auto symbol = i->layer_->RetrieveSymbol(i->fn_name_);

  if (symbol->sym_type != ast::scope::SymbolType::FUN) {
    return;
  }

  // 2) Enter context

  auto poly = symbol->GetType();
  auto mono = i->callable_type_;

  fmt::print(stderr, "[!] Poly {}\n", FormatType(*poly));
  fmt::print(stderr, "[!] Mono {}\n", FormatType(*mono));

  current_substitution_.clear();
  BuildSubstitution(poly, mono, current_substitution_);

  // 3) Find definition

  auto definition = symbol->as_fn_sym.def;
  call_context_ = i->layer_;

  // 4) Evaluate

  auto mono_fun = Eval(definition)->as<FunDeclStatement>();

  // 5) Save result
  if (mono_fun->body_) {
    mono_items_.insert({mono_fun->name_, mono_fun});
  }
}

//////////////////////////////////////////////////////////////////////

void TemplateInstantiator::ProcessQueue() {
  while (instantiation_quque_.size()) {
    auto i = instantiation_quque_.front();
    ProcessQueueItem(i);
    instantiation_quque_.pop_front();
  }
}

//////////////////////////////////////////////////////////////////////

TemplateInstantiator::TemplateInstantiator(Declaration* main) {
  StartUp(main->as<FunDeclStatement>());

  fmt::print(stderr, "Finished processing main\n");

  ProcessQueue();
}

//////////////////////////////////////////////////////////////////////

TemplateInstantiator::TemplateInstantiator(
    std::vector<FunDeclStatement*>& tests) {
  for (auto& test : tests) {
    StartUp(test->as<FunDeclStatement>());
  }

  fmt::print(stderr, "Finished processing tests n");

  ProcessQueue();
}

//////////////////////////////////////////////////////////////////////

using Result = std::pair<std::vector<FunDeclStatement*>, std::vector<Type*>>;

auto TemplateInstantiator::Flush() -> Result {
  std::vector<FunDeclStatement*> result;

  for (auto& mono : mono_items_) {
    fmt::print(stderr, "name: {} type: {}\n",  //
               mono.second->GetName(), FormatType(*mono.second->type_));

    result.push_back(mono.second);
  }

  return {std::move(result), std::move(types_to_gen_)};
}

//////////////////////////////////////////////////////////////////////

void TemplateInstantiator::BuildSubstitution(
    Type* poly, Type* mono, std::unordered_map<Type*, Type*>& poly_to_mono) {
  poly = FindLeader(poly);
  mono = FindLeader(mono);

  switch (poly->tag) {
    case TypeTag::TY_PTR:
      FMT_ASSERT(mono->tag == TypeTag::TY_PTR, "Mismatch");
      BuildSubstitution(poly->as_ptr.underlying, mono->as_ptr.underlying,
                        poly_to_mono);
      break;

    case TypeTag::TY_STRUCT: {
      auto& a_mem = poly->as_struct.first;
      auto& b_mem = mono->as_struct.first;

      if (a_mem.size() != b_mem.size()) {
        throw std::runtime_error{"Inference error: struct size mismatch"};
      }

      for (size_t i = 0; i < a_mem.size(); i++) {
        BuildSubstitution(a_mem[i].ty, b_mem[i].ty, poly_to_mono);
      }

      break;
    }

    case TypeTag::TY_FUN: {
      auto& pack = poly->as_fun.param_pack;
      auto& pack2 = mono->as_fun.param_pack;

      if (pack.size() != pack2.size()) {
        throw std::runtime_error{"Function unification size mismatch"};
      }

      for (size_t i = 0; i < pack.size(); i++) {
        BuildSubstitution(pack[i], pack2[i], poly_to_mono);
      }

      BuildSubstitution(poly->as_fun.result_type, mono->as_fun.result_type,
                        poly_to_mono);
      break;
    }

    case TypeTag::TY_APP: {
      if (poly->as_tyapp.name.GetName() != mono->as_tyapp.name) {
        std::abort();
      }

      auto& a_pack = poly->as_tyapp.param_pack;
      auto& b_pack = mono->as_tyapp.param_pack;

      for (size_t i = 0; i < a_pack.size(); i++) {
        BuildSubstitution(a_pack[i], b_pack[i], poly_to_mono);
      }

      break;
    }

      // G13 -> Int
    case TypeTag::TY_PARAMETER:
      // This function exists for this callback
      current_substitution_.insert({poly, mono});
      break;

    case TypeTag::TY_CONS:
    case TypeTag::TY_KIND:
    case TypeTag::TY_VARIABLE:
    case TypeTag::TY_UNION:
      FMT_ASSERT(false, "Unreachable");

    default:
      break;
  }
}

//////////////////////////////////////////////////////////////////////

void TemplateInstantiator::MaybeSaveForIL(Type* ty) {
  CheckTypes();

  if (ty->tag != TypeTag::TY_APP && ty->tag != TypeTag::TY_STRUCT) {
    return;
  }

  auto storage = TypeStorage(ty);

  if (storage->tag <= TypeTag::TY_PTR) {
    return;
  }

  types_to_gen_.push_back(ty);
}

//////////////////////////////////////////////////////////////////////

}  // namespace types::instantiate
