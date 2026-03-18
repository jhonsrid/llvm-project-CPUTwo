//===-- CPUTwoMCInstLower.cpp - Convert MachineInstr to MCInst ------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "CPUTwoMCInstLower.h"
#include "MCTargetDesc/CPUTwoMCAsmInfo.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"

using namespace llvm;

MCOperand CPUTwoMCInstLower::LowerSymbolOperand(const MachineOperand &MO,
                                                  MCSymbol *Sym) const {
  const MCExpr *Expr = MCSymbolRefExpr::create(Sym, Ctx);

  if (!MO.isJTI() && MO.getOffset())
    Expr = MCBinaryExpr::createAdd(
        Expr, MCConstantExpr::create(MO.getOffset(), Ctx), Ctx);

  CPUTwo::Specifier Spec = CPUTwo::S_None;
  if (MO.getTargetFlags() == CPUTwo::S_HI16)
    Spec = CPUTwo::S_HI16;
  else if (MO.getTargetFlags() == CPUTwo::S_LO16)
    Spec = CPUTwo::S_LO16;

  if (Spec != CPUTwo::S_None)
    Expr = MCSpecifierExpr::create(Expr, Spec, Ctx);

  return MCOperand::createExpr(Expr);
}

void CPUTwoMCInstLower::Lower(const MachineInstr *MI, MCInst &OutMI) const {
  OutMI.setOpcode(MI->getOpcode());

  for (const MachineOperand &MO : MI->operands()) {
    MCOperand MCOp;

    switch (MO.getType()) {
    default:
      MI->print(errs());
      llvm_unreachable("unknown operand type");
    case MachineOperand::MO_Register:
      if (MO.isImplicit())
        continue;
      MCOp = MCOperand::createReg(MO.getReg());
      break;
    case MachineOperand::MO_Immediate:
      MCOp = MCOperand::createImm(MO.getImm());
      break;
    case MachineOperand::MO_MachineBasicBlock:
      MCOp = MCOperand::createExpr(
          MCSymbolRefExpr::create(MO.getMBB()->getSymbol(), Ctx));
      break;
    case MachineOperand::MO_RegisterMask:
      continue;
    case MachineOperand::MO_GlobalAddress:
      MCOp = LowerSymbolOperand(
          MO, Printer.getSymbol(MO.getGlobal()));
      break;
    case MachineOperand::MO_BlockAddress:
      MCOp = LowerSymbolOperand(
          MO, Printer.GetBlockAddressSymbol(MO.getBlockAddress()));
      break;
    case MachineOperand::MO_ExternalSymbol:
      MCOp = LowerSymbolOperand(
          MO, Printer.GetExternalSymbolSymbol(MO.getSymbolName()));
      break;
    case MachineOperand::MO_JumpTableIndex:
      MCOp = LowerSymbolOperand(MO, Printer.GetJTISymbol(MO.getIndex()));
      break;
    case MachineOperand::MO_ConstantPoolIndex:
      MCOp = LowerSymbolOperand(MO, Printer.GetCPISymbol(MO.getIndex()));
      break;
    }

    OutMI.addOperand(MCOp);
  }
}
