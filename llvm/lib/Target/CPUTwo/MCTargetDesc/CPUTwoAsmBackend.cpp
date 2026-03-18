//===-- CPUTwoAsmBackend.cpp - CPUTwo Assembler Backend --------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "CPUTwoMCTargetDesc.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"

using namespace llvm;

namespace {

class CPUTwoAsmBackend : public MCAsmBackend {
public:
  CPUTwoAsmBackend(const MCSubtargetInfo &STI, const MCTargetOptions &Options)
      : MCAsmBackend(llvm::endianness::little) {}

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
  // TODO: Implement fixup application for CPUTwo relocations
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
