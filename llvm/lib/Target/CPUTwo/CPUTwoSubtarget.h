//===-- CPUTwoSubtarget.h - Define Subtarget for CPUTwo --------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CPUTWO_CPUTWOSUBTARGET_H
#define LLVM_LIB_TARGET_CPUTWO_CPUTWOSUBTARGET_H

#include "CPUTwoFrameLowering.h"
#include "CPUTwoISelLowering.h"
#include "CPUTwoInstrInfo.h"
#include "llvm/CodeGen/SelectionDAGTargetInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/Target/TargetMachine.h"

#define GET_SUBTARGETINFO_HEADER
#include "CPUTwoGenSubtargetInfo.inc"

namespace llvm {

class CPUTwoSubtarget : public CPUTwoGenSubtargetInfo {
  CPUTwoInstrInfo InstrInfo;
  CPUTwoFrameLowering FrameLowering;
  CPUTwoTargetLowering TLInfo;
  SelectionDAGTargetInfo TSInfo;

public:
  CPUTwoSubtarget(const Triple &TT, StringRef CPU, StringRef FS,
                  const TargetMachine &TM);

  void ParseSubtargetFeatures(StringRef CPU, StringRef TuneCPU, StringRef FS);

  const CPUTwoInstrInfo *getInstrInfo() const override { return &InstrInfo; }
  const TargetFrameLowering *getFrameLowering() const override {
    return &FrameLowering;
  }
  const CPUTwoRegisterInfo *getRegisterInfo() const override {
    return &InstrInfo.getRegisterInfo();
  }
  const CPUTwoTargetLowering *getTargetLowering() const override {
    return &TLInfo;
  }
  const SelectionDAGTargetInfo *getSelectionDAGInfo() const override {
    return &TSInfo;
  }
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_CPUTWO_CPUTWOSUBTARGET_H
