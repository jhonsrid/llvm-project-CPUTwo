//===-- CPUTwoDisassembler.cpp - Disassembler for CPUTwo ------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/CPUTwoMCTargetDesc.h"
#include "TargetInfo/CPUTwoTargetInfo.h"
#include "llvm/MC/MCDecoder.h"
#include "llvm/MC/MCDecoderOps.h"
#include "llvm/MC/MCDisassembler/MCDisassembler.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/MathExtras.h"

using namespace llvm;
using namespace llvm::MCD;

#define DEBUG_TYPE "cputwo-disassembler"

typedef MCDisassembler::DecodeStatus DecodeStatus;

namespace {

class CPUTwoDisassembler : public MCDisassembler {
public:
  CPUTwoDisassembler(const MCSubtargetInfo &STI, MCContext &Ctx)
      : MCDisassembler(STI, Ctx) {}

  DecodeStatus getInstruction(MCInst &Instr, uint64_t &Size,
                              ArrayRef<uint8_t> Bytes, uint64_t Address,
                              raw_ostream &CStream) const override;
};

} // end anonymous namespace

static DecodeStatus DecodeGPRRegisterClass(MCInst &Inst, unsigned RegNo,
                                           uint64_t /*Address*/,
                                           const MCDisassembler * /*Decoder*/) {
  if (RegNo > 15)
    return MCDisassembler::Fail;

  static const MCPhysReg GPRDecoderTable[] = {
    CPUTwo::R0,  CPUTwo::R1,  CPUTwo::R2,  CPUTwo::R3,
    CPUTwo::R4,  CPUTwo::R5,  CPUTwo::R6,  CPUTwo::R7,
    CPUTwo::R8,  CPUTwo::R9,  CPUTwo::R10, CPUTwo::R11,
    CPUTwo::R12, CPUTwo::SP,  CPUTwo::LR,  CPUTwo::PC
  };

  Inst.addOperand(MCOperand::createReg(GPRDecoderTable[RegNo]));
  return MCDisassembler::Success;
}

static DecodeStatus DecodeGPRnoSPRegisterClass(MCInst &Inst, unsigned RegNo,
                                               uint64_t Address,
                                               const MCDisassembler *Decoder) {
  return DecodeGPRRegisterClass(Inst, RegNo, Address, Decoder);
}

template <unsigned N>
static DecodeStatus decodeUImmOperand(MCInst &Inst, uint64_t Imm,
                                      uint64_t Address,
                                      const MCDisassembler *Decoder) {
  Inst.addOperand(MCOperand::createImm(Imm));
  return MCDisassembler::Success;
}

template <unsigned N>
static DecodeStatus decodeSImmOperand(MCInst &Inst, uint64_t Imm,
                                      uint64_t Address,
                                      const MCDisassembler *Decoder) {
  Inst.addOperand(MCOperand::createImm(SignExtend64<N>(Imm)));
  return MCDisassembler::Success;
}

static DecodeStatus decodeBranchTarget(MCInst &Inst, uint64_t Imm,
                                       uint64_t Address,
                                       const MCDisassembler *Decoder) {
  Inst.addOperand(MCOperand::createImm(SignExtend64<20>(Imm)));
  return MCDisassembler::Success;
}

#include "CPUTwoGenDisassemblerTables.inc"

DecodeStatus CPUTwoDisassembler::getInstruction(MCInst &Instr, uint64_t &Size,
                                                ArrayRef<uint8_t> Bytes,
                                                uint64_t Address,
                                                raw_ostream &CStream) const {
  if (Bytes.size() < 4) {
    Size = 0;
    return MCDisassembler::Fail;
  }

  Size = 4;
  uint32_t Insn = support::endian::read32le(Bytes.data());

  return decodeInstruction(DecoderTable32, Instr, Insn, Address, this, STI);
}

static MCDisassembler *createCPUTwoDisassembler(const Target &T,
                                                 const MCSubtargetInfo &STI,
                                                 MCContext &Ctx) {
  return new CPUTwoDisassembler(STI, Ctx);
}

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void
LLVMInitializeCPUTwoDisassembler() {
  TargetRegistry::RegisterMCDisassembler(getTheCPUTwoTarget(),
                                         createCPUTwoDisassembler);
}
