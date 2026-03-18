//===-- CPUTwoMCTargetDesc.cpp - CPUTwo Target Descriptions ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "CPUTwoMCTargetDesc.h"
#include "CPUTwoInstPrinter.h"
#include "CPUTwoMCAsmInfo.h"
#include "TargetInfo/CPUTwoTargetInfo.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/TargetParser/Triple.h"

#define GET_INSTRINFO_MC_DESC
#define ENABLE_INSTR_PREDICATE_VERIFIER
#include "CPUTwoGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "CPUTwoGenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "CPUTwoGenRegisterInfo.inc"

using namespace llvm;

static MCInstrInfo *createCPUTwoMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitCPUTwoMCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createCPUTwoMCRegisterInfo(const Triple & /*TT*/) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitCPUTwoMCRegisterInfo(X, CPUTwo::LR, 0, 0, CPUTwo::PC);
  return X;
}

static MCSubtargetInfo *
createCPUTwoMCSubtargetInfo(const Triple &TT, StringRef CPU, StringRef FS) {
  std::string CPUName = std::string(CPU);
  if (CPUName.empty())
    CPUName = "generic";
  return createCPUTwoMCSubtargetInfoImpl(TT, CPUName, /*TuneCPU*/ CPUName, FS);
}

static MCStreamer *createMCStreamer(const Triple &T, MCContext &Context,
                                   std::unique_ptr<MCAsmBackend> &&MAB,
                                   std::unique_ptr<MCObjectWriter> &&OW,
                                   std::unique_ptr<MCCodeEmitter> &&Emitter) {
  if (!T.isOSBinFormatELF())
    llvm_unreachable("OS not supported");
  return createELFStreamer(Context, std::move(MAB), std::move(OW),
                           std::move(Emitter));
}

static MCInstPrinter *createCPUTwoMCInstPrinter(const Triple & /*T*/,
                                                 unsigned SyntaxVariant,
                                                 const MCAsmInfo &MAI,
                                                 const MCInstrInfo &MII,
                                                 const MCRegisterInfo &MRI) {
  if (SyntaxVariant == 0)
    return new CPUTwoInstPrinter(MAI, MII, MRI);
  return nullptr;
}

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void
LLVMInitializeCPUTwoTargetMC() {
  // Register the MC asm info.
  RegisterMCAsmInfo<CPUTwoMCAsmInfo> X(getTheCPUTwoTarget());

  // Register the MC instruction info.
  TargetRegistry::RegisterMCInstrInfo(getTheCPUTwoTarget(),
                                      createCPUTwoMCInstrInfo);

  // Register the MC register info.
  TargetRegistry::RegisterMCRegInfo(getTheCPUTwoTarget(),
                                    createCPUTwoMCRegisterInfo);

  // Register the MC subtarget info.
  TargetRegistry::RegisterMCSubtargetInfo(getTheCPUTwoTarget(),
                                          createCPUTwoMCSubtargetInfo);

  // Register the MC code emitter.
  TargetRegistry::RegisterMCCodeEmitter(getTheCPUTwoTarget(),
                                        createCPUTwoMCCodeEmitter);

  // Register the ASM backend.
  TargetRegistry::RegisterMCAsmBackend(getTheCPUTwoTarget(),
                                       createCPUTwoAsmBackend);

  // Register the MCInstPrinter.
  TargetRegistry::RegisterMCInstPrinter(getTheCPUTwoTarget(),
                                        createCPUTwoMCInstPrinter);

  // Register the ELF streamer.
  TargetRegistry::RegisterELFStreamer(getTheCPUTwoTarget(), createMCStreamer);
}
