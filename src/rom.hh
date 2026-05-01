#pragma once
#include <cinttypes>
#include <string>
#include <vector>
#include <fstream>
#include <span>

namespace lc3 {

static std::vector<uint8_t> ReadObjectFile(std::string_view filename) {
    std::ifstream file(filename.data(), std::ios::binary);
    std::vector<uint8_t> buffer(std::istreambuf_iterator<char>(file), {});
    return buffer;
}

struct RomFile {
    std::string filename;
    std::vector<uint8_t> buffer;

    // These are just non owning copies into buffer
    std::span<uint8_t> program;
    uint16_t loadAddr;

    RomFile(const std::string& file) {
        filename = file;
    }

    // TODO: Add Error handling
    int ReadFile() {
        buffer = ReadObjectFile(filename);
        loadAddr = buffer[0] << 8 | buffer[1];
        program = std::span(&buffer[2], buffer.size() - 2);

        return 0;
    }
};

} // End of namespace lc3