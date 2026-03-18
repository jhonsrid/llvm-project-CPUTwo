//===-- CPUTwoSubtarget.cpp - CPUTwo Subtarget Information -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "CPUTwoSubtarget.h"

#define DEBUG_TYPE "cputwo-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "CPUTwoGenSubtargetInfo.inc"

using namespace llvm;

CPUTwoSubtarget::CPUTwoSubtarget(const Triple &TT, StringRef CPU,
                                   StringRef FS, const TargetMachine &TM)
    : CPUTwoGenSubtargetInfo(TT, CPU, /*TuneCPU*/ CPU, FS),
      InstrInfo(*this), FrameLowering(*this), TLInfo(TM, *this) {
  std::string CPUName = std::string(CPU);
  if (CPUName.empty())
    CPUName = "generic";
  ParseSubtargetFeatures(CPUName, /*TuneCPU*/ CPUName, FS);
}
