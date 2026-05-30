#pragma once
#include <format>
#include <unordered_map>
#include "ir.hh"

namespace lc3 {

using instruction_t = void(*)(uint16_t raw, size_t fileOffset, uint16_t address, IRContext&);

// DR <- SR1 + SExt(imm5)
// DR <- SR1 + SR2
static void add(uint16_t raw, size_t fileOffset, uint16_t address, IRContext& ctx) {
    (void)address;
    (void)fileOffset;

    uint8_t DR = (raw >> 9) & 0x7;
    uint8_t SR1 = (raw >> 6) & 0x7;

    auto& builder = ctx.builder;

    auto Sreg1 = ctx.mod->getNamedGlobal(std::format("R{}", SR1));
    auto Dreg = ctx.mod->getNamedGlobal(std::format("R{}", DR));
    ctx.lastReg = Dreg;

    if ((raw >> 5) & 0x1) {
        uint8_t imm5 = raw & 0x1F;
        uint16_t num = (imm5 ^ 0x10) - 0x10; // sign extend to 16 bits

        auto val1 = builder.CreateAlignedLoad(Sreg1->getValueType(), Sreg1, Align(1));
        auto v = builder.CreateAdd(val1, ConstantInt::get(ctx.builder.getInt16Ty(), num));

        builder.CreateAlignedStore(v, Dreg, Align(1));
    } else {
        uint8_t SR2 = raw & 0x7;
        auto Sreg2 = ctx.mod->getNamedGlobal(std::format("R{}", SR2));

        auto val1 = builder.CreateAlignedLoad(Sreg1->getValueType(), Sreg1, Align(1));
        auto val2 = builder.CreateAlignedLoad(Sreg2->getValueType(), Sreg2, Align(1));
        auto v = builder.CreateAdd(val1, val2);

        builder.CreateAlignedStore(v, Dreg, Align(1));
    }
}

// DR <- SR1 & SExt(imm5)
// DR <- SR1 & SR2
static void and_(uint16_t raw, size_t fileOffset, uint16_t address, IRContext& ctx) {
    (void)address;
    (void)fileOffset;

    uint8_t DR = (raw >> 9) & 0x7;
    uint8_t SR1 = (raw >> 6) & 0x7;

    auto& builder = ctx.builder;

    auto Sreg1 = ctx.mod->getNamedGlobal(std::format("R{}", SR1));
    auto Dreg = ctx.mod->getNamedGlobal(std::format("R{}", DR));
    ctx.lastReg = Dreg;

    if ((raw >> 5) & 0x1) {
        uint8_t imm5 = raw & 0x1F;
        int16_t num = (imm5 ^ 0x10) - 0x10; // sign extend to 16 bits

        auto val1 = builder.CreateAlignedLoad(Sreg1->getValueType(), Sreg1, Align(1));
        auto v = builder.CreateAnd(val1, ConstantInt::get(ctx.builder.getInt16Ty(), num));

        builder.CreateAlignedStore(v, Dreg, Align(1));
    } else {
        uint8_t SR2 = raw & 0x7;
        auto Sreg2 = ctx.mod->getNamedGlobal(std::format("R{}", SR2));

        auto val1 = builder.CreateAlignedLoad(Sreg1->getValueType(), Sreg1, Align(1));
        auto val2 = builder.CreateAlignedLoad(Sreg2->getValueType(), Sreg2, Align(1));
        auto v = builder.CreateAnd(val1, val2);

        builder.CreateAlignedStore(v, Dreg, Align(1));
    }
}

static void jsr(uint16_t raw, size_t fileOffset, uint16_t address, IRContext& ctx) {
    (void)address;
    (void)fileOffset;
    (void)ctx;

    if ((raw >> 11) & 1) {
        /* JSR */
        uint16_t PCOffset = raw & 0x7FF;
        std::cout << std::format("JSR 0x{:X}", PCOffset) << std::endl;
    } else {
        /* JSRR */
        uint8_t BaseR = (raw >> 6) & 0x7;
        std::cout << std::format("JSRR 0x{:X}", BaseR) << std::endl;
    }
}

static void jmp(uint16_t raw, size_t fileOffset, uint16_t address, IRContext& ctx) {
    (void)raw;
    (void)fileOffset;
    (void)address;
    (void)ctx;
}

static void br(uint16_t raw, size_t fileOffset, uint16_t address, IRContext& ctx) {
    uint8_t flag = (raw >> 9) & 0x7; // N Z P
    uint8_t PCOffset = raw & 0x1FF;

    uint16_t SExtOffset = (PCOffset ^ 0x100) - 0x100;
    uint16_t FinalOffset = address + SExtOffset;

    auto& builder = ctx.builder;

    auto BBInsert = [&](std::string name, uint16_t off) -> BasicBlock * {
        if (ctx.blocks.find(off) != ctx.blocks.end()) {
            return ctx.blocks[off];
        }

        auto *currFunc = builder.GetInsertBlock()->getParent();
        auto *block = BasicBlock::Create(ctx.lctx, name, currFunc);
        ctx.blocks[off] = block;
        return block;
    };

    // Create a true block
    auto *trueEntry = BBInsert(std::format("br_true_{:X}", FinalOffset), FinalOffset);

    // Create a false block
    BasicBlock *falseEntry = nullptr;

    auto FalseBlockCreate = [&](char c) -> void {
        if (falseEntry) {
            builder.SetInsertPoint(falseEntry);
        }

        falseEntry = BBInsert(std::format("br_false_{}_{:X}", c, address), address);
    };

    if ((flag & 0x1) == 0x1) { // p (positive) -> cmp > 0
        FalseBlockCreate('p');

        // Load last known register
        auto v = builder.CreateAlignedLoad(ctx.lastReg->getValueType(), ctx.lastReg, Align(1));
        auto cmp = builder.CreateICmpSGT(v, ConstantInt::get(ctx.builder.getInt16Ty(), 0));
        builder.CreateCondBr(cmp, trueEntry, falseEntry);
    }

    if ((flag & 0x2) == 0x2) { // z (zero) -> cmp == 0
        FalseBlockCreate('z');

        // Load last known register
        auto v = builder.CreateAlignedLoad(ctx.lastReg->getValueType(), ctx.lastReg, Align(1));
        auto cmp = builder.CreateICmpEQ(v, ConstantInt::get(ctx.builder.getInt16Ty(), 0));
        builder.CreateCondBr(cmp, trueEntry, falseEntry);
    }

    if ((flag & 0x4) == 0x4) { // n (negative) -> cmp < 0
        FalseBlockCreate('n');

        // Load last known register
        auto v = builder.CreateAlignedLoad(ctx.lastReg->getValueType(), ctx.lastReg, Align(1));
        auto cmp = builder.CreateICmpSLT(v, ConstantInt::get(ctx.builder.getInt16Ty(), 0));
        builder.CreateCondBr(cmp, trueEntry, falseEntry);
    }

    if (flag != 0) { // if just BR -> unconditional jump, so no false block
        // pc is already incremented by 1
        // +2 because file buffer is u8 and not u16
        ctx.blockList.push_back({address, fileOffset + 2, falseEntry});
    } else {
        builder.CreateBr(trueEntry);
    }

    ctx.blockList.push_back({FinalOffset, (FinalOffset - 0x3000) * 2, trueEntry});

    // mark end of current block
    ctx.done = true;

    std::cout << std::format("BR{} 0x{:X}", flag, PCOffset) << std::endl;
}

static void ld(uint16_t raw, size_t fileOffset, uint16_t address, IRContext& ctx) {
    (void)fileOffset;

    uint8_t DR = (raw >> 9) & 0x7;
    uint8_t PCOffset = raw & 0x1FF;

    uint16_t SExtOffset = (PCOffset ^ 0x100) - 0x100;
    uint16_t FinalOffset = address + SExtOffset;

    auto& builder = ctx.builder;
    auto reg = ctx.mod->getNamedGlobal(std::format("R{}", DR));
    auto memory = ctx.mod->getNamedGlobal("memory");
    ctx.lastReg = reg;

    auto ptr = ctx.builder.CreateGEP(memory->getValueType(),
                memory, {ConstantInt::get(builder.getInt32Ty(), 0), ConstantInt::get(builder.getInt16Ty(), FinalOffset)});
            
    auto val = builder.CreateAlignedLoad(reg->getValueType(), ptr, Align(1));
    builder.CreateAlignedStore(val, reg, Align(1));

    std::cout << std::format("LD R{} 0x{:X}", DR, PCOffset) << std::endl;
}

// DR <- MEM[MEM[PC + SExt(Offset9)]]
static void ldi(uint16_t raw, size_t fileOffset, uint16_t address, IRContext& ctx) {
    (void)fileOffset;

    uint8_t DR = (raw >> 9) & 0x7;
    uint8_t PCOffset = raw & 0x1FF;

    uint16_t SExtOffset = (PCOffset ^ 0x100) - 0x100;

    uint16_t FinalOffset = address + SExtOffset;

    auto& builder = ctx.builder;
    auto Dreg = ctx.mod->getNamedGlobal(std::format("R{}", DR));
    auto memory = ctx.mod->getNamedGlobal("memory");
    ctx.lastReg = Dreg;

    auto ptr = builder.CreateGEP(memory->getValueType(),
        memory, {ConstantInt::get(builder.getInt32Ty(), 0), ConstantInt::get(builder.getInt32Ty(), FinalOffset)});
    
    auto val = builder.CreateAlignedLoad(Dreg->getValueType(), ptr, Align(1));

    auto ptr2 = builder.CreateGEP(memory->getValueType(),
        memory, {ConstantInt::get(builder.getInt32Ty(), 0), val});
    
    auto val2 = builder.CreateAlignedLoad(Dreg->getValueType(), ptr2, Align(1));
    builder.CreateAlignedStore(val2, Dreg, Align(1));

    std::cout << std::format("LDR R{} 0x{:X}", DR, PCOffset) << std::endl;
}

// DR <- MEM[BaseR + SExt(offset)]
static void ldr(uint16_t raw, size_t fileOffset, uint16_t address, IRContext& ctx) {
    (void)address;
    (void)fileOffset;

    uint8_t DR = (raw >> 9) & 0x7;
    uint8_t BaseR = (raw >> 6) & 0x7;
    int8_t offset = (((raw & 0x3F) ^ 0x20) - 0x20);

    auto& builder = ctx.builder;

    auto Dreg = ctx.mod->getNamedGlobal(std::format("R{}", DR));
    ctx.lastReg = Dreg;

    auto Breg = ctx.mod->getNamedGlobal(std::format("R{}", BaseR));
    auto memory = ctx.mod->getNamedGlobal("memory");

    auto val = builder.CreateAlignedLoad(Breg->getValueType(), Breg, Align(1));
    auto v = builder.CreateAdd(val, ConstantInt::get(builder.getInt16Ty(), offset));

    auto ptr = builder.CreateGEP(memory->getValueType(),
            memory, {ConstantInt::get(builder.getInt32Ty(), 0), v});
    auto fval = builder.CreateAlignedLoad(Dreg->getValueType(), ptr, Align(1));
    builder.CreateAlignedStore(fval, Dreg, Align(1));

    std::cout << "LDR DR: " << (int)DR << "Offset: " << (int)offset << "BaseR: " << BaseR << std::endl;
}

static void lea(uint16_t raw, size_t fileOffset, uint16_t address, IRContext& ctx) {
    (void)fileOffset;

    uint8_t DR = (raw >> 9) & 0x7;
    uint8_t PCOffset = raw & 0x1FF;

    uint16_t FinalOffset = address + PCOffset;

    auto& builder = ctx.builder;
    auto reg = ctx.mod->getNamedGlobal(std::format("R{}", DR));
    builder.CreateAlignedStore(ConstantInt::get(builder.getInt16Ty(), FinalOffset), reg, Align(1));

    std::cout << std::format("LEA R{} 0x{:X}", DR, PCOffset) << std::endl;
}

// DR <- NOT(SR)
static void not_(uint16_t raw, size_t fileOffset, uint16_t address, IRContext& ctx) {
    (void)address;
    (void)fileOffset;

    uint8_t DR = (raw >> 9) & 0x7;
    uint8_t SR = (raw >> 6) & 0x7;

    auto& builder = ctx.builder;
    auto Dreg = ctx.mod->getNamedGlobal(std::format("R{}", DR));
    auto Sreg = ctx.mod->getNamedGlobal(std::format("R{}", SR));

    ctx.lastReg = Dreg;

    auto val = builder.CreateAlignedLoad(Sreg->getValueType(), Sreg, Align(1));
    auto fval = builder.CreateNot(val);

    builder.CreateAlignedStore(fval, Dreg, Align(1));

    std::cout << std::format("NOT D: R{} S: R{}", DR, SR) << std::endl;
}

// MEM[PC + SExt(PCOffset9)] = SR
static void st(uint16_t raw, size_t fileOffset, uint16_t address, IRContext& ctx) {
    (void)fileOffset;

    uint8_t SR = (raw >> 9) & 0x7;
    uint8_t PCOffset = raw & 0x1FF;
    uint16_t SExtOffset = (PCOffset ^ 0x100) - 0x100;

    uint16_t FinalOffset = address + SExtOffset;

    auto& builder = ctx.builder;
    auto Sreg = ctx.mod->getNamedGlobal(std::format("R{}", SR));
    auto memory = ctx.mod->getNamedGlobal("memory");

    auto ptr = builder.CreateGEP(memory->getValueType(),
            memory, {ConstantInt::get(builder.getInt32Ty(), 0), ConstantInt::get(builder.getInt16Ty(), FinalOffset)});

    auto val = builder.CreateAlignedLoad(Sreg->getValueType(), Sreg, Align(1));
    
    builder.CreateAlignedStore(val, ptr, Align(1));

    std::cout << std::format("ST R{} 0x{:X}", SR, PCOffset) << std::endl;
}

// MEM[MEM[PC + SExt(PCOffset9)]] = SR
static void sti(uint16_t raw, size_t fileOffset, uint16_t address, IRContext& ctx) {
    (void)fileOffset;

    uint8_t SR = (raw >> 9) & 0x7;
    uint8_t PCOffset = raw & 0x1FF;
    uint16_t SExtOffset = (PCOffset ^ 0x100) - 0x100;

    uint16_t FinalOffset = address + SExtOffset;

    auto& builder = ctx.builder;
    auto Sreg = ctx.mod->getNamedGlobal(std::format("R{}", SR));
    auto memory = ctx.mod->getNamedGlobal("memory");

    auto ptr = builder.CreateGEP(memory->getValueType(),
            memory, {ConstantInt::get(builder.getInt32Ty(), 0), ConstantInt::get(builder.getInt16Ty(), FinalOffset)});
    
    auto val = builder.CreateAlignedLoad(Sreg->getValueType(), ptr, Align(1));
    
    auto ptr2 = builder.CreateGEP(memory->getValueType(),
            memory, {ConstantInt::get(builder.getInt32Ty(), 0), val});

    auto val2 = builder.CreateAlignedLoad(Sreg->getValueType(), Sreg, Align(1));
    
    builder.CreateAlignedStore(val2, ptr2, Align(1));

    std::cout << std::format("STI R{} 0x{:X}", SR, PCOffset) << std::endl;
}

// MEM[BaseR + SExt(offset6)] = SR
static void str(uint16_t raw, size_t fileOffset, uint16_t address, IRContext& ctx) {
    (void)address;
    (void)fileOffset;

    uint8_t SR = (raw >> 9) & 0x7;
    uint8_t BaseR = (raw >> 6) & 0x7;
    int8_t offset6 = (((raw & 0x3F) ^ 0x20) - 0x20);

    auto& builder = ctx.builder;
    auto Sreg = ctx.mod->getNamedGlobal(std::format("R{}", SR));
    auto Breg = ctx.mod->getNamedGlobal(std::format("R{}", BaseR));
    auto memory = ctx.mod->getNamedGlobal("memory");

    auto val = builder.CreateAlignedLoad(Breg->getValueType(), Breg, Align(1));
    auto v = builder.CreateAdd(val, ConstantInt::get(builder.getInt16Ty(), offset6));
    
    auto ptr2 = builder.CreateGEP(memory->getValueType(),
            memory, {ConstantInt::get(builder.getInt32Ty(), 0), v});

    auto val2 = builder.CreateAlignedLoad(Sreg->getValueType(), Sreg, Align(1));
    builder.CreateAlignedStore(val2, ptr2, Align(1));

    std::cout << std::format("STR R{} BaseR{} 0x{:X}", SR, BaseR, offset6) << std::endl;
}

static void trap(uint16_t raw, size_t fileOffset, uint16_t address, IRContext& ctx) {
    (void)address;
    (void)fileOffset;

    uint8_t TrapVect8 = raw & 0xFF;

    switch (TrapVect8) {
        case 0x20: { // GETC
            auto func = ctx.mod->getFunction("_getc");
            auto f = ctx.builder.CreateCall(func, {});

            auto R0 = ctx.mod->getNamedGlobal("R0");
            ctx.builder.CreateAlignedStore(f, R0, Align(1));
            break;
        }

        case 0x21: { // OUT
            auto R0 = ctx.mod->getNamedGlobal("R0");
            auto load = ctx.builder.CreateAlignedLoad(R0->getValueType(), R0, Align(1));
            
            auto func = ctx.mod->getFunction("_out");
            ctx.builder.CreateCall(func, {load});
            break;
        }

        case 0x22: { // PUTS
            // Use GEP to call external function
            auto R0 = ctx.mod->getNamedGlobal("R0");
            auto memory = ctx.mod->getNamedGlobal("memory");

            auto load = ctx.builder.CreateAlignedLoad(R0->getValueType(), R0, Align(1));
            auto ptr = ctx.builder.CreateGEP(memory->getValueType(),
                memory, {ConstantInt::get(ctx.builder.getInt32Ty(), 0), load});

            auto func = ctx.mod->getFunction("_fputws");
            ctx.builder.CreateCall(func, {ptr});
            break;
        }

        case 0x25: { // HALT
            ctx.builder.CreateRetVoid();
            ctx.done = true;
            break;
        }
    }

    std::cout << std::format("TRAP 0x{:X}", TrapVect8) << std::endl;
}

static const std::unordered_map<uint8_t, instruction_t> INSMAP = {
    {0b0001, add},
    {0b0101, and_},
    {0b0000, br},
    {0b1100, jmp},
    {0b0100, jsr},
    {0b0010, ld},
    {0b1010, ldi},
    {0b0110, ldr},
    {0b1110, lea},
    {0b1001, not_},
    {0b0011, st},
    {0b1011, sti},
    {0b0111, str},
    {0b1111, trap}
};

} // End of namespace lc3