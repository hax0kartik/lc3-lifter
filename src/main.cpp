#include <iostream>
#include "disass.hh"

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cout << "File not found!" << std::endl;
        return -1;
    }

    lc3::Disassembler ds {};
    ds.run(std::string(argv[1]));

    return 0;
}