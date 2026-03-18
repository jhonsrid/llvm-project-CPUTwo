//===-- CPUTwoRegisterInfo.cpp - CPUTwo Register Information ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "CPUTwoRegisterInfo.h"
#include "CPUTwo.h"
#include "CPUTwoSubtarget.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/CodeGen/TargetInstrInfo.h"

using namespace llvm;

#define GET_REGINFO_TARGET_DESC
#include "CPUTwoGenRegisterInfo.inc"

CPUTwoRegisterInfo::CPUTwoRegisterInfo() : CPUTwoGenRegisterInfo(CPUTwo::LR) {}

const MCPhysReg *
CPUTwoRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  return CSR_CPUTwo_SaveList;
}

const uint32_t *
CPUTwoRegisterInfo::getCallPreservedMask(const MachineFunction &MF,
                                         CallingConv::ID) const {
  return CSR_CPUTwo_RegMask;
}

BitVector CPUTwoRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());
  Reserved.set(CPUTwo::SP);
  Reserved.set(CPUTwo::PC);
  Reserved.set(CPUTwo::SR);
  return Reserved;
}

bool CPUTwoRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                              int SPAdj,
                                              unsigned FIOperandNum,
                                              RegScavenger *RS) const {
  MachineInstr &MI = *II;
  MachineFunction &MF = *MI.getParent()->getParent();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();

  int FrameIndex = MI.getOperand(FIOperandNum).getIndex();
  int64_t Offset = MFI.getObjectOffset(FrameIndex) + SPAdj +
                   MI.getOperand(FIOperandNum + 1).getImm();
  int64_t StackSize = MFI.getStackSize();

  // SP-relative addressing: offset from current SP
  Offset += StackSize;

  // Replace the frame index with SP and the computed offset
  MI.getOperand(FIOperandNum).ChangeToRegister(CPUTwo::SP, false);

  if (isInt<16>(Offset)) {
    MI.getOperand(FIOperandNum + 1).ChangeToImmediate(Offset);
  } else {
    // For large offsets, try scavenging a register; fall back to R12 (scratch)
    Register ScratchReg;
    if (RS)
      ScratchReg = RS->scavengeRegisterBackwards(
          CPUTwo::GPRRegClass, II, /*RestoreAfter=*/false, SPAdj,
          /*AllowSpill=*/true);
    if (!ScratchReg)
      ScratchReg = CPUTwo::R12;

    DebugLoc DL = MI.getDebugLoc();
    MachineBasicBlock &MBB = *MI.getParent();

    // LUI scratch, hi16(offset)
    // ORI scratch, scratch, lo16(offset)
    // ADD scratch, SP, scratch
    BuildMI(MBB, II, DL, TII.get(CPUTwo::LUI), ScratchReg)
        .addImm((Offset >> 16) & 0xFFFF);
    BuildMI(MBB, II, DL, TII.get(CPUTwo::ORI), ScratchReg)
        .addReg(ScratchReg)
        .addImm(Offset & 0xFFFF);
    BuildMI(MBB, II, DL, TII.get(CPUTwo::ADD), ScratchReg)
        .addReg(CPUTwo::SP)
        .addReg(ScratchReg);

    MI.getOperand(FIOperandNum).ChangeToRegister(ScratchReg, false);
    MI.getOperand(FIOperandNum + 1).ChangeToImmediate(0);
  }

  return false;
}

Register
CPUTwoRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  return CPUTwo::SP;
}
