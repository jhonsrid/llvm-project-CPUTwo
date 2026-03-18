//===-- CPUTwoInstrInfo.cpp - CPUTwo Instruction Information ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "CPUTwoInstrInfo.h"
#include "CPUTwo.h"
#include "CPUTwoSubtarget.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineMemOperand.h"

using namespace llvm;

#define GET_INSTRINFO_CTOR_DTOR
#include "CPUTwoGenInstrInfo.inc"

CPUTwoInstrInfo::CPUTwoInstrInfo(const CPUTwoSubtarget &STI)
    : CPUTwoGenInstrInfo(STI, RegisterInfo,
                         CPUTwo::ADJCALLSTACKDOWN, CPUTwo::ADJCALLSTACKUP),
      RegisterInfo() {}

void CPUTwoInstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                   MachineBasicBlock::iterator Position,
                                   const DebugLoc &DL, Register DestReg,
                                   Register SrcReg, bool KillSource,
                                   bool RenamableDest,
                                   bool RenamableSrc) const {
  if (!CPUTwo::GPRRegClass.contains(DestReg) ||
      !CPUTwo::GPRRegClass.contains(SrcReg)) {
    llvm_unreachable("Impossible reg-to-reg copy");
  }

  BuildMI(MBB, Position, DL, get(CPUTwo::MOV), DestReg)
      .addReg(SrcReg, getKillRegState(KillSource));
}

void CPUTwoInstrInfo::storeRegToStackSlot(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator Position,
    Register SrcReg, bool IsKill, int FrameIndex,
    const TargetRegisterClass *RegClass, Register VReg,
    MachineInstr::MIFlag Flags) const {
  DebugLoc DL;
  if (Position != MBB.end())
    DL = Position->getDebugLoc();

  MachineFunction &MF = *MBB.getParent();
  MachineFrameInfo &MFI = MF.getFrameInfo();

  MachineMemOperand *MMO = MF.getMachineMemOperand(
      MachinePointerInfo::getFixedStack(MF, FrameIndex),
      MachineMemOperand::MOStore, MFI.getObjectSize(FrameIndex),
      MFI.getObjectAlign(FrameIndex));

  BuildMI(MBB, Position, DL, get(CPUTwo::SW))
      .addReg(SrcReg, getKillRegState(IsKill))
      .addFrameIndex(FrameIndex)
      .addImm(0)
      .addMemOperand(MMO)
      .setMIFlag(Flags);
}

void CPUTwoInstrInfo::loadRegFromStackSlot(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator Position,
    Register DestReg, int FrameIndex, const TargetRegisterClass *RegClass,
    Register VReg, unsigned SubReg, MachineInstr::MIFlag Flags) const {
  DebugLoc DL;
  if (Position != MBB.end())
    DL = Position->getDebugLoc();

  MachineFunction &MF = *MBB.getParent();
  MachineFrameInfo &MFI = MF.getFrameInfo();

  MachineMemOperand *MMO = MF.getMachineMemOperand(
      MachinePointerInfo::getFixedStack(MF, FrameIndex),
      MachineMemOperand::MOLoad, MFI.getObjectSize(FrameIndex),
      MFI.getObjectAlign(FrameIndex));

  BuildMI(MBB, Position, DL, get(CPUTwo::LW), DestReg)
      .addFrameIndex(FrameIndex)
      .addImm(0)
      .addMemOperand(MMO)
      .setMIFlag(Flags);
}

bool CPUTwoInstrInfo::expandPostRAPseudo(MachineInstr &MI) const {
  switch (MI.getOpcode()) {
  case CPUTwo::RET:
    // RET -> MOV PC, LR
    BuildMI(*MI.getParent(), MI, MI.getDebugLoc(), get(CPUTwo::MOV), CPUTwo::PC)
        .addReg(CPUTwo::LR);
    MI.eraseFromParent();
    return true;
  default:
    return false;
  }
}
