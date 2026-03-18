//===-- CPUTwoTargetMachine.cpp - Define TargetMachine for CPUTwo ----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "CPUTwoTargetMachine.h"
#include "CPUTwo.h"
#include "CPUTwoTargetObjectFile.h"
#include "TargetInfo/CPUTwoTargetInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Compiler.h"

using namespace llvm;

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void
LLVMInitializeCPUTwoTarget() {
  RegisterTargetMachine<CPUTwoTargetMachine> X(getTheCPUTwoTarget());
}

static std::string computeDataLayout() {
  // Little-endian, 32-bit pointers, 32-bit aligned
  return "e-m:e-p:32:32-i32:32-n32-S32";
}

CPUTwoTargetMachine::CPUTwoTargetMachine(const Target &T, const Triple &TT,
                                         StringRef CPU, StringRef FS,
                                         const TargetOptions &Options,
                                         std::optional<Reloc::Model> RM,
                                         std::optional<CodeModel::Model> CM,
                                         CodeGenOptLevel OL, bool JIT)
    : CodeGenTargetMachineImpl(T, computeDataLayout(), TT, CPU, FS, Options,
                               RM.value_or(Reloc::Static),
                               CM.value_or(CodeModel::Small), OL),
      TLOF(std::make_unique<CPUTwoTargetObjectFile>()),
      Subtarget(TT, std::string(CPU), std::string(FS), *this) {
  initAsmInfo();
}

namespace {
class CPUTwoPassConfig : public TargetPassConfig {
public:
  CPUTwoPassConfig(CPUTwoTargetMachine &TM, PassManagerBase &PM)
      : TargetPassConfig(TM, PM) {}

  CPUTwoTargetMachine &getCPUTwoTargetMachine() const {
    return getTM<CPUTwoTargetMachine>();
  }

  bool addInstSelector() override;
};
} // end anonymous namespace

bool CPUTwoPassConfig::addInstSelector() {
  addPass(createCPUTwoISelDag(getCPUTwoTargetMachine()));
  return false;
}

TargetPassConfig *CPUTwoTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new CPUTwoPassConfig(*this, PM);
}
