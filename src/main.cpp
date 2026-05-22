#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/Error.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <iostream>
#include "rom.hh"
#include "ir.hh"
#include "disass.hh"

extern "C" void _fputws(char16_t *ptr, uint16_t a) {
    // Can you believe we do not have a direct function to print
    // char_16t in 2026? Me neither. 
    std::string out;
    while (*ptr != 0) {
        // Meh, good enough for now. Assume, *ptr < 0xFF
        out.push_back(*ptr);
        ptr++;
    }

    std::cout << out << std::endl;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cout << "File not found!" << std::endl;
        return -1;
    }

    lc3::RomFile file {std::string(argv[1])};
    file.ReadFile();

    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    auto JITOrErr = llvm::orc::LLJITBuilder().create();

    if (!JITOrErr) {
        llvm::logAllUnhandledErrors(
            JITOrErr.takeError(),
            llvm::errs(),
            "LLJIT creation failed: ");
        return 1;
    }

    auto JIT = std::move(*JITOrErr);

    lc3::IRContext ctx;

    auto M = std::make_unique<llvm::Module>("jit", ctx.lctx);
    ctx.mod = M.get();

    ctx.Initialize(file.program, file.loadAddr);
    ctx.InsertExternalFuncs();

    lc3::Disassembler ds {};
    ds.run(file, ctx);

    ctx.mod->dump();

    auto &JD = JIT->getMainJITDylib();

    llvm::orc::SymbolMap map;

    map[JIT->mangleAndIntern("_fputws")] =
        llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr((uint64_t)&_fputws),
            JITSymbolFlags::Exported);

    JD.define(llvm::orc::absoluteSymbols(std::move(map)));

    JD.addGenerator(
        cantFail(llvm::orc::DynamicLibrarySearchGenerator::GetForCurrentProcess(
            JIT->getDataLayout().getGlobalPrefix()))
    );

    auto TSM = llvm::orc::ThreadSafeModule(
        std::move(M),
        std::make_unique<llvm::LLVMContext>());

    if (auto Err = JIT->addIRModule(std::move(TSM))) {
        llvm::logAllUnhandledErrors(std::move(Err), llvm::errs());
        return 1;
    }

    // Lookup entry
    auto Sym = JIT->lookup("main");

    if (!Sym) {
        llvm::logAllUnhandledErrors(Sym.takeError(), llvm::errs());
        return 1;
    }

    auto fn = (void (*)()) Sym->getValue();
    fn();

    return 0;
}