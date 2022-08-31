#pragma once

#include <vm/stack.hpp>
#include <vm/chunk.hpp>

#include <optional>

namespace vm {

class BytecodeInterpreter {
 public:
  void Interpret(const Instr* instruction) {
    switch (instruction->type) {
      case InstrType::PUSH_STACK: {
        size_t index = instruction->arg1;
        stack_.Push(Current()->attached_vals[index]);
        break;
      }

      case InstrType::POP_STACK: {
        stack_.Pop();
        break;
      }

      case InstrType::RET_FN: {
        // Obtain the return value
        eax = stack_.Pop();

        stack_.Ret();

        // Restore chunk
        current_chunk = stack_.Pop().as_int;

        // Restore ip
        ip_ = stack_.Pop().as_int;

        // Then the caller must clean up

        break;
      }

      case InstrType::CALL_FN: {
        // Note: Args have been placed

        // Push IP onto the stack
        stack_.Push(rt::PrimitiveValue{
            .tag = rt::ValueTag::Int,
            .as_int = (int)ip_,
        });

        // Push chunk number onto the stack
        stack_.Push(rt::PrimitiveValue{
            .tag = rt::ValueTag::Int,
            .as_int = (int)current_chunk,
        });

        // Create a new call frame
        stack_.PrepareCallframe();

        // Jmp into the function
        current_chunk = ReadByte(*instruction);
        ip_ = ReadWord(*instruction);

        break;
      }

      case InstrType::JUMP_IF_FALSE: {
        auto val = stack_.Pop();

        FMT_ASSERT(val.tag == rt::ValueTag::Bool,  //
                   "Typechecker fault");

        if (!val.as_bool) {
          ip_ = ReadWord(*instruction);
        }

        break;
      }

      case InstrType::ADD: {
        auto rhs = stack_.Pop();
        auto lhs = stack_.Pop();
        stack_.Push({
            .tag = rt::ValueTag::Int,
            .as_int = lhs.as_int + rhs.as_int,
        });
        break;
      }

        // Alias: GetArg
      case InstrType::FROM_STACK: {
        size_t offset = ReadByte(*instruction);
        auto arg = stack_.GetFnArg(offset);
        stack_.Push(arg);
        break;
      }

        // Alias: GetArg
      case InstrType::FIN_CALL: {
        size_t count = ReadByte(*instruction);
        stack_.PopCount(count);

        // Safety: teh value must be present in eax after the call
        stack_.Push(eax.value());

        break;
      }
    }
  }

  int Interpret() {
    while (auto instr = NextInstruction()) {
      stack_.PrintStack();
      Interpret(instr);
    }

    stack_.PrintStack();

    // Exit code
    return stack_.Pop().as_int;
  }

  static int InterpretStandalone(ExecutableChunk* chunk) {
    BytecodeInterpreter a;
    a.chunks = {*chunk};
    return a.Interpret();
  }

  static int InterpretStandalone(std::vector<ExecutableChunk> chunks) {
    BytecodeInterpreter a;
    a.chunks = std::move(chunks);
    return a.Interpret();
  }

 private:
  const ExecutableChunk* Current() const {
    return &chunks[current_chunk];
  }

  auto NextInstruction() const -> const Instr* {
    return ip_ < Current()->instructions.size()
               ? &Current()->instructions[ip_++]
               : nullptr;
  }

 private:
  // Instruction pointer
  mutable size_t ip_ = 0;

  vm::VmStack stack_;

  using Retval = std::optional<rt::PrimitiveValue>;
  // Return value (makes life easier)
  Retval eax;

  size_t current_chunk = 0;
  std::vector<ExecutableChunk> chunks;
};

}  // namespace vm