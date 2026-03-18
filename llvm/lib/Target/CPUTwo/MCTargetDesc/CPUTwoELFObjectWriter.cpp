//===-- CPUTwoELFObjectWriter.cpp - CPUTwo ELF Writer ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "CPUTwoMCTargetDesc.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCValue.h"

using namespace llvm;

namespace {

class CPUTwoELFObjectWriter : public MCELFObjectTargetWriter {
public:
  explicit CPUTwoELFObjectWriter(uint8_t OSABI)
      : MCELFObjectTargetWriter(/*Is64Bit=*/false, OSABI, ELF::EM_CPUTWO,
                                /*HasRelocationAddend=*/true) {}

  unsigned getRelocType(const MCFixup &Fixup, const MCValue &Target,
                        bool IsPCRel) const override {
    return ELF::R_CPUTWO_NONE;
  }
};

} // end anonymous namespace

std::unique_ptr<MCObjectTargetWriter>
llvm::createCPUTwoELFObjectWriter(uint8_t OSABI) {
  return std::make_unique<CPUTwoELFObjectWriter>(OSABI);
}
