#pragma once
#include <format>
#include <unordered_map>
#include "ir.hh"

namespace lc3 {

using instruction_t = void(*)(uint16_t raw, uint16_t address, IRContext&);

static void add(uint16_t raw, uint16_t address, IRContext& ctx) {

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

static void ret(uint16_t raw, uint16_t address, IRContext& ctx) {

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
    {0b1100, ret},
    {0b0011, st},
    {0b1011, sti},
    {0b0111, str},
    {0b1111, trap}
};

} // End of namespace lc3