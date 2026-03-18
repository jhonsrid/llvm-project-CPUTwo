//===-- CPUTwoMCTargetDesc.h - CPUTwo Target Descriptions -------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CPUTWO_MCTARGETDESC_CPUTWOMCTARGETDESC_H
#define LLVM_LIB_TARGET_CPUTWO_MCTARGETDESC_CPUTWOMCTARGETDESC_H

#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCTargetOptions.h"
#include "llvm/Support/DataTypes.h"

namespace llvm {
class MCAsmBackend;
class MCCodeEmitter;
class MCContext;
class MCInstrInfo;
class MCObjectTargetWriter;
class MCSubtargetInfo;
class Target;

MCCodeEmitter *createCPUTwoMCCodeEmitter(const MCInstrInfo &MCII,
                                          MCContext &Ctx);

MCAsmBackend *createCPUTwoAsmBackend(const Target &T,
                                      const MCSubtargetInfo &STI,
                                      const MCRegisterInfo &MRI,
                                      const MCTargetOptions &Options);

std::unique_ptr<MCObjectTargetWriter> createCPUTwoELFObjectWriter(uint8_t OSABI);
} // namespace llvm

// Defines symbolic names for CPUTwo registers.
#define GET_REGINFO_ENUM
#include "CPUTwoGenRegisterInfo.inc"

// Defines symbolic names for the CPUTwo instructions.
#define GET_INSTRINFO_ENUM
#define GET_INSTRINFO_MC_HELPER_DECLS
#include "CPUTwoGenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "CPUTwoGenSubtargetInfo.inc"

#endif // LLVM_LIB_TARGET_CPUTWO_MCTARGETDESC_CPUTWOMCTARGETDESC_H
