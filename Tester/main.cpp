// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Tester.h"

#include <getopt.h>
#include <iostream>
#include <unistd.h>

static void printUsage(const char *progName)
{
    std::cerr << "Usage: " << progName << " [options] [hexfile]\n"
              << "Options:\n"
              << "  -f, --file FILE       Hex file to execute (default: program.hex)\n"
              << "  -s, --start ADDRESS   Start PC in decimal or 0x-prefixed hex\n"
              << "  -m, --max COUNT       Maximum number of instructions (default: 10)\n"
              << "  -v, --verbose         Enable verbose logging\n"
              << "  -h, --help            Show this help text\n";
}

int main(int argc, char **argv)
{
    moiracpu = new TestCPU();

    std::string hexFile = "program.hex";
    u32 startPC = 0x1600;
    bool verbose = false;
    int maxInstructions = 10;
    bool pcProvided = false;

    static struct option longOptions[] = {
        {"verbose", no_argument, nullptr, 'v'},
        {"file", required_argument, nullptr, 'f'},
        {"start", required_argument, nullptr, 's'},
        {"max", required_argument, nullptr, 'm'},
        {"help", no_argument, nullptr, 'h'},
        {nullptr, 0, nullptr, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "vf:s:m:h", longOptions, nullptr)) != -1) {
        switch (opt) {
            case 'v':
                verbose = true;
                break;
            case 'f':
                hexFile = optarg;
                break;
            case 's':
                startPC = std::stoul(optarg, nullptr, 0);
                pcProvided = true;
                break;
            case 'm':
                maxInstructions = std::stoi(optarg);
                break;
            case 'h':
                printUsage(argv[0]);
                return 0;
            default:
                printUsage(argv[0]);
                return 1;
        }
    }

    if (optind < argc) {
        hexFile = argv[optind++];
    }

    if (optind < argc) {
        std::cerr << "Unexpected argument: " << argv[optind] << "\n";
        printUsage(argv[0]);
        return 1;
    }

    
    moiracpu->setPCValue(startPC);
    

    VERBOSE = verbose;
    moiracpu->initTxFile("console_output.txt");
    runHexProgram(hexFile, maxInstructions);
    moiracpu->closeTxFile();
    return 0;
}
