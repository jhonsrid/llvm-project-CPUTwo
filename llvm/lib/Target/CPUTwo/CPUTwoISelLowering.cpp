//===-- CPUTwoISelLowering.cpp - CPUTwo DAG Lowering Implementation -------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "CPUTwoISelLowering.h"
#include "CPUTwo.h"
#include "CPUTwoSubtarget.h"
#include "CPUTwoTargetMachine.h"
#include "CPUTwoCondCode.h"
#include "CPUTwoMachineFunctionInfo.h"
#include "MCTargetDesc/CPUTwoMCAsmInfo.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineJumpTableInfo.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/IR/Function.h"

using namespace llvm;

#define DEBUG_TYPE "cputwo-lower"

#include "CPUTwoGenCallingConv.inc"

CPUTwoTargetLowering::CPUTwoTargetLowering(const TargetMachine &TM,
                                             const CPUTwoSubtarget &STI)
    : TargetLowering(TM, STI), Subtarget(STI) {

  addRegisterClass(MVT::i32, &CPUTwo::GPRRegClass);
  computeRegisterProperties(STI.getRegisterInfo());

  setStackPointerRegisterToSaveRestore(CPUTwo::SP);

  // Branch/compare lowering
  setOperationAction(ISD::BR_CC, MVT::i32, Custom);
  setOperationAction(ISD::BRCOND, MVT::Other, Expand);
  setOperationAction(ISD::SELECT, MVT::i32, Expand);
  setOperationAction(ISD::SELECT_CC, MVT::i32, Custom);
  setOperationAction(ISD::SETCC, MVT::i32, Expand);

  // Address materialization
  setOperationAction(ISD::GlobalAddress, MVT::i32, Custom);
  setOperationAction(ISD::BlockAddress, MVT::i32, Custom);
  setOperationAction(ISD::JumpTable, MVT::i32, Custom);
  setOperationAction(ISD::ConstantPool, MVT::i32, Custom);

  // No native support for these
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i1, Expand);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i8, Expand);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i16, Expand);

  // No bswap instruction
  setOperationAction(ISD::BSWAP, MVT::i32, Expand);
  setOperationAction(ISD::BSWAP, MVT::i16, Expand);

  // Expand i64 operations to i32 pairs
  setOperationAction(ISD::SHL_PARTS, MVT::i32, Expand);
  setOperationAction(ISD::SRA_PARTS, MVT::i32, Expand);
  setOperationAction(ISD::SRL_PARTS, MVT::i32, Expand);

  // No FPU
  setOperationAction(ISD::UINT_TO_FP, MVT::i32, Expand);
  setOperationAction(ISD::SINT_TO_FP, MVT::i32, Expand);
  setOperationAction(ISD::FP_TO_UINT, MVT::i32, Expand);
  setOperationAction(ISD::FP_TO_SINT, MVT::i32, Expand);

  // Multiply hi/lo — expand to separate MUL + MULH/MULHU
  setOperationAction(ISD::SMUL_LOHI, MVT::i32, Expand);
  setOperationAction(ISD::UMUL_LOHI, MVT::i32, Expand);

  // No bit-counting instructions
  setOperationAction(ISD::CTPOP, MVT::i32, Expand);
  setOperationAction(ISD::CTLZ, MVT::i32, Expand);
  setOperationAction(ISD::CTTZ, MVT::i32, Expand);

  // Variable arguments
  setOperationAction(ISD::VASTART, MVT::Other, Custom);
  setOperationAction(ISD::VAARG, MVT::Other, Expand);
  setOperationAction(ISD::VACOPY, MVT::Other, Expand);
  setOperationAction(ISD::VAEND, MVT::Other, Expand);

  // Jump tables — expand BR_JT to BRIND (indirect branch)
  setOperationAction(ISD::BR_JT, MVT::Other, Expand);

  // Disable jump tables — use binary search trees for switch statements.
  // Jump table support requires fixing the integrated assembler to handle
  // HI16/LO16 relocations referencing temporary symbols.
  setMinimumJumpTableEntries(INT_MAX);

  // Dynamic stack allocation
  setOperationAction(ISD::DYNAMIC_STACKALLOC, MVT::i32, Custom);
  setOperationAction(ISD::STACKSAVE, MVT::Other, Expand);
  setOperationAction(ISD::STACKRESTORE, MVT::Other, Expand);

  // Frame/return address
  setOperationAction(ISD::FRAMEADDR, MVT::i32, Custom);
  setOperationAction(ISD::RETURNADDR, MVT::i32, Custom);

  // Extended loads for i1 are promoted
  for (MVT VT : MVT::integer_valuetypes()) {
    setLoadExtAction(ISD::EXTLOAD, VT, MVT::i1, Promote);
    setLoadExtAction(ISD::ZEXTLOAD, VT, MVT::i1, Promote);
    setLoadExtAction(ISD::SEXTLOAD, VT, MVT::i1, Promote);
  }

  setBooleanContents(ZeroOrOneBooleanContent);
  setMaxAtomicSizeInBitsSupported(32);

  setMinFunctionAlignment(Align(4));
  setPrefFunctionAlignment(Align(4));
}

SDValue CPUTwoTargetLowering::LowerOperation(SDValue Op,
                                              SelectionDAG &DAG) const {
  switch (Op.getOpcode()) {
  case ISD::GlobalAddress:
    return LowerGlobalAddress(Op, DAG);
  case ISD::BlockAddress:
    return LowerBlockAddress(Op, DAG);
  case ISD::ConstantPool:
    return LowerConstantPool(Op, DAG);
  case ISD::JumpTable:
    return LowerJumpTable(Op, DAG);
  case ISD::BR_CC:
    return LowerBR_CC(Op, DAG);
  case ISD::SELECT_CC:
    return LowerSELECT_CC(Op, DAG);
  case ISD::VASTART:
    return LowerVASTART(Op, DAG);
  case ISD::FRAMEADDR:
    return LowerFRAMEADDR(Op, DAG);
  case ISD::RETURNADDR:
    return LowerRETURNADDR(Op, DAG);
  case ISD::DYNAMIC_STACKALLOC:
    return LowerDYNAMIC_STACKALLOC(Op, DAG);
  default:
    report_fatal_error("unimplemented operand");
  }
}

const char *CPUTwoTargetLowering::getTargetNodeName(unsigned Opcode) const {
  switch (static_cast<CPUTwoISD::NodeType>(Opcode)) {
  case CPUTwoISD::FIRST_NUMBER:
    break;
  case CPUTwoISD::RET:
    return "CPUTwoISD::RET";
  case CPUTwoISD::CALL:
    return "CPUTwoISD::CALL";
  case CPUTwoISD::CMP:
    return "CPUTwoISD::CMP";
  case CPUTwoISD::BRCOND:
    return "CPUTwoISD::BRCOND";
  case CPUTwoISD::SELECT_CC:
    return "CPUTwoISD::SELECT_CC";
  case CPUTwoISD::HI16:
    return "CPUTwoISD::HI16";
  case CPUTwoISD::LO16:
    return "CPUTwoISD::LO16";
  case CPUTwoISD::WRAPPER:
    return "CPUTwoISD::WRAPPER";
  }
  return nullptr;
}

unsigned CPUTwoTargetLowering::getJumpTableEncoding() const {
  // Jump tables are disabled (setMinimumJumpTableEntries(INT_MAX)) because
  // the integrated assembler can't emit relocations for temporary symbols
  // used in jump table address materialization. Switch statements are
  // lowered as binary search trees instead.
  return MachineJumpTableInfo::EK_BlockAddress;
}

//===----------------------------------------------------------------------===//
// Address materialization
//===----------------------------------------------------------------------===//

SDValue CPUTwoTargetLowering::LowerGlobalAddress(SDValue Op,
                                                  SelectionDAG &DAG) const {
  SDLoc DL(Op);
  const GlobalAddressSDNode *GN = cast<GlobalAddressSDNode>(Op);
  const GlobalValue *GV = GN->getGlobal();
  int64_t Offset = GN->getOffset();

  SDValue Hi = DAG.getTargetGlobalAddress(GV, DL, MVT::i32, Offset,
                                           CPUTwo::S_HI16);
  SDValue Lo = DAG.getTargetGlobalAddress(GV, DL, MVT::i32, Offset,
                                           CPUTwo::S_LO16);

  SDValue LuiNode = DAG.getNode(CPUTwoISD::HI16, DL, MVT::i32, Hi);
  return DAG.getNode(CPUTwoISD::LO16, DL, MVT::i32, LuiNode, Lo);
}

SDValue CPUTwoTargetLowering::LowerBlockAddress(SDValue Op,
                                                 SelectionDAG &DAG) const {
  SDLoc DL(Op);
  const BlockAddressSDNode *BA = cast<BlockAddressSDNode>(Op);
  SDValue Hi =
      DAG.getTargetBlockAddress(BA->getBlockAddress(), MVT::i32, 0,
                                CPUTwo::S_HI16);
  SDValue Lo =
      DAG.getTargetBlockAddress(BA->getBlockAddress(), MVT::i32, 0,
                                CPUTwo::S_LO16);
  SDValue LuiNode = DAG.getNode(CPUTwoISD::HI16, DL, MVT::i32, Hi);
  return DAG.getNode(CPUTwoISD::LO16, DL, MVT::i32, LuiNode, Lo);
}

SDValue CPUTwoTargetLowering::LowerConstantPool(SDValue Op,
                                                  SelectionDAG &DAG) const {
  SDLoc DL(Op);
  ConstantPoolSDNode *CP = cast<ConstantPoolSDNode>(Op);
  SDValue Hi = DAG.getTargetConstantPool(CP->getConstVal(), MVT::i32,
                                          CP->getAlign(), CP->getOffset(),
                                          CPUTwo::S_HI16);
  SDValue Lo = DAG.getTargetConstantPool(CP->getConstVal(), MVT::i32,
                                          CP->getAlign(), CP->getOffset(),
                                          CPUTwo::S_LO16);
  SDValue LuiNode = DAG.getNode(CPUTwoISD::HI16, DL, MVT::i32, Hi);
  return DAG.getNode(CPUTwoISD::LO16, DL, MVT::i32, LuiNode, Lo);
}

SDValue CPUTwoTargetLowering::LowerJumpTable(SDValue Op,
                                              SelectionDAG &DAG) const {
  SDLoc DL(Op);
  JumpTableSDNode *JT = cast<JumpTableSDNode>(Op);
  SDValue Hi = DAG.getTargetJumpTable(JT->getIndex(), MVT::i32,
                                       CPUTwo::S_HI16);
  SDValue Lo = DAG.getTargetJumpTable(JT->getIndex(), MVT::i32,
                                       CPUTwo::S_LO16);
  SDValue LuiNode = DAG.getNode(CPUTwoISD::HI16, DL, MVT::i32, Hi);
  return DAG.getNode(CPUTwoISD::LO16, DL, MVT::i32, LuiNode, Lo);
}

//===----------------------------------------------------------------------===//
// Branch/compare lowering
//===----------------------------------------------------------------------===//

static CPUTwoCC::CondCode intCondCodeToCC(ISD::CondCode CC) {
  switch (CC) {
  case ISD::SETEQ:  return CPUTwoCC::CC_EQ;
  case ISD::SETNE:  return CPUTwoCC::CC_NE;
  case ISD::SETLT:  return CPUTwoCC::CC_LT;
  case ISD::SETGE:  return CPUTwoCC::CC_GE;
  case ISD::SETGT:  return CPUTwoCC::CC_GT;
  case ISD::SETLE:  return CPUTwoCC::CC_LE;
  case ISD::SETULT: return CPUTwoCC::CC_LTU;
  case ISD::SETUGE: return CPUTwoCC::CC_GEU;
  case ISD::SETUGT: return CPUTwoCC::CC_GTU;
  case ISD::SETULE: return CPUTwoCC::CC_LEU;
  default:
    llvm_unreachable("Unsupported condition code");
  }
}

SDValue CPUTwoTargetLowering::LowerBR_CC(SDValue Op,
                                          SelectionDAG &DAG) const {
  SDLoc DL(Op);
  SDValue Chain = Op.getOperand(0);
  ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(1))->get();
  SDValue LHS = Op.getOperand(2);
  SDValue RHS = Op.getOperand(3);
  SDValue Dest = Op.getOperand(4);

  CPUTwoCC::CondCode TargetCC = intCondCodeToCC(CC);
  SDValue Cmp = DAG.getNode(CPUTwoISD::CMP, DL, MVT::Glue, LHS, RHS);
  SDValue CCVal = DAG.getConstant(TargetCC, DL, MVT::i32);
  return DAG.getNode(CPUTwoISD::BRCOND, DL, MVT::Other, Chain, Dest, CCVal,
                     Cmp);
}

SDValue CPUTwoTargetLowering::LowerSELECT_CC(SDValue Op,
                                               SelectionDAG &DAG) const {
  SDLoc DL(Op);
  SDValue LHS = Op.getOperand(0);
  SDValue RHS = Op.getOperand(1);
  SDValue TrueV = Op.getOperand(2);
  SDValue FalseV = Op.getOperand(3);
  ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(4))->get();

  CPUTwoCC::CondCode TargetCC = intCondCodeToCC(CC);
  SDValue Cmp = DAG.getNode(CPUTwoISD::CMP, DL, MVT::Glue, LHS, RHS);
  SDValue CCVal = DAG.getConstant(TargetCC, DL, MVT::i32);
  return DAG.getNode(CPUTwoISD::SELECT_CC, DL, Op.getValueType(), TrueV,
                     FalseV, CCVal, Cmp);
}

SDValue CPUTwoTargetLowering::LowerVASTART(SDValue Op,
                                             SelectionDAG &DAG) const {
  MachineFunction &MF = DAG.getMachineFunction();
  auto *FI = MF.getInfo<CPUTwoMachineFunctionInfo>();
  SDLoc DL(Op);

  // vastart stores the address of the first vararg on the stack
  SDValue FIN = DAG.getFrameIndex(FI->getVarArgsFrameIndex(), MVT::i32);
  const Value *SV = cast<SrcValueSDNode>(Op.getOperand(2))->getValue();
  return DAG.getStore(Op.getOperand(0), DL, FIN, Op.getOperand(1),
                      MachinePointerInfo(SV));
}

SDValue CPUTwoTargetLowering::LowerFRAMEADDR(SDValue Op,
                                               SelectionDAG &DAG) const {
  MachineFrameInfo &MFI = DAG.getMachineFunction().getFrameInfo();
  MFI.setFrameAddressIsTaken(true);
  SDLoc DL(Op);
  SDValue FrameAddr = DAG.getCopyFromReg(DAG.getEntryNode(), DL, CPUTwo::SP,
                                          MVT::i32);
  return FrameAddr;
}

SDValue CPUTwoTargetLowering::LowerRETURNADDR(SDValue Op,
                                                SelectionDAG &DAG) const {
  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  MFI.setReturnAddressIsTaken(true);

  SDLoc DL(Op);
  // Mark LR as an implicit live-in and use the virtual register copy.
  Register Reg = MF.addLiveIn(CPUTwo::LR, &CPUTwo::GPRRegClass);
  return DAG.getCopyFromReg(DAG.getEntryNode(), DL, Reg, MVT::i32);
}

SDValue CPUTwoTargetLowering::LowerDYNAMIC_STACKALLOC(SDValue Op,
                                                        SelectionDAG &DAG) const {
  SDLoc DL(Op);
  SDValue Chain = Op.getOperand(0);
  SDValue Size = Op.getOperand(1);

  Register SPReg = CPUTwo::SP;
  SDValue SP = DAG.getCopyFromReg(Chain, DL, SPReg, MVT::i32);
  SDValue NewSP = DAG.getNode(ISD::SUB, DL, MVT::i32, SP, Size);
  Chain = DAG.getCopyToReg(SP.getValue(1), DL, SPReg, NewSP);

  SDValue Ops[2] = {NewSP, Chain};
  return DAG.getMergeValues(Ops, DL);
}

//===----------------------------------------------------------------------===//
// Custom inserter for SELECT_CC pseudo
//===----------------------------------------------------------------------===//

MachineBasicBlock *CPUTwoTargetLowering::EmitInstrWithCustomInserter(
    MachineInstr &MI, MachineBasicBlock *MBB) const {
  assert(MI.getOpcode() == CPUTwo::SELECT_CC &&
         "Unexpected instruction for custom inserter");

  const TargetInstrInfo &TII = *Subtarget.getInstrInfo();
  DebugLoc DL = MI.getDebugLoc();

  // Diamond:
  //   MBB:
  //     CMP ... (flags already set by glue)
  //     Bcc TrueMBB
  //   FalseMBB:
  //     ... (fallthrough)
  //   TrueMBB:
  //     ... (fallthrough)
  //   SinkMBB:
  //     %result = PHI [TrueV, TrueMBB], [FalseV, FalseMBB]

  Register DstReg = MI.getOperand(0).getReg();
  Register TrueReg = MI.getOperand(1).getReg();
  Register FalseReg = MI.getOperand(2).getReg();
  auto CC = static_cast<CPUTwoCC::CondCode>(MI.getOperand(3).getImm());

  MachineFunction *MF = MBB->getParent();
  MachineBasicBlock *FalseMBB = MF->CreateMachineBasicBlock();
  MachineBasicBlock *SinkMBB = MF->CreateMachineBasicBlock();

  MachineFunction::iterator It = ++MBB->getIterator();
  MF->insert(It, FalseMBB);
  MF->insert(It, SinkMBB);

  // Transfer successors and remaining instructions to SinkMBB
  SinkMBB->splice(SinkMBB->begin(), MBB,
                  std::next(MachineBasicBlock::iterator(MI)), MBB->end());
  SinkMBB->transferSuccessorsAndUpdatePHIs(MBB);

  MBB->addSuccessor(FalseMBB);
  MBB->addSuccessor(SinkMBB);
  FalseMBB->addSuccessor(SinkMBB);

  // Emit conditional branch: if CC, goto SinkMBB (true path)
  unsigned BrOpc;
  switch (CC) {
  case CPUTwoCC::CC_EQ:  BrOpc = CPUTwo::BEQ; break;
  case CPUTwoCC::CC_NE:  BrOpc = CPUTwo::BNE; break;
  case CPUTwoCC::CC_LT:  BrOpc = CPUTwo::BLT; break;
  case CPUTwoCC::CC_GE:  BrOpc = CPUTwo::BGE; break;
  case CPUTwoCC::CC_LTU: BrOpc = CPUTwo::BLTU; break;
  case CPUTwoCC::CC_GEU: BrOpc = CPUTwo::BGEU; break;
  case CPUTwoCC::CC_GT:  BrOpc = CPUTwo::BGT; break;
  case CPUTwoCC::CC_LE:  BrOpc = CPUTwo::BLE; break;
  case CPUTwoCC::CC_GTU: BrOpc = CPUTwo::BGTU; break;
  case CPUTwoCC::CC_LEU: BrOpc = CPUTwo::BLEU; break;
  default: llvm_unreachable("Invalid CC");
  }

  BuildMI(MBB, DL, TII.get(BrOpc)).addMBB(SinkMBB);

  // FalseMBB is empty, falls through to SinkMBB

  // SinkMBB: PHI node
  BuildMI(*SinkMBB, SinkMBB->begin(), DL, TII.get(CPUTwo::PHI), DstReg)
      .addReg(TrueReg)
      .addMBB(MBB)
      .addReg(FalseReg)
      .addMBB(FalseMBB);

  MI.eraseFromParent();
  return SinkMBB;
}

//===----------------------------------------------------------------------===//
// Calling Convention
//===----------------------------------------------------------------------===//

SDValue CPUTwoTargetLowering::LowerFormalArguments(
    SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &DL,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {

  MachineFunction &MF = DAG.getMachineFunction();
  MachineRegisterInfo &RegInfo = MF.getRegInfo();

  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, IsVarArg, MF, ArgLocs, *DAG.getContext());
  CCInfo.AnalyzeFormalArguments(Ins, CC_CPUTwo);

  for (auto &VA : ArgLocs) {
    if (VA.isRegLoc()) {
      Register VReg = RegInfo.createVirtualRegister(&CPUTwo::GPRRegClass);
      RegInfo.addLiveIn(VA.getLocReg(), VReg);
      SDValue ArgVal = DAG.getCopyFromReg(Chain, DL, VReg, MVT::i32);
      InVals.push_back(ArgVal);
    } else {
      // Stack argument
      assert(VA.isMemLoc());
      int FI = MF.getFrameInfo().CreateFixedObject(4, VA.getLocMemOffset(),
                                                     true);
      SDValue FIN = DAG.getFrameIndex(FI, MVT::i32);
      InVals.push_back(
          DAG.getLoad(MVT::i32, DL, Chain, FIN, MachinePointerInfo()));
    }
  }

  if (IsVarArg) {
    // For varargs, save unused argument registers to the stack immediately
    // below the caller's stack arguments so va_arg can walk contiguously
    // from register-saved args into stack-passed args.
    //
    // Caller's frame (at incoming SP):
    //   [SP + 4]  5th arg (stack arg 1)
    //   [SP + 0]  5th arg (stack arg 0)  <-- CCInfo.getStackSize() bytes
    //   [SP - 4]  saved r3               <-- we create these
    //   [SP - 8]  saved r2
    //   [SP - 12] saved r1  <-- VarArgsFrameIndex points here
    //
    // Fixed objects have offsets relative to incoming SP. Negative offsets
    // are below incoming SP (in the callee's frame).
    static const MCPhysReg ArgRegs[] = {CPUTwo::R0, CPUTwo::R1, CPUTwo::R2,
                                        CPUTwo::R3};
    unsigned FirstVarArgReg = 0;
    for (auto &VA : ArgLocs)
      if (VA.isRegLoc())
        FirstVarArgReg++;

    unsigned NumSavedRegs = 4 - FirstVarArgReg;
    MachineFrameInfo &MFI = MF.getFrameInfo();
    SmallVector<SDValue, 4> MemOps;

    // Save each register to a fixed stack slot below incoming SP.
    // Saved in ascending address order: first vararg at lowest address.
    SmallVector<int, 4> FIs;
    for (unsigned i = 0; i < NumSavedRegs; ++i) {
      // Offset from incoming SP: -(NumSavedRegs - i) * 4
      int Offset = -static_cast<int>(NumSavedRegs - i) * 4;
      int FI = MFI.CreateFixedObject(4, Offset, true);
      FIs.push_back(FI);

      SDValue FIN = DAG.getFrameIndex(FI, MVT::i32);
      Register VReg = RegInfo.createVirtualRegister(&CPUTwo::GPRRegClass);
      RegInfo.addLiveIn(ArgRegs[FirstVarArgReg + i], VReg);
      SDValue Val = DAG.getCopyFromReg(Chain, DL, VReg, MVT::i32);
      MemOps.push_back(
          DAG.getStore(Chain, DL, Val, FIN, MachinePointerInfo()));
    }

    if (!MemOps.empty())
      Chain = DAG.getNode(ISD::TokenFactor, DL, MVT::Other, MemOps);

    // VarArgsFrameIndex: first vararg's slot (lowest address)
    auto *CPUFI = MF.getInfo<CPUTwoMachineFunctionInfo>();
    if (NumSavedRegs > 0)
      CPUFI->setVarArgsFrameIndex(FIs[0]);
    else {
      // All 4 arg regs used for named params; varargs start on caller stack
      int FI = MFI.CreateFixedObject(4, CCInfo.getStackSize(), true);
      CPUFI->setVarArgsFrameIndex(FI);
    }
  }

  return Chain;
}

SDValue CPUTwoTargetLowering::LowerReturn(
    SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
    const SmallVectorImpl<ISD::OutputArg> &Outs,
    const SmallVectorImpl<SDValue> &OutVals, const SDLoc &DL,
    SelectionDAG &DAG) const {

  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, IsVarArg, DAG.getMachineFunction(), RVLocs,
                 *DAG.getContext());
  CCInfo.AnalyzeReturn(Outs, RetCC_CPUTwo);

  SDValue Glue;
  SmallVector<SDValue, 4> RetOps;
  RetOps.push_back(Chain);

  for (unsigned i = 0; i < RVLocs.size(); ++i) {
    CCValAssign &VA = RVLocs[i];
    Chain = DAG.getCopyToReg(Chain, DL, VA.getLocReg(), OutVals[i], Glue);
    Glue = Chain.getValue(1);
    RetOps.push_back(DAG.getRegister(VA.getLocReg(), VA.getLocVT()));
  }

  RetOps[0] = Chain;
  if (Glue.getNode())
    RetOps.push_back(Glue);

  return DAG.getNode(CPUTwoISD::RET, DL, MVT::Other, RetOps);
}

SDValue CPUTwoTargetLowering::LowerCall(
    TargetLowering::CallLoweringInfo &CLI,
    SmallVectorImpl<SDValue> &InVals) const {

  // Tail calls not yet supported
  CLI.IsTailCall = false;

  SelectionDAG &DAG = CLI.DAG;
  SDLoc &DL = CLI.DL;
  SmallVectorImpl<ISD::OutputArg> &Outs = CLI.Outs;
  SmallVectorImpl<SDValue> &OutVals = CLI.OutVals;
  SmallVectorImpl<ISD::InputArg> &Ins = CLI.Ins;
  SDValue Chain = CLI.Chain;
  SDValue Callee = CLI.Callee;
  CallingConv::ID CallConv = CLI.CallConv;
  bool IsVarArg = CLI.IsVarArg;

  MachineFunction &MF = DAG.getMachineFunction();

  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, IsVarArg, MF, ArgLocs, *DAG.getContext());
  CCInfo.AnalyzeCallOperands(Outs, CC_CPUTwo);

  unsigned NumBytes = CCInfo.getStackSize();

  // Adjust stack pointer
  Chain = DAG.getCALLSEQ_START(Chain, NumBytes, 0, DL);

  SmallVector<std::pair<Register, SDValue>, 4> RegsToPass;
  SmallVector<SDValue, 8> MemOpChains;

  SDValue StackPtr;
  for (unsigned i = 0; i < ArgLocs.size(); ++i) {
    CCValAssign &VA = ArgLocs[i];
    SDValue Arg = OutVals[i];

    if (VA.isRegLoc()) {
      RegsToPass.push_back(std::make_pair(VA.getLocReg(), Arg));
    } else {
      assert(VA.isMemLoc());
      if (!StackPtr.getNode())
        StackPtr = DAG.getCopyFromReg(Chain, DL, CPUTwo::SP, MVT::i32);
      SDValue Address =
          DAG.getNode(ISD::ADD, DL, MVT::i32, StackPtr,
                      DAG.getIntPtrConstant(VA.getLocMemOffset(), DL));
      MemOpChains.push_back(
          DAG.getStore(Chain, DL, Arg, Address, MachinePointerInfo()));
    }
  }

  if (!MemOpChains.empty())
    Chain = DAG.getNode(ISD::TokenFactor, DL, MVT::Other, MemOpChains);

  SDValue Glue;
  for (auto &Reg : RegsToPass) {
    Chain = DAG.getCopyToReg(Chain, DL, Reg.first, Reg.second, Glue);
    Glue = Chain.getValue(1);
  }

  // Wrap the callee for direct calls to prevent address materialization
  if (auto *GA = dyn_cast<GlobalAddressSDNode>(Callee))
    Callee = DAG.getTargetGlobalAddress(GA->getGlobal(), DL, MVT::i32,
                                         GA->getOffset());
  else if (auto *ES = dyn_cast<ExternalSymbolSDNode>(Callee))
    Callee = DAG.getTargetExternalSymbol(ES->getSymbol(), MVT::i32);

  // Build call operands
  SmallVector<SDValue, 8> Ops;
  Ops.push_back(Chain);
  Ops.push_back(Callee);

  for (auto &Reg : RegsToPass)
    Ops.push_back(DAG.getRegister(Reg.first, Reg.second.getValueType()));

  const TargetRegisterInfo *TRI = Subtarget.getRegisterInfo();
  const uint32_t *Mask = TRI->getCallPreservedMask(MF, CallConv);
  Ops.push_back(DAG.getRegisterMask(Mask));

  if (Glue.getNode())
    Ops.push_back(Glue);

  SDVTList NodeTys = DAG.getVTList(MVT::Other, MVT::Glue);
  Chain = DAG.getNode(CPUTwoISD::CALL, DL, NodeTys, Ops);
  Glue = Chain.getValue(1);

  Chain = DAG.getCALLSEQ_END(Chain, NumBytes, 0, Glue, DL);
  Glue = Chain.getValue(1);

  // Copy return values
  SmallVector<CCValAssign, 16> RVLocs;
  CCState RVInfo(CallConv, IsVarArg, MF, RVLocs, *DAG.getContext());
  RVInfo.AnalyzeCallResult(Ins, RetCC_CPUTwo);

  for (auto &VA : RVLocs) {
    Chain = DAG.getCopyFromReg(Chain, DL, VA.getLocReg(), VA.getValVT(), Glue)
                .getValue(1);
    InVals.push_back(Chain.getValue(0));
    Glue = Chain.getValue(2);
  }

  return Chain;
}
