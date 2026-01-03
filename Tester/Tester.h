// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>


#include "TestCPU.h"

extern "C" {

#include "m68k-dis.h"

}

#include "config.h"


using namespace moira;

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

extern class TestCPU *moiracpu;


extern u8 moiraMem[0x10000];

extern moira::Model cpuModel;
extern u16 opcode;

extern void logPrintf(const char* fmt, ...);

// Binutils
struct meminfo { unsigned char bytes[0x10000]; unsigned len; };
extern meminfo mi;
extern disassemble_info di;

inline u8 get8(u8 *p, u32 addr) {
    return p[addr & 0xFFFF];
}
inline u16 get16(u8 *p, u32 addr) {
    return (u16)(get8(p, addr) << 8 | get8(p, addr + 1));
}
inline void set8(u8 *p, u32 addr, u8 val) {
    p[addr & 0xFFFF] = val;
}
inline void set16(u8 *p, u32 addr, u16 val) {
    set8(p, addr, val >> 8); set8(p, addr + 1, val & 0xFF);
}



// A test result
struct Result {

    u16     dasmResult;
    u32     oldpc;
    u16     opcode;
    u32     pc;
    u32     usp;
    u32     isp;
    u32     msp;
    u32     d[8];
    u32     a[8];
    u16     sr;
    u32     fc;
    u32     vbr;
    u32     sfc;
    u32     dfc;
    u32     cacr;
    u32     caar;
    char    dasmMoira[128];
    int     dasmCntMoira;
    char    dasmBinutils[128];
    int     dasmCntBinutils;
    int     cycles;
    clock_t elapsed[2];
};






//
void runHexProgram(const std::string& filename, int maxInstructions = 10);




//
// Analysing a test result
//


void recordMoiraRegisters(Result &r);


