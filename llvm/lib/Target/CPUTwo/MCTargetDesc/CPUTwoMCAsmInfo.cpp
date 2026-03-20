//===-- CPUTwoMCAsmInfo.cpp - CPUTwo asm properties -----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "CPUTwoMCAsmInfo.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/TargetParser/Triple.h"

using namespace llvm;

void CPUTwoMCAsmInfo::anchor() {}

CPUTwoMCAsmInfo::CPUTwoMCAsmInfo(const Triple & /*TheTriple*/,
                                 const MCTargetOptions &Options) {
  IsLittleEndian = true;
  InternalSymbolPrefix = ".L";
  WeakRefDirective = "\t.weak\t";
  ExceptionsType = ExceptionHandling::DwarfCFI;

  UsesELFSectionDirectiveForBSS = true;

  // Default CommentString is "#", SeparatorString is ";".
  // Do NOT set CommentString to ";" as that conflicts with the
  // statement separator used in macros like _ENTRY.

  SupportsDebugInformation = true;

  MinInstAlignment = 4;
}

void CPUTwoMCAsmInfo::printSpecifierExpr(raw_ostream &OS,
                                          const MCSpecifierExpr &Expr) const {
  switch (Expr.getSpecifier()) {
  default:
    printExpr(OS, *Expr.getSubExpr());
    return;
  case CPUTwo::S_HI16:
    OS << "%hi(";
    break;
  case CPUTwo::S_LO16:
    OS << "%lo(";
    break;
  }
  printExpr(OS, *Expr.getSubExpr());
  OS << ')';
}
