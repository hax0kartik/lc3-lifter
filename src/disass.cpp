#include <iostream>
#include <iomanip>
#include "disass.hh"

namespace lc3 {

int Disassembler::run(const RomFile &rom, IRContext& ctx) {
    auto buffer = rom.program;

    for (size_t i = 0; i < ctx.blockList.size(); i++) {
        auto currBlock = ctx.blockList[i];

        for (size_t j = currBlock.fileOffset; j < buffer.size() - 1; j += 2) {
            uint16_t raw = buffer[j] << 8 | buffer[j + 1];
            uint8_t opcode = (raw >> 12) & 0xFF;

            auto f = INSMAP.find(opcode);
            if (f != INSMAP.end()) {
                // + 1 is needed here, because lc3 uses incremented pc
                f->second(raw, rom.loadAddr + (j / 2) + 1, ctx);

                if (ctx.done) {
                    break;
                }

            } else {
                std::cout << std::hex << std::setfill('0') << std::setw(4) << (int)raw << std::endl;
            }
        }
    }

    return 0;
}

} // End of namespace lc3