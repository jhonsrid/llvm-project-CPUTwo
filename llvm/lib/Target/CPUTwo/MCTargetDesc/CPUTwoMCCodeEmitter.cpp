//===-- CPUTwoMCCodeEmitter.cpp - Convert CPUTwo MCInst to machine code ----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "CPUTwoMCTargetDesc.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/EndianStream.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

#define DEBUG_TYPE "mccodeemitter"

namespace {

class CPUTwoMCCodeEmitter : public MCCodeEmitter {
  const MCInstrInfo &MCII;
  MCContext &Ctx;

public:
  CPUTwoMCCodeEmitter(const MCInstrInfo &MCII, MCContext &Ctx)
      : MCII(MCII), Ctx(Ctx) {}

  void encodeInstruction(const MCInst &MI, SmallVectorImpl<char> &CB,
                         SmallVectorImpl<MCFixup> &Fixups,
                         const MCSubtargetInfo &STI) const override;

  // TableGen-generated
  uint64_t getBinaryCodeForInstr(const MCInst &MI,
                                 SmallVectorImpl<MCFixup> &Fixups,
                                 const MCSubtargetInfo &STI) const;

  unsigned getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const;

  unsigned encodeBranchTarget(const MCInst &MI, unsigned OpNo,
                              SmallVectorImpl<MCFixup> &Fixups,
                              const MCSubtargetInfo &STI) const;

  unsigned encodeMemOperand(const MCInst &MI, unsigned OpNo,
                            SmallVectorImpl<MCFixup> &Fixups,
                            const MCSubtargetInfo &STI) const;
};

} // end anonymous namespace

#include "CPUTwoGenMCCodeEmitter.inc"

MCCodeEmitter *llvm::createCPUTwoMCCodeEmitter(const MCInstrInfo &MCII,
                                                MCContext &Ctx) {
  return new CPUTwoMCCodeEmitter(MCII, Ctx);
}

void CPUTwoMCCodeEmitter::encodeInstruction(const MCInst &MI,
                                            SmallVectorImpl<char> &CB,
                                            SmallVectorImpl<MCFixup> &Fixups,
                                            const MCSubtargetInfo &STI) const {
  uint32_t Bits = getBinaryCodeForInstr(MI, Fixups, STI);
  support::endian::write<uint32_t>(CB, Bits, llvm::endianness::little);
}

unsigned CPUTwoMCCodeEmitter::getMachineOpValue(
    const MCInst &MI, const MCOperand &MO, SmallVectorImpl<MCFixup> &Fixups,
    const MCSubtargetInfo &STI) const {
  if (MO.isReg())
    return Ctx.getRegisterInfo()->getEncodingValue(MO.getReg());
  if (MO.isImm())
    return static_cast<unsigned>(MO.getImm());

  // MO must be an expression
  assert(MO.isExpr() && "Expected expression operand");
  return 0;
}

unsigned CPUTwoMCCodeEmitter::encodeBranchTarget(
    const MCInst &MI, unsigned OpNo, SmallVectorImpl<MCFixup> &Fixups,
    const MCSubtargetInfo &STI) const {
  const MCOperand &MO = MI.getOperand(OpNo);
  if (MO.isImm())
    return static_cast<unsigned>(MO.getImm()) & 0xFFFFF;

  // Create a fixup for the branch target
  Fixups.push_back(MCFixup::create(0, MO.getExpr(),
                                    static_cast<MCFixupKind>(FK_Data_4)));
  return 0;
}

unsigned CPUTwoMCCodeEmitter::encodeMemOperand(
    const MCInst &MI, unsigned OpNo, SmallVectorImpl<MCFixup> &Fixups,
    const MCSubtargetInfo &STI) const {
  unsigned Base = getMachineOpValue(MI, MI.getOperand(OpNo), Fixups, STI);
  unsigned Offset = getMachineOpValue(MI, MI.getOperand(OpNo + 1), Fixups, STI);
  return (Base << 16) | (Offset & 0xFFFF);
}
