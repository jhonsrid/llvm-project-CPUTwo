//===-- CPUTwoFrameLowering.h - CPUTwo Frame Lowering ----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CPUTWO_CPUTWOFRAMELOWERING_H
#define LLVM_LIB_TARGET_CPUTWO_CPUTWOFRAMELOWERING_H

#include "llvm/CodeGen/TargetFrameLowering.h"

namespace llvm {

class CPUTwoSubtarget;

class CPUTwoFrameLowering : public TargetFrameLowering {
  const CPUTwoSubtarget &STI;

public:
  explicit CPUTwoFrameLowering(const CPUTwoSubtarget &Subtarget)
      : TargetFrameLowering(StackGrowsDown,
                            /*StackAlignment=*/Align(4),
                            /*LocalAreaOffset=*/0),
        STI(Subtarget) {}

  void emitPrologue(MachineFunction &MF,
                    MachineBasicBlock &MBB) const override;
  void emitEpilogue(MachineFunction &MF,
                    MachineBasicBlock &MBB) const override;

  MachineBasicBlock::iterator
  eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator I) const override;

  bool hasFPImpl(const MachineFunction &MF) const override { return false; }
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_CPUTWO_CPUTWOFRAMELOWERING_H
