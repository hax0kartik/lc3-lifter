#include <iostream>
#include <iomanip>
#include "disass.hh"

namespace lc3 {

int Disassembler::run(const RomFile &rom, IRContext& ctx) {
    auto buffer = rom.program;

    for (int i = 0; i < buffer.size() - 1; i += 2) {
        uint16_t raw = buffer[i] << 8 | buffer[i + 1];
        uint8_t opcode = (raw >> 12) & 0xFF;

        auto f = INSMAP.find(opcode);
        if (f != INSMAP.end()) {
            // + 1 is needed here, because lc3 uses incremented pc
            f->second(raw, rom.loadAddr + i + 1, ctx);
        } else {
            std::cout << std::hex << std::setfill('0') << std::setw(4) << (int)raw << std::endl;
        }
    }

    return 0;
}

} // End of namespace lc3