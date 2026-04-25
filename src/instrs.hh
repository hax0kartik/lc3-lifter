#pragma once
#include <format>
#include <unordered_map>
#include "ir.hh"

namespace lc3 {

struct Instruction {
    uint16_t raw;
};

using instruction_t = void(*)(Instruction&, IRContext&);

static void add(Instruction& info, IRContext& ctx) {

}

static void and_(Instruction& info, IRContext& ctx) {

}

static void jsr(Instruction& info, IRContext& ctx) {
    if ((info.raw >> 11) & 1) {
        /* JSR */
        uint16_t PCOffset = info.raw & 0x7FF;
        std::cout << std::format("JSR 0x{:X}", PCOffset) << std::endl;
    } else {
        /* JSRR */
        uint8_t BaseR = (info.raw >> 6) & 0x7;
        std::cout << std::format("JSRR 0x{:X}", BaseR) << std::endl;
    }
}

static void jmp(Instruction& info, IRContext& ctx) {
    
}

static void br(Instruction& info, IRContext& ctx) {
    uint8_t flag = (info.raw >> 9) & 0x7;
    uint8_t PCOffset = info.raw & 0x1FF;

    std::cout << std::format("BR{} 0x{:X}", flag, PCOffset) << std::endl;
}

static void ld(Instruction& info, IRContext& ctx) {
    uint8_t DR = (info.raw >> 9) & 0x7;
    uint8_t PCOffset = info.raw & 0x1FF;

    std::cout << std::format("LD R{} 0x{:X}", DR, PCOffset) << std::endl;
}

static void ldi(Instruction& info, IRContext& ctx) {

}

static void ldr(Instruction& info, IRContext& ctx) {
    uint8_t DR = (info.raw >> 9) & 0x7;
    // uint8_t PCOffset = info.raw & 0x1FF;

    // std::cout << "LDR DR: " << (int)DR << "PCOffset: " << (int)PCOffset << std::endl;
}

static void lea(Instruction& info, IRContext& ctx) {
    uint8_t DR = (info.raw >> 9) & 0x7;
    uint8_t PCOffset = info.raw & 0x1FF;

    std::cout << std::format("LEA R{} 0x{:X}", DR, PCOffset) << std::endl;
}

static void not_(Instruction& info, IRContext& ctx) {

}

static void ret(Instruction& info, IRContext& ctx) {

}

static void st(Instruction& info, IRContext& ctx) {

}

static void sti(Instruction& info, IRContext& ctx) {
    uint8_t SR = (info.raw >> 9) & 0x7;
    uint8_t PCOffset = info.raw & 0x1FF;

    std::cout << std::format("STI R{} 0x{:X}", SR, PCOffset) << std::endl;
}

static void str(Instruction& info, IRContext& ctx) {

}

static void trap(Instruction& info, IRContext& ctx) {
    uint8_t TrapVect8 = info.raw & 0xFF;

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