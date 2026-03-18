//===-- CPUTwoISelLowering.h - CPUTwo DAG Lowering Interface ----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CPUTWO_CPUTWOISELLOWERING_H
#define LLVM_LIB_TARGET_CPUTWO_CPUTWOISELLOWERING_H

#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/CodeGen/TargetLowering.h"

namespace llvm {

namespace CPUTwoISD {
enum NodeType : unsigned {
  FIRST_NUMBER = ISD::BUILTIN_OP_END,
  RET,
  CALL,
  CMP,
  BRCOND,
  SELECT_CC,
  HI16,
  LO16,
  WRAPPER,
};
} // namespace CPUTwoISD

class CPUTwoSubtarget;

class CPUTwoTargetLowering : public TargetLowering {
public:
  CPUTwoTargetLowering(const TargetMachine &TM, const CPUTwoSubtarget &STI);

  SDValue LowerOperation(SDValue Op, SelectionDAG &DAG) const override;

  const char *getTargetNodeName(unsigned Opcode) const override;

private:
  SDValue LowerGlobalAddress(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerBlockAddress(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerConstantPool(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerJumpTable(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerBR_CC(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerSELECT_CC(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerVASTART(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerFRAMEADDR(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerRETURNADDR(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerDYNAMIC_STACKALLOC(SDValue Op, SelectionDAG &DAG) const;

  SDValue LowerCall(TargetLowering::CallLoweringInfo &CLI,
                    SmallVectorImpl<SDValue> &InVals) const override;
  SDValue LowerFormalArguments(SDValue Chain, CallingConv::ID CallConv,
                               bool IsVarArg,
                               const SmallVectorImpl<ISD::InputArg> &Ins,
                               const SDLoc &DL, SelectionDAG &DAG,
                               SmallVectorImpl<SDValue> &InVals) const override;
  SDValue LowerReturn(SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
                      const SmallVectorImpl<ISD::OutputArg> &Outs,
                      const SmallVectorImpl<SDValue> &OutVals, const SDLoc &DL,
                      SelectionDAG &DAG) const override;

  const CPUTwoSubtarget &Subtarget;
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_CPUTWO_CPUTWOISELLOWERING_H
