#include <types/type.hpp>

namespace types {

//////////////////////////////////////////////////////////////////////

std::string FormatStruct(Type& type) {
  std::string result;
  auto insert = std::back_inserter(result);
  fmt::format_to(insert, "struct {{ ");

  for (auto& a : type.as_struct.first) {
    fmt::format_to(insert, "{}: {}, ", a.field, FormatType(*a.ty));
  }

  fmt::format_to(insert, "}};");
  return result;
}

//////////////////////////////////////////////////////////////////////

std::string FormatApp(Type& type) {
  std::string result;
  auto insert = std::back_inserter(result);
  fmt::format_to(insert, "{}( ", type.as_tyapp.name.GetName());

  for (auto& a : type.as_tyapp.param_pack) {
    fmt::format_to(insert, "{}, ", FormatType(*a));
  }

  fmt::format_to(insert, ")");
  return result;
}

//////////////////////////////////////////////////////////////////////

std::string FormatFun(Type& type) {
  std::string result;
  auto insert = std::back_inserter(result);
  fmt::format_to(insert, "(");

  for (auto& a : type.as_fun.param_pack) {
    fmt::format_to(insert, " {} -> ", FormatType(*a));
  }

  fmt::format_to(insert, "{} )", FormatType(*type.as_fun.result_type));
  return result;
}

//////////////////////////////////////////////////////////////////////

std::string FormatPtr(Type& type) {
  return fmt::format("*{}", FormatType(*type.as_ptr.underlying));
}

//////////////////////////////////////////////////////////////////////

std::string FormatUnion(Type&) {
  return fmt::format("union {{ <unimplemented> }}");
}

//////////////////////////////////////////////////////////////////////

std::string FormatCons(Type& type) {
  return fmt::format(
      "Cons {} of {}", type.as_tycons.name.GetName(),
      type.as_tycons.kind ? FormatType(*type.as_tycons.kind) : "<kind>");
}

//////////////////////////////////////////////////////////////////////

std::string FormatType(Type& type) {
  if (type.leader) {
    return FormatType(*FindLeader(&type));
  }

  switch (type.tag) {
    case TypeTag::TY_KIND:
      return fmt::format("*");
    case TypeTag::TY_INT:
      return fmt::format("Int");
    case TypeTag::TY_BOOL:
      return fmt::format("Bool");
    case TypeTag::TY_CHAR:
      return fmt::format("Char");
    case TypeTag::TY_UNIT:
      return fmt::format("Unit");

    case TypeTag::TY_PTR:
      return FormatPtr(type);
    case TypeTag::TY_UNION:
      return FormatUnion(type);
    case TypeTag::TY_STRUCT:
      return FormatStruct(type);
    case TypeTag::TY_FUN:
      return FormatFun(type);
    case TypeTag::TY_CONS:
      return FormatCons(type);
    case TypeTag::TY_APP:
      return FormatApp(type);

    case TypeTag::TY_VARIABLE:
      return fmt::format("${}", type.id);

    case TypeTag::TY_PARAMETER:
      return fmt::format("G{}", type.id);

    default:
      std::abort();
  }
}

//////////////////////////////////////////////////////////////////////

};  // namespace types
