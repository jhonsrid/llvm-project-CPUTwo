//===-- CPUTwoInstrInfo.cpp - CPUTwo Instruction Information ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "CPUTwoInstrInfo.h"
#include "CPUTwo.h"
#include "CPUTwoCondCode.h"
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

static unsigned getBranchOpcodeForCC(CPUTwoCC::CondCode CC);

bool CPUTwoInstrInfo::expandPostRAPseudo(MachineInstr &MI) const {
  switch (MI.getOpcode()) {
  case CPUTwo::RET:
    // RET -> MOV PC, LR
    BuildMI(*MI.getParent(), MI, MI.getDebugLoc(), get(CPUTwo::MOV), CPUTwo::PC)
        .addReg(CPUTwo::LR);
    MI.eraseFromParent();
    return true;
  case CPUTwo::BRCC: {
    // BRCC target, cc -> Bcc target
    auto CC = static_cast<CPUTwoCC::CondCode>(MI.getOperand(1).getImm());
    unsigned Opc = getBranchOpcodeForCC(CC);
    BuildMI(*MI.getParent(), MI, MI.getDebugLoc(), get(Opc))
        .addMBB(MI.getOperand(0).getMBB());
    MI.eraseFromParent();
    return true;
  }
  default:
    return false;
  }
}

static unsigned getBranchOpcodeForCC(CPUTwoCC::CondCode CC) {
  switch (CC) {
  case CPUTwoCC::CC_EQ:  return CPUTwo::BEQ;
  case CPUTwoCC::CC_NE:  return CPUTwo::BNE;
  case CPUTwoCC::CC_LT:  return CPUTwo::BLT;
  case CPUTwoCC::CC_GE:  return CPUTwo::BGE;
  case CPUTwoCC::CC_LTU: return CPUTwo::BLTU;
  case CPUTwoCC::CC_GEU: return CPUTwo::BGEU;
  case CPUTwoCC::CC_AL:  return CPUTwo::BA;
  case CPUTwoCC::CC_GT:  return CPUTwo::BGT;
  case CPUTwoCC::CC_LE:  return CPUTwo::BLE;
  case CPUTwoCC::CC_GTU: return CPUTwo::BGTU;
  case CPUTwoCC::CC_LEU: return CPUTwo::BLEU;
  }
  llvm_unreachable("Invalid condition code");
}

static CPUTwoCC::CondCode getCondFromBranchOpc(unsigned Opc) {
  switch (Opc) {
  case CPUTwo::BEQ:  return CPUTwoCC::CC_EQ;
  case CPUTwo::BNE:  return CPUTwoCC::CC_NE;
  case CPUTwo::BLT:  return CPUTwoCC::CC_LT;
  case CPUTwo::BGE:  return CPUTwoCC::CC_GE;
  case CPUTwo::BLTU: return CPUTwoCC::CC_LTU;
  case CPUTwo::BGEU: return CPUTwoCC::CC_GEU;
  case CPUTwo::BA:   return CPUTwoCC::CC_AL;
  case CPUTwo::BGT:  return CPUTwoCC::CC_GT;
  case CPUTwo::BLE:  return CPUTwoCC::CC_LE;
  case CPUTwo::BGTU: return CPUTwoCC::CC_GTU;
  case CPUTwo::BLEU: return CPUTwoCC::CC_LEU;
  default:           return static_cast<CPUTwoCC::CondCode>(-1);
  }
}

bool CPUTwoInstrInfo::analyzeBranch(MachineBasicBlock &MBB,
                                     MachineBasicBlock *&TBB,
                                     MachineBasicBlock *&FBB,
                                     SmallVectorImpl<MachineOperand> &Cond,
                                     bool AllowModify) const {
  // Start from the bottom of the block and work up, examining the
  // terminator instructions.
  MachineBasicBlock::iterator I = MBB.end();
  if (I == MBB.begin())
    return false;
  --I;
  while (I->isDebugInstr()) {
    if (I == MBB.begin())
      return false;
    --I;
  }

  // Not a terminator — no branches
  if (!isUnpredicatedTerminator(*I))
    return false;

  // Get the last instruction
  MachineInstr *LastInst = &*I;

  // If there is only one terminator instruction, process it.
  if (I == MBB.begin() || !isUnpredicatedTerminator(*--I)) {
    if (LastInst->getOpcode() == CPUTwo::BA) {
      TBB = LastInst->getOperand(0).getMBB();
      return false;
    }
    CPUTwoCC::CondCode CC = getCondFromBranchOpc(LastInst->getOpcode());
    if (CC != static_cast<CPUTwoCC::CondCode>(-1) && CC != CPUTwoCC::CC_AL) {
      TBB = LastInst->getOperand(0).getMBB();
      Cond.push_back(MachineOperand::CreateImm(CC));
      return false;
    }
    return true; // Can't analyze
  }

  // Two terminators: should be conditional followed by unconditional
  MachineInstr *SecondLastInst = &*I;

  // If there are three terminators, we don't know what sort of block this is.
  if (I != MBB.begin() && isUnpredicatedTerminator(*--I))
    return true;

  // Conditional + unconditional
  if (LastInst->getOpcode() == CPUTwo::BA) {
    CPUTwoCC::CondCode CC = getCondFromBranchOpc(SecondLastInst->getOpcode());
    if (CC != static_cast<CPUTwoCC::CondCode>(-1) && CC != CPUTwoCC::CC_AL) {
      TBB = SecondLastInst->getOperand(0).getMBB();
      FBB = LastInst->getOperand(0).getMBB();
      Cond.push_back(MachineOperand::CreateImm(CC));
      return false;
    }
  }

  return true; // Can't analyze
}

unsigned CPUTwoInstrInfo::insertBranch(MachineBasicBlock &MBB,
                                        MachineBasicBlock *TBB,
                                        MachineBasicBlock *FBB,
                                        ArrayRef<MachineOperand> Cond,
                                        const DebugLoc &DL,
                                        int *BytesAdded) const {
  assert(TBB && "insertBranch must not be called with a null TBB");
  unsigned Count = 0;

  if (Cond.empty()) {
    // Unconditional branch
    BuildMI(&MBB, DL, get(CPUTwo::BA)).addMBB(TBB);
    if (BytesAdded)
      *BytesAdded = 4;
    return 1;
  }

  // Conditional branch
  auto CC = static_cast<CPUTwoCC::CondCode>(Cond[0].getImm());
  unsigned Opc = getBranchOpcodeForCC(CC);
  BuildMI(&MBB, DL, get(Opc)).addMBB(TBB);
  Count = 1;

  if (FBB) {
    BuildMI(&MBB, DL, get(CPUTwo::BA)).addMBB(FBB);
    Count = 2;
  }

  if (BytesAdded)
    *BytesAdded = Count * 4;
  return Count;
}

unsigned CPUTwoInstrInfo::removeBranch(MachineBasicBlock &MBB,
                                        int *BytesRemoved) const {
  unsigned Count = 0;
  MachineBasicBlock::iterator I = MBB.getLastNonDebugInstr();
  if (I == MBB.end())
    return 0;

  while (I != MBB.begin()) {
    unsigned Opc = I->getOpcode();
    if (Opc == CPUTwo::BA || getCondFromBranchOpc(Opc) !=
                                 static_cast<CPUTwoCC::CondCode>(-1)) {
      I->eraseFromParent();
      I = MBB.getLastNonDebugInstr();
      ++Count;
    } else {
      break;
    }
  }

  if (BytesRemoved)
    *BytesRemoved = Count * 4;
  return Count;
}

bool CPUTwoInstrInfo::reverseBranchCondition(
    SmallVectorImpl<MachineOperand> &Cond) const {
  assert(Cond.size() == 1);
  auto CC = static_cast<CPUTwoCC::CondCode>(Cond[0].getImm());
  Cond[0].setImm(CPUTwoCC::getOppositeCondition(CC));
  return false;
}
