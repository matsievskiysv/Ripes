#include "ripes_picorv32.h"

// Autogenerated by CMake verilate(...)
#include "Vpicorv32.h"

#include "../../binutils.h"
#include "../RISC-V/riscv.h"

namespace Ripes {

PicoRV32::PicoRV32(const QStringList&) : m_dataAccess(), m_instrAccess() {
    m_features = 0;
    top = new Vpicorv32;
}

PicoRV32::~PicoRV32() {
    delete top;
}

void PicoRV32::clockProcessor() {
    handleMemoryAccess();
    if (m_doPCPI) {
        top->pcpi_ready = 1;
    }
    for (int i = 0; i < 2; i++) {
        top->clk = !top->clk;
        top->eval();
    }
    if (m_doPCPI && !top->pcpi_valid) {
        top->pcpi_ready = 0;
        m_doPCPI = false;
    }

    if (!m_doPCPI && top->pcpi_valid && top->pcpi_insn == RVISA::Opcode::ECALL) {
        trapHandler();
        m_doPCPI = true;
    }

    if (top->trap) {
        m_finished = true;
    }

    processorWasClocked.Emit();
}

void PicoRV32::resetProcessor() {
    m_memory.reset();
    top->clk = 0;
    top->resetn = 0;
    clockProcessor();
    clockProcessor();
    top->resetn = 1;

    // Instead of forcing the user to use a linker script for having a reset vector, we compromise a bit and forcibly
    // set the initial PC values as per what Ripes defines as the entry point. Reduces realism but makes assembly
    // programming easier.
    top->picorv32__DOT__reg_next_pc = m_initialPC;
    top->picorv32__DOT__reg_pc = m_initialPC;

    m_finished = false;

    processorWasReset.Emit();
}

void PicoRV32::finalize(FinalizeReason fr) {
    if (fr == FinalizeReason::exitSyscall) {
        m_finished = true;
    }
}

StageInfo PicoRV32::stageInfo(unsigned) const {
    StageInfo si;
    State state = static_cast<State>(top->picorv32__DOT__cpu_state);
    auto stateIt = stateToString.find(state);
    if (stateIt != stateToString.end()) {
        si.namedState = stateIt->second;
    } else {
        si.namedState = "unknown";
    }
    si.pc = getPcForStage(0);
    si.state = StageInfo::State::None;
    si.stage_valid = true;
    return si;
};

void PicoRV32::handleMemoryAccess() {
    auto access = MemoryAccess();
    if (top->mem_valid) {
        top->mem_ready = 1;
        const unsigned offset = firstSetBitIdx(top->mem_wstrb);
        if (top->mem_wstrb) {
            const unsigned bytes = vsrtl::bitcount(top->mem_wstrb);
            m_memory.writeMem(top->mem_addr + offset, top->mem_wdata, bytes);
            access.type = MemoryAccess::Write;
            access.bytes = bytes;
        } else {
            top->mem_rdata = m_memory.readMemConst(top->mem_addr + offset, 4);
            access.type = MemoryAccess::Read;
            access.bytes = 4;
        }
        access.address = top->mem_addr;
    } else {
        top->mem_ready = 0;
    }

    m_instrAccess = top->mem_instr ? access : MemoryAccess();
    m_dataAccess = top->mem_instr ? MemoryAccess() : access;
}

unsigned int PicoRV32::getPcForStage(unsigned) const {
    return top->picorv32__DOT__reg_pc;
};

AInt PicoRV32::nextFetchedAddress() const {
    return top->picorv32__DOT__reg_next_pc;
};

void PicoRV32::setProgramCounter(AInt address) {
    top->picorv32__DOT__reg_pc = address;
};

void PicoRV32::setPCInitialValue(AInt address) {
    m_initialPC = address;
}

VInt PicoRV32::getRegister(RegisterFileType, unsigned i) const {
    Q_ASSERT(i < 32);
    return top->picorv32__DOT__cpuregs[i];
};
void PicoRV32::setRegister(RegisterFileType, unsigned i, VInt v) {
    Q_ASSERT(i < 32);
    top->picorv32__DOT__cpuregs[i] = v;
};

long long PicoRV32::getInstructionsRetired() const {
    return top->picorv32__DOT__count_instr;
}
long long PicoRV32::getCycleCount() const {
    return top->picorv32__DOT__count_cycle;
}

}  // namespace Ripes
