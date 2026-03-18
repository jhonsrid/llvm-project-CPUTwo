//===-- CPUTwoFrameLowering.cpp - CPUTwo Frame Lowering -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "CPUTwoFrameLowering.h"
#include "CPUTwo.h"
#include "CPUTwoInstrInfo.h"
#include "CPUTwoSubtarget.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/RegisterScavenging.h"

using namespace llvm;

void CPUTwoFrameLowering::emitPrologue(MachineFunction &MF,
                                        MachineBasicBlock &MBB) const {
  MachineFrameInfo &MFI = MF.getFrameInfo();
  const CPUTwoInstrInfo &TII =
      *static_cast<const CPUTwoInstrInfo *>(STI.getInstrInfo());
  MachineBasicBlock::iterator MBBI = MBB.begin();
  DebugLoc DL;

  uint64_t StackSize = MFI.getStackSize();
  if (StackSize == 0)
    return;

  // ADDI SP, SP, -StackSize
  if (isInt<16>(-static_cast<int64_t>(StackSize))) {
    BuildMI(MBB, MBBI, DL, TII.get(CPUTwo::ADDI), CPUTwo::SP)
        .addReg(CPUTwo::SP)
        .addImm(-static_cast<int64_t>(StackSize))
        .setMIFlag(MachineInstr::FrameSetup);
  } else {
    // Large frame: materialize size in scratch reg, then subtract
    Register ScratchReg = CPUTwo::R12;
    BuildMI(MBB, MBBI, DL, TII.get(CPUTwo::LUI), ScratchReg)
        .addImm((StackSize >> 16) & 0xFFFF)
        .setMIFlag(MachineInstr::FrameSetup);
    BuildMI(MBB, MBBI, DL, TII.get(CPUTwo::ORI), ScratchReg)
        .addReg(ScratchReg)
        .addImm(StackSize & 0xFFFF)
        .setMIFlag(MachineInstr::FrameSetup);
    BuildMI(MBB, MBBI, DL, TII.get(CPUTwo::SUB), CPUTwo::SP)
        .addReg(CPUTwo::SP)
        .addReg(ScratchReg)
        .setMIFlag(MachineInstr::FrameSetup);
  }
}

void CPUTwoFrameLowering::emitEpilogue(MachineFunction &MF,
                                        MachineBasicBlock &MBB) const {
  MachineFrameInfo &MFI = MF.getFrameInfo();
  const CPUTwoInstrInfo &TII =
      *static_cast<const CPUTwoInstrInfo *>(STI.getInstrInfo());
  MachineBasicBlock::iterator MBBI = MBB.getLastNonDebugInstr();
  DebugLoc DL = MBBI->getDebugLoc();

  uint64_t StackSize = MFI.getStackSize();
  if (StackSize == 0)
    return;

  // ADDI SP, SP, StackSize
  if (isInt<16>(StackSize)) {
    BuildMI(MBB, MBBI, DL, TII.get(CPUTwo::ADDI), CPUTwo::SP)
        .addReg(CPUTwo::SP)
        .addImm(StackSize)
        .setMIFlag(MachineInstr::FrameDestroy);
  } else {
    Register ScratchReg = CPUTwo::R12;
    BuildMI(MBB, MBBI, DL, TII.get(CPUTwo::LUI), ScratchReg)
        .addImm((StackSize >> 16) & 0xFFFF)
        .setMIFlag(MachineInstr::FrameDestroy);
    BuildMI(MBB, MBBI, DL, TII.get(CPUTwo::ORI), ScratchReg)
        .addReg(ScratchReg)
        .addImm(StackSize & 0xFFFF)
        .setMIFlag(MachineInstr::FrameDestroy);
    BuildMI(MBB, MBBI, DL, TII.get(CPUTwo::ADD), CPUTwo::SP)
        .addReg(CPUTwo::SP)
        .addReg(ScratchReg)
        .setMIFlag(MachineInstr::FrameDestroy);
  }
}

void CPUTwoFrameLowering::determineCalleeSaves(MachineFunction &MF,
                                                 BitVector &SavedRegs,
                                                 RegScavenger *RS) const {
  TargetFrameLowering::determineCalleeSaves(MF, SavedRegs, RS);

  // Create an emergency spill slot for the register scavenger.
  // This is needed when eliminateFrameIndex needs a scratch register
  // for large stack offsets.
  if (RS && !MF.getFrameInfo().isFrameAddressTaken()) {
    MachineFrameInfo &MFI = MF.getFrameInfo();
    // GPR is 4 bytes, 4-byte aligned
    RS->addScavengingFrameIndex(MFI.CreateStackObject(4, Align(4), false));
  }
}

MachineBasicBlock::iterator
CPUTwoFrameLowering::eliminateCallFramePseudoInstr(
    MachineFunction &MF, MachineBasicBlock &MBB,
    MachineBasicBlock::iterator I) const {
  return MBB.erase(I);
}
