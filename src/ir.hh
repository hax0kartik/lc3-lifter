#pragma once
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/NoFolder.h>
#include <llvm/IR/LLVMContext.h>

using namespace llvm;

namespace lc3 {

struct IRContext {
    LLVMContext lcntxt;
    IRBuilder<NoFolder> builder {lcntxt};
    Module mod {"IR", lcntxt};

    void Initialize() {
        // First create function
        auto ftype = FunctionType::get(builder.getVoidTy(), false);
        auto function = Function::Create(ftype, Function::ExternalLinkage, "main", mod);

        // Next create the basic startpoint
        auto entry = BasicBlock::Create(lcntxt, "entry", function);
        builder.SetInsertPoint(entry);
    }
};

} // End of namespace lc3