#pragma once
#include <format>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/NoFolder.h>
#include <llvm/IR/LLVMContext.h>

using namespace llvm;

namespace lc3 {

struct IRContext {
    LLVMContext lctx;
    IRBuilder<NoFolder> builder {lctx};
    Module mod {"IR", lctx};

    void Initialize(std::span<uint8_t> program, const uint16_t addr) {
        // First create function
        auto ftype = FunctionType::get(builder.getVoidTy(), false);
        auto function = Function::Create(ftype, Function::ExternalLinkage, "main", mod);

        // Next create the basic startpoint
        auto entry = BasicBlock::Create(lctx, "entry", function);
        builder.SetInsertPoint(entry);

        auto *i16Type = builder.getInt16Ty();

        // Now setup the regs
        for (int i = 0; i < 8; i++) {
            auto reg = std::format("R{}", i);
            auto *global = new GlobalVariable(mod, i16Type, false, \
                GlobalValue::InternalLinkage, nullptr, reg);

            // Initial value should be 0
            global->setInitializer(ConstantInt::get(i16Type, 0));
        }

        // Now setup the memory
        // Memory - 2^16, 16 bit addressable
        auto *arrTy = ArrayType::get(i16Type, 65535);
        auto *global = new GlobalVariable(mod, arrTy, false, \
                GlobalValue::InternalLinkage, nullptr, "memory");

        std::vector<Constant*> mem(65535, ConstantInt::get(i16Type, 0));

        for (int i = 0, j = 0; j < program.size() - 1; j += 2, i++) {
            uint16_t by = program.data()[j] << 8 | program.data()[j + 1];
            mem[addr + i] = ConstantInt::get(i16Type, by);
        }

        auto *init = ConstantArray::get(arrTy, mem);
        global->setInitializer(init);
    }
};

} // End of namespace lc3