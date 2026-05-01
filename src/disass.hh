#pragma once
#include <cstdint>
#include <string_view>
#include <vector>
#include "rom.hh"
#include "instrs.hh"

namespace lc3 {

class Disassembler {
public:
    int run(const RomFile &rom, IRContext& ctx);
};

} // End of namespace lc3