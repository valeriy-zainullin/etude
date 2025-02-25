#include <driver/compil_driver.hpp>
#include <ast/scope/context.hpp>
#include <types/type.hpp>
#include <ast/error_at_location.hpp>

namespace ast::scope {

//////////////////////////////////////////////////////////////////////

void ScopeLayer::InsertSymbol(Symbol symbol) {
  symbols.push_back(std::move(symbol));
  symbol_map.insert({
      symbols.back().name,
      &symbols.back(),
  });
}

//////////////////////////////////////////////////////////////////////

Context* Context::FindLayer(std::string_view name) {
  if (bindings.symbol_map.contains(name)) {
    return this;
  }
  return parent == nullptr ? nullptr : parent->FindLayer(name);
}

//////////////////////////////////////////////////////////////////////

Symbol* Context::RetrieveSymbol(std::string_view name, bool nothrow) {
  if (auto f = FindLocalSymbol(name)) {
    return f;
  }
  return FindFromExported(name, nothrow);
}

//////////////////////////////////////////////////////////////////////

Symbol* Context::FindLocalSymbol(std::string_view name) {
  if (bindings.symbol_map.contains(name)) {
    return bindings.symbol_map.at(name);
  }
  return parent == nullptr ? nullptr : parent->FindLocalSymbol(name);
}

//////////////////////////////////////////////////////////////////////

Symbol* Context::FindFromExported(std::string_view name, bool nothrow = false) {
  if (auto mod = driver->GetModuleOf(name)) {
    if (auto sym = mod->GetExportedSymbol(name)) {
      return sym;
    }
  }

  if (nothrow) {
    return nullptr;
  } else {
    Print();
    throw ErrorAtLocation(location, fmt::format("No such symbol {}\n", name));
  }
}

//////////////////////////////////////////////////////////////////////

Context* Context::MakeNewScopeLayer(lex::Location loc, std::string_view name) {
  auto child = new Context{
      .name = name,
      .unit = unit,
      .location = loc,
      .level = level + 1,
      .parent = this,
      .driver = driver,
  };
  children.push_back(child);
  return child;
}

//////////////////////////////////////////////////////////////////////

void Context::Print() {
  fmt::print(stderr, "[!] Context {} at {}, level {}\n", name, location.Format(),
             level);

  fmt::print(stderr, "Bindings: \n");
  for (auto& sym : bindings.symbols) {
    fmt::print("{}:{}\n", sym.FormatSymbol(),
               types::FormatType(*sym.GetType()));
  }

  fmt::print(stderr, "\n\n\n");

  for (auto& ch : children) {
    ch->Print();
  }
}

//////////////////////////////////////////////////////////////////////

Symbol* Context::RetrieveFromChild(std::string_view name) {
  if (bindings.symbol_map.contains(name)) {
    return bindings.symbol_map.at(name);
  }

  for (auto& ch : children) {
    if (auto s = ch->RetrieveFromChild(name))
      return s;
  }

  return nullptr;
}

}  // namespace ast::scope
