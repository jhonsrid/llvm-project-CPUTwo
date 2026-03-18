//===-- CPUTwoMachineFunctionInfo.h - CPUTwo Machine Function Info -*- C++ -*-=//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CPUTWO_CPUTWOMACHINEFUNCTIONINFO_H
#define LLVM_LIB_TARGET_CPUTWO_CPUTWOMACHINEFUNCTIONINFO_H

#include "llvm/CodeGen/MachineFunction.h"

namespace llvm {

class CPUTwoMachineFunctionInfo : public MachineFunctionInfo {
  int VarArgsFrameIndex = 0;

public:
  CPUTwoMachineFunctionInfo(const Function &F,
                            const TargetSubtargetInfo *STI) {}

  int getVarArgsFrameIndex() const { return VarArgsFrameIndex; }
  void setVarArgsFrameIndex(int Index) { VarArgsFrameIndex = Index; }
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_CPUTWO_CPUTWOMACHINEFUNCTIONINFO_H
