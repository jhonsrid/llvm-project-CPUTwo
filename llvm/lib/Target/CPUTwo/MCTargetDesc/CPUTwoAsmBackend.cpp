//===-- CPUTwoAsmBackend.cpp - CPUTwo Assembler Backend --------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "CPUTwoFixupKinds.h"
#include "CPUTwoMCTargetDesc.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Endian.h"

#define DEBUG_TYPE "cputwo-asm-backend"

using namespace llvm;

namespace {

class CPUTwoAsmBackend : public MCAsmBackend {
public:
  CPUTwoAsmBackend(const MCSubtargetInfo &STI, const MCTargetOptions &Options)
      : MCAsmBackend(llvm::endianness::little) {}

  MCFixupKindInfo getFixupKindInfo(MCFixupKind Kind) const override {
    const static MCFixupKindInfo Infos[CPUTwo::NumTargetFixupKinds] = {
        {"fixup_cputwo_pc20", 0, 20, 0},
        {"fixup_cputwo_hi16", 0, 16, 0},
        {"fixup_cputwo_lo16", 0, 16, 0},
    };
    if (Kind < FirstTargetFixupKind)
      return MCAsmBackend::getFixupKindInfo(Kind);
    assert(unsigned(Kind - FirstTargetFixupKind) < CPUTwo::NumTargetFixupKinds);
    return Infos[Kind - FirstTargetFixupKind];
  }

  void applyFixup(const MCFragment &, const MCFixup &Fixup,
                  const MCValue &Target, uint8_t *Data, uint64_t Value,
                  bool IsResolved) override;

  std::unique_ptr<MCObjectTargetWriter>
  createObjectTargetWriter() const override {
    return createCPUTwoELFObjectWriter(ELF::ELFOSABI_NONE);
  }

  bool writeNopData(raw_ostream &OS, uint64_t Count,
                    const MCSubtargetInfo *STI) const override;
};

} // end anonymous namespace

void CPUTwoAsmBackend::applyFixup(const MCFragment &, const MCFixup &Fixup,
                                   const MCValue &Target, uint8_t *Data,
                                   uint64_t Value, bool IsResolved) {
  unsigned Kind = Fixup.getKind();

  switch (Kind) {
  case FK_Data_1:
    *Data = Value;
    break;
  case FK_Data_2:
    support::endian::write16le(Data, Value);
    break;
  case FK_Data_4:
    support::endian::write32le(Data, Value);
    break;
  case CPUTwo::fixup_cputwo_pc20: {
    // PC-relative 20-bit offset in bits [19:0] of instruction
    uint32_t Insn = support::endian::read32le(Data);
    Insn = (Insn & 0xFFF00000) | (static_cast<uint32_t>(Value) & 0xFFFFF);
    support::endian::write32le(Data, Insn);
    break;
  }
  case CPUTwo::fixup_cputwo_hi16: {
    uint32_t Insn = support::endian::read32le(Data);
    Insn = (Insn & 0xFFFF0000) | ((Value >> 16) & 0xFFFF);
    support::endian::write32le(Data, Insn);
    break;
  }
  case CPUTwo::fixup_cputwo_lo16: {
    uint32_t Insn = support::endian::read32le(Data);
    Insn = (Insn & 0xFFFF0000) | (Value & 0xFFFF);
    support::endian::write32le(Data, Insn);
    break;
  }
  default:
    break;
  }
}

bool CPUTwoAsmBackend::writeNopData(raw_ostream &OS, uint64_t Count,
                                     const MCSubtargetInfo *STI) const {
  if ((Count % 4) != 0)
    return false;
  for (uint64_t i = 0; i < Count; i += 4)
    OS.write("\0\0\0\0", 4);
  return true;
}

MCAsmBackend *llvm::createCPUTwoAsmBackend(const Target &T,
                                            const MCSubtargetInfo &STI,
                                            const MCRegisterInfo &MRI,
                                            const MCTargetOptions &Options) {
  return new CPUTwoAsmBackend(STI, Options);
}
