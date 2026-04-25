#include <iostream>
#include "ir.hh"
#include "disass.hh"

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cout << "File not found!" << std::endl;
        return -1;
    }

    lc3::IRContext ctx;
    ctx.Initialize();

    lc3::Disassembler ds {};
    ds.run(std::string(argv[1]), ctx);

    return 0;
}