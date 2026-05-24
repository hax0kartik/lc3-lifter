#pragma once
#include <format>
#include <unordered_map>
#include "ir.hh"

namespace lc3 {

using instruction_t = void(*)(uint16_t raw, uint16_t address, IRContext&);

// DR <- SR1 + SExt(imm5)
// DR <- SR1 + SR2
static void add(uint16_t raw, uint16_t address, IRContext& ctx) {
    uint8_t DR = (raw >> 9) & 0x7;
    uint8_t SR1 = (raw >> 6) & 0x7;

    auto& builder = ctx.builder;

    auto Sreg1 = ctx.mod->getNamedGlobal(std::format("R{}", SR1));
    auto Dreg = ctx.mod->getNamedGlobal(std::format("R{}", DR));
    
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

static void and_(uint16_t raw, uint16_t address, IRContext& ctx) {

}

static void jsr(uint16_t raw, uint16_t address, IRContext& ctx) {
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

static void jmp(uint16_t raw, uint16_t address, IRContext& ctx) {
    
}

static void br(uint16_t raw, uint16_t address, IRContext& ctx) {
    uint8_t flag = (raw >> 9) & 0x7;
    uint8_t PCOffset = raw & 0x1FF;

    std::cout << std::format("BR{} 0x{:X}", flag, PCOffset) << std::endl;
}

static void ld(uint16_t raw, uint16_t address, IRContext& ctx) {
    uint8_t DR = (raw >> 9) & 0x7;
    uint8_t PCOffset = raw & 0x1FF;

    uint16_t FinalOffset = address + PCOffset;

    auto& builder = ctx.builder;
    auto reg = ctx.mod->getNamedGlobal(std::format("R{}", DR));
    auto memory = ctx.mod->getNamedGlobal("memory");

    auto ptr = ctx.builder.CreateGEP(memory->getValueType(),
                memory, {ConstantInt::get(builder.getInt32Ty(), 0), ConstantInt::get(builder.getInt16Ty(), FinalOffset)});
            
    auto val = builder.CreateAlignedLoad(reg->getValueType(), ptr, Align(1));
    builder.CreateAlignedStore(val, reg, Align(1));

    std::cout << std::format("LD R{} 0x{:X}", DR, PCOffset) << std::endl;
}

static void ldi(uint16_t raw, uint16_t address, IRContext& ctx) {

}

static void ldr(uint16_t raw, uint16_t address, IRContext& ctx) {
    uint8_t DR = (raw >> 9) & 0x7;
    // uint8_t PCOffset = raw & 0x1FF;

    // std::cout << "LDR DR: " << (int)DR << "PCOffset: " << (int)PCOffset << std::endl;
}

static void lea(uint16_t raw, uint16_t address, IRContext& ctx) {
    uint8_t DR = (raw >> 9) & 0x7;
    uint8_t PCOffset = raw & 0x1FF;

    uint16_t FinalOffset = address + PCOffset;

    auto& builder = ctx.builder;
    auto reg = ctx.mod->getNamedGlobal(std::format("R{}", DR));
    builder.CreateAlignedStore(ConstantInt::get(builder.getInt16Ty(), FinalOffset), reg, Align(1));

    std::cout << std::format("LEA R{} 0x{:X}", DR, PCOffset) << std::endl;
}

static void not_(uint16_t raw, uint16_t address, IRContext& ctx) {

}

static void st(uint16_t raw, uint16_t address, IRContext& ctx) {

}

static void sti(uint16_t raw, uint16_t address, IRContext& ctx) {
    uint8_t SR = (raw >> 9) & 0x7;
    uint8_t PCOffset = raw & 0x1FF;

    std::cout << std::format("STI R{} 0x{:X}", SR, PCOffset) << std::endl;
}

static void str(uint16_t raw, uint16_t address, IRContext& ctx) {

}

static void trap(uint16_t raw, uint16_t address, IRContext& ctx) {
    uint8_t TrapVect8 = raw & 0xFF;

    switch (TrapVect8) {
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
            auto value = ConstantInt::get(ctx.builder.getInt16Ty(), 1); // STDOUT

            ctx.builder.CreateCall(func, {ptr, value});
            break;
        }

        case 0x25: { // HALT
            ctx.builder.CreateRetVoid();
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