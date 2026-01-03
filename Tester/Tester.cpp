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
#include <cstdio>
#include <cstdarg>
#include <ctime>
#include <fstream>
#include <sstream>
#include <string>

bool VERBOSE = false;

TestCPU *moiracpu;


u8 moiraMem[0x10000];

moira::Model cpuModel = Model::M68000;
u16 opcode = 0;

static FILE* logFile = nullptr;

 void logPrintf(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    if (VERBOSE)
        if (logFile) {
            vfprintf(logFile, fmt, ap);
            fflush(logFile);
        }
    va_end(ap);
}



void recordMoiraRegisters(Result &r)
{
    r.pc = moiracpu->getPC();
    r.sr = moiracpu->getSR();
    r.usp = moiracpu->getUSP();
    r.isp = moiracpu->getISP();
    r.msp = moiracpu->getMSP();
    r.fc = moiracpu->readFC();
    r.vbr = moiracpu->getVBR();
    r.sfc = moiracpu->getSFC();
    r.dfc = moiracpu->getDFC();
    r.cacr = moiracpu->getCACR();
    r.caar = moiracpu->getCAAR();
    for (int i = 0; i < 8; i++) r.d[i] = moiracpu->getD(i);
    for (int i = 0; i < 8; i++) r.a[i] = moiracpu->getA(i);
}

void dumpResult(Result &r)
{
    logPrintf("PC: %04x   cycles: %d\n", r.pc, r.cycles);
    logPrintf("         ");
    logPrintf("SR: %x  ", r.sr);
    logPrintf("USP: %x  ", r.usp);
    logPrintf("ISP: %x  ", r.isp);
    logPrintf("MSP: %x  ", r.msp);
    logPrintf("FC: %x  ", r.fc);
    logPrintf("VBR: %x  ", r.vbr);
    logPrintf("SFC: %x  ", r.sfc);
    logPrintf("DFC: %x  ", r.dfc);
    logPrintf("CACR: %02x ", r.cacr);
    logPrintf("CAAR: %02x ", r.caar);

    logPrintf("\n");

    logPrintf("         Dn: ");
    for (int i = 0; i < 8; i++) logPrintf("%8x ", r.d[i]);
    logPrintf("\n");
    logPrintf("         An: ");
    for (int i = 0; i < 8; i++) logPrintf("%8x ", r.a[i]);
    logPrintf("\n");
}



// Ajoutez cette fonction pour charger un programme hexadécimal en mémoire
bool loadHexProgram(const std::string& filename, u8* memory, size_t memsize, u32 loadAddr = 0)
{
    std::ifstream file(filename);
    if (!file) {
        printf("Impossible d'ouvrir le fichier %s\n", filename.c_str());
        return false;
    }

    std::string line;
    size_t totalBytes = 0;
    while (std::getline(file, line)) {
        // Ignore tout ce qui suit le caractère '#'
        size_t pos = line.find('#');
        if (pos != std::string::npos) {
            line = line.substr(0, pos);
        }
        // Permettre un ':' à la fin de l'adresse
        std::istringstream iss(line);
        std::string addrStr;
        if (!(iss >> addrStr)) {
            // Ignore les lignes vides ou invalides
            continue;
        }
        // Retirer le ':' si présent
        if (!addrStr.empty() && addrStr.back() == ':') {
            addrStr.pop_back();
        }
        unsigned int addr;
        std::istringstream addrStream(addrStr);
        if (!(addrStream >> std::hex >> addr)) {
            // Ignore si l'adresse n'est pas valide
            continue;
        }
        unsigned int byte;
        while (iss >> std::hex >> byte) {
            if (addr >= memsize) {
                printf("Dépassement de la mémoire lors du chargement\n");
                return false;
            }
            memory[addr++] = static_cast<u8>(byte);
            ++totalBytes;
        }
    }
    printf("Programme chargé à partir de %s (%zu octets)\n", filename.c_str(), totalBytes);
    return true;
}

// Affiche le contenu de la mémoire à partir d'une adresse donnée
void dumpMemory(const u8* memory, size_t start, size_t end)
{
    logPrintf("Contenu mémoire de $%04zx à $%04zx :\n", start, end);
    for (size_t addr = start; addr < end; addr += 16) {
        logPrintf("$%04zx : ", addr);
        for (size_t i = 0; i < 16 && (addr + i) < end; ++i) {
            logPrintf("%02x ", memory[addr + i]);
        }
        logPrintf("\n");
    }
}

// Modifiez runHexProgram pour afficher la mémoire après chargement
void runHexProgram(const std::string& filename, int maxInstructions)
{
    if (VERBOSE){

        logFile = fopen("tester.log", "w");
        if (!logFile) {
            perror("tester.log");
            return;
        }
    }
    logPrintf("Exécution du programme...\n");
    Result mor;
    mor.elapsed[0] = mor.elapsed[1] = 0;
    // Charger le programme en mémoire
    if (!loadHexProgram(filename, moiraMem, sizeof(moiraMem))) {
        logPrintf("Erreur lors du chargement du programme hexadécimal.\n");
        return;
    }

    // Afficher la mémoire chargée (par exemple, les 64 premiers octets)
    if( VERBOSE) dumpMemory(moiraMem, 0x1600, 0x1600 + 64);

    // Initialiser le CPU
    moiracpu->reset();
    moiracpu->setModel(Model::M68000);


    logPrintf("Exécution du programme à partir de PC = $%08x\n", moiracpu->getPC());

    // Boucle d'exécution simple (arrêt sur instruction ILLEGAL ou limite d'instructions)
    int executed = 0;
    while (executed++ < maxInstructions) {
        u32 pc = moiracpu->getPC();
        u16 op = get16(moiraMem, pc);
        logPrintf("[%d] PC: $%08x  Opcode: $%04x.", executed, pc, op);
        if (moiracpu->getInstrInfo(op).I == moira::Instr::ILLEGAL) {
            logPrintf("Instruction ILLEGAL rencontrée à $%08x, arrêt.\n", pc);
            break;
        }

        // Détection d'une instruction TRAP (opcodes 0x4E40 à 0x4E4F)
       // if ((op & 0xFFF0) == 0x4E40) {
            //logPrintf("  Dump de la stack :\n");
            // Récupérer le SP (User Stack Pointer)
            // check user or supervisor mode
            u32 sr  = moiracpu->getSR();
            u32 sp ;
            if (sr & 0x2000) {
                sp = moiracpu->getISP();
            } else {
                sp = moiracpu->getUSP();
            }
            
            // Afficher les 4 premiers mots de la stack
            for (int i = 0; i < 4; i++) {
                u32 addr = sp + i * 2;
                u16 val = get16(moiraMem, addr);
                if (VERBOSE) logPrintf("  [$%08x] = $%04x\n", addr, val);
            }
           
        //}

        // print disassembled instruction
        moiracpu->setDasmSyntax(Syntax::GNU);
        moiracpu->setDasmNumberFormat({ .prefix = "$", .radix = 16 });
        char dasm[128];
        moiracpu->disassemble(dasm, pc);
        logPrintf(": %32s", dasm);
        logPrintf("    Timer: %d\n", TestCPU::timer);
        int64_t cycles = moiracpu->getClock();
        // Execute instruction
    
        moiracpu->execute();
 
        // Record the result
        mor.cycles = (int)(moiracpu->getClock() - cycles);
        mor.elapsed[0] +=mor.cycles;
        recordMoiraRegisters(mor);
        if (VERBOSE) dumpResult(mor);
        // dump elapsed time
        if (VERBOSE) logPrintf("    Elapsed cycles: %ld\n", mor.elapsed[0] );
    }
    logPrintf("Exécution terminée (%d instructions).\n", executed - 1);
    fclose(logFile);
    logFile = nullptr;
}


