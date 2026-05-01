#include <iostream>
#include "rom.hh"
#include "ir.hh"
#include "disass.hh"

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cout << "File not found!" << std::endl;
        return -1;
    }

    lc3::RomFile file {std::string(argv[1])};
    file.ReadFile();

    lc3::IRContext ctx;
    ctx.Initialize(file.program, file.loadAddr);

    lc3::Disassembler ds {};
    ds.run(file, ctx);

    ctx.mod.dump();

    return 0;
}