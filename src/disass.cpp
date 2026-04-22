#include <iostream>
#include <iomanip>
#include <fstream>
#include "disass.hh"

namespace lc3 {

static std::vector<uint8_t> ReadObjectFile(std::string_view filename) {
    std::ifstream file(filename.data(), std::ios::binary);
    std::vector<uint8_t> buffer(std::istreambuf_iterator<char>(file), {});
    return buffer;
}

int Disassembler::run(std::string_view vw) {
    _buffer = ReadObjectFile(vw);
    if (_buffer.empty()) {
        return -1;
    }

    uint16_t address = _buffer[0] << 8 | _buffer[1];

    Instruction ins;

    for (int i = 2; i < _buffer.size() - 1; i += 2) {
        uint16_t raw = _buffer[i] << 8 | _buffer[i + 1];
        ins.raw = raw;
        uint8_t opcode = (raw >> 12) & 0xFF;

        auto f = INSMAP.find(opcode);
        if (f != INSMAP.end()) {
            f->second(ins);
        } else {
            std::cout << std::hex << std::setfill('0') << std::setw(4) << (int)raw << std::endl;
        }
    }

    return 0;
}

} // End of namespace lc3