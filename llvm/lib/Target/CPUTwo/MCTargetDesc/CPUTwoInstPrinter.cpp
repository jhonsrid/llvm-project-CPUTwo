//===-- CPUTwoInstPrinter.cpp - Convert MCInst to asm text ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "CPUTwoInstPrinter.h"
#include "CPUTwoMCTargetDesc.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

#define DEBUG_TYPE "asm-printer"

#include "CPUTwoGenAsmWriter.inc"

void CPUTwoInstPrinter::printRegName(raw_ostream &OS, MCRegister Reg) {
  OS << getRegisterName(Reg);
}

void CPUTwoInstPrinter::printInst(const MCInst *MI, uint64_t Address,
                                  StringRef Annot,
                                  const MCSubtargetInfo &STI,
                                  raw_ostream &O) {
  printInstruction(MI, Address, O);
  printAnnotation(O, Annot);
}

void CPUTwoInstPrinter::printOperand(const MCInst *MI, unsigned OpNo,
                                     raw_ostream &O) {
  const MCOperand &Op = MI->getOperand(OpNo);
  if (Op.isReg())
    printRegName(O, Op.getReg());
  else if (Op.isImm())
    O << Op.getImm();
  else if (Op.isExpr())
    MAI.printExpr(O, *Op.getExpr());
}

void CPUTwoInstPrinter::printCondCodeOperand(const MCInst *MI, unsigned OpNo,
                                              raw_ostream &O) {
  static const char *const CCNames[] = {
    "eq", "ne", "lt", "ge", "ltu", "geu", "al", "gt", "le", "gtu", "leu"
  };
  unsigned CC = MI->getOperand(OpNo).getImm();
  if (CC < 11)
    O << CCNames[CC];
  else
    O << CC;
}

void CPUTwoInstPrinter::printMemOperand(const MCInst *MI, unsigned OpNo,
                                        raw_ostream &O) {
  printRegName(O, MI->getOperand(OpNo).getReg());
  O << ", ";
  printOperand(MI, OpNo + 1, O);
}
