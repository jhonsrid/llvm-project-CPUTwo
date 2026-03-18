//===-- CPUTwoAsmPrinter.cpp - CPUTwo LLVM Assembly Printer ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "CPUTwo.h"
#include "CPUTwoMCInstLower.h"
#include "CPUTwoSubtarget.h"
#include "CPUTwoTargetMachine.h"
#include "TargetInfo/CPUTwoTargetInfo.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Compiler.h"

using namespace llvm;

#define DEBUG_TYPE "asm-printer"

namespace {

class CPUTwoAsmPrinter : public AsmPrinter {
  CPUTwoMCInstLower MCInstLowering;

public:
  explicit CPUTwoAsmPrinter(TargetMachine &TM,
                            std::unique_ptr<MCStreamer> Streamer)
      : AsmPrinter(TM, std::move(Streamer)),
        MCInstLowering(OutContext, *this) {}

  StringRef getPassName() const override { return "CPUTwo Assembly Printer"; }

  void emitInstruction(const MachineInstr *MI) override;
};

} // end anonymous namespace

void CPUTwoAsmPrinter::emitInstruction(const MachineInstr *MI) {
  MCInst TmpInst;
  MCInstLowering.Lower(MI, TmpInst);
  EmitToStreamer(*OutStreamer, TmpInst);
}

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void
LLVMInitializeCPUTwoAsmPrinter() {
  RegisterAsmPrinter<CPUTwoAsmPrinter> X(getTheCPUTwoTarget());
}
