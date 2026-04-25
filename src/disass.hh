#pragma once
#include <cstdint>
#include <string_view>
#include <vector>
#include "instrs.hh"

namespace lc3 {

class Disassembler {
    std::vector<uint8_t> _buffer;

public:
    int run(std::string_view vw, IRContext& ctx);
};

} // End of namespace lc3