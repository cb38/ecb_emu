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
#include "TestCPU.h"

FILE* TestCPU::txFile = nullptr;
u32 TestCPU::timer = 0;
u32 TestCPU::timer_init = 0;
bool TestCPU::aciaEnabled = false;
bool TestCPU::timerEnabled = false;
int TestCPU::prescaler = 0;

/* Signals the CPU clock to advance.
 */
void
TestCPU::sync(int cycles)
{
    clock += cycles;
    // decrement prescaler by 32
    if (prescaler >= 0 && (TestCPU::timerEnabled) ) {
        TestCPU::prescaler -= cycles;

    }
    if (prescaler < 0) {
        // One timer tick
        TestCPU::timer--;
        TestCPU::prescaler += 32;
    }
    // timer reach 0 ; reload from timer_init and generate interrupt
    if (TestCPU::timer <= 0) {
        TestCPU::timer = TestCPU::timer_init;
        // signal timer interrupt 
       if (TestCPU::timerEnabled) {
           moiracpu->setIPL(5);
       }
    }    
}

/* Reads a byte from memory.
 *
 * This function is called whenever the 68000 CPU reads a byte from memory.
 * It should emulate the read access including all side effects.
 */
u8
TestCPU::read8(u32 addr) const
{
    switch (addr) {
        case 0x0C0080: logPrintf("[IO] Read ACIA Status register at $%06X\n", addr); 
            return 2 ; // TDR empty 
                break;
        case 0x0C0082: logPrintf("[IO] Read ACIA DATA/RX/TX register at $%06X\n", addr); exit(1); break;
        case 0x0E0027: logPrintf("[IO] Read timerPrH at $%06X\n", addr); exit(1); break;
        case 0x0E0029: logPrintf("[IO] Read timerPrM at $%06X\n", addr); exit(1); break;
        case 0x0E002B: logPrintf("[IO] Read timerPrL at $%06X\n", addr); exit(1); break;
        case 0x0E0021: logPrintf("[IO] Read timerCR at $%06X\n", addr); exit(1); break;
        case 0x0E0035: 
                logPrintf("[IO] Read timerSR at $%06X\n", addr);  break;
    }
    return get8(moiraMem, addr);
}

/* Reads a word from memory.
 *
 * This function is called whenever the 68000 CPU reads a word from memory.
 * It should emulate the read access including all side effects.
 */
u16
TestCPU::read16(u32 addr) const
{
    switch (addr) {
        case 0x0C0080: logPrintf("[IO] Read16 ACIA Status register at $%06X\n", addr); exit(1); break;
        case 0x0C0082: logPrintf("[IO] Read16 ACIA DATA/RX register at $%06X\n", addr); exit(1); break;
        case 0x0E0027: logPrintf("[IO] Read16 timerPrH at $%06X\n", addr); exit(1); break;
        case 0x0E0029: logPrintf("[IO] Read16 timerPrM at $%06X\n", addr); exit(1); break;
        case 0x0E002B: logPrintf("[IO] Read16 timerPrL at $%06X\n", addr); exit(1); break;
        case 0x0E0021: logPrintf("[IO] Read16 timerCR at $%06X\n", addr); exit(1); break;
        case 0x0E0035: logPrintf("[IO] Read16 timerSR at $%06X\n", addr); exit(1); break;
    }
    return get16(moiraMem, addr);
}

/* Reads a word from memory.
 *
 * This function is called by the disassembler to read a word from memory.
 * In contrast to read16, no side effects should be emulated.
 */
u16
TestCPU::read16Dasm(u32 addr) const
{
    return get16(moiraMem, addr);
}

/* Reads a word from memory.
 *
 * This function is called by the reset routine to read a word from memory.
 * It's up to you if you want to emulate side effects here.
 */
u16
TestCPU::read16OnReset(u32 addr) const
{
    switch (addr) {
        case 0: return 0x0000;
        case 2: return 0x2000;
        case 4: return 0x0000;
        case 6: return TestCPU::startPC;
    }
    return get16(moiraMem, addr);
}

/* Writes a byte into memory.
 *
 * This function is called whenever the 68000 CPU writes a byte into memory.
 * It should emulate the write access including all side effects.
 */
void
TestCPU::write8(u32 addr, u8  val) const
{
    switch (addr) {
        case 0x0C0080: 
                logPrintf("[IO] Write ACIA Status/Control register at $%06X = $%02X\n", addr, val); 
                if ((val ) == 0x95) {
                    // enable acia
                    TestCPU::aciaEnabled = true;
                     logPrintf("ACIA enabled\n");
                } else {
                    // disable acia
                    TestCPU::aciaEnabled = false;
                    logPrintf("ACIA disabled\n");
                }
            break;
        case 0x0C0082: 
                logPrintf("[IO] Write ACIA DATA TX register at $%06X = $%02X\n", addr, val); 
                printf("%c",val);
                if (TestCPU::txFile) {
                    if (fputc(val, TestCPU::txFile) == EOF) {
                        perror("fputc to txFile failed");
                    }
                    fflush(TestCPU::txFile);
                }
            break;
        case 0x0E0027: 
                logPrintf("[IO] Write timerPrH at $%06X = $%02X\n", addr, val);  
                TestCPU::timer_init = (TestCPU::timer_init & 0xFF00FFFF) | (val << 16);
             break;
        case 0x0E0029: 
                logPrintf("[IO] Write timerPrM at $%06X = $%02X\n", addr, val); 
                TestCPU::timer_init = (TestCPU::timer_init & 0xFFFF00FF) | (val << 8);
        break;
        case 0x0E002B: 
                logPrintf("[IO] Write timerPrL at $%06X = $%02X\n", addr, val); 
                TestCPU::timer_init = (TestCPU::timer_init & 0xFFFFFF00) | (val << 0);
                break;
        case 0x0E0021: 
                logPrintf("[IO] Write timerCR at $%06X = $%02X\n", addr, val);  
                if ((val & 0x20) == 0x20) {
                    // enable timer
                    TestCPU::timerEnabled = true;
                    logPrintf("Timer enabled\n");
                } else {
                    // disable timer
                    TestCPU::timerEnabled = false;
                    logPrintf("Timer disabled\n");
                }
            break;
        case 0x0E0035: 
                logPrintf("[IO] Write timerSR at $%06X = $%02X\n", addr, val); 
                if (val == 1) {
                    // clear timer IRQ
                    moiracpu->setIPL(0);
                }
            break;
                default : set8(moiraMem, addr, val); break;
    }
    
}

/* Writes a word into memory.
 *
 * This function is called whenever the CPU writes a word into memory.
 * It should emulate the write access including all side effects.
 */
void
TestCPU::write16 (u32 addr, u16 val) const
{
    switch (addr) {
        case 0x0C0080: logPrintf("[IO] Write16 ACIA Status/Control register at $%06X = $%04X\n", addr, val); exit(1); break;
        case 0x0C0082: logPrintf("[IO] Write16 ACIA DATA/RX/TX register at $%06X = $%04X\n", addr, val); exit(1); break;
        case 0x0E0027: logPrintf("[IO] Write16 timerPrH at $%06X = $%04X\n", addr, val); exit(1); break;
        case 0x0E0029: logPrintf("[IO] Write16 timerPrM at $%06X = $%04X\n", addr, val); exit(1); break;
        case 0x0E002B: logPrintf("[IO] Write16 timerPrL at $%06X = $%04X\n", addr, val); exit(1); break;
        case 0x0E0021: logPrintf("[IO] Write16 timerCR at $%06X = $%04X\n", addr, val); exit(1); break;
        case 0x0E0035: logPrintf("[IO] Write16 timerSR at $%06X = $%04X\n", addr, val); exit(1); break;
    }
    set16(moiraMem, addr, val);
}

/* Returns the interrupt vector in IRQ_USER mode
 */
u16
TestCPU::readIrqUserVector(moira::u8 level) const { return 0; }

/* Breakpoint handler
 *
 * Moira calls this function to signal that a breakpoint has been reached.
 */
void
TestCPU::didReachBreakpoint(moira::u32 addr) { }

/* Watchpoint handler
 *
 * Moira calls this function to signal that a watchpoint has been reached.
 */
void
TestCPU::didReachWatchpoint(moira::u32 addr) { }

void
TestCPU::setPCValue(u32 pc)  
{
    TestCPU::startPC = pc;
}




void TestCPU::initTxFile(const char* filename)
{
    if (!TestCPU::txFile) {
        TestCPU::txFile = fopen(filename, "wb");
    }
}
void TestCPU::closeTxFile()
{
    if (TestCPU::txFile) {
        fclose(TestCPU::txFile);
        TestCPU::txFile = nullptr;
    }
}