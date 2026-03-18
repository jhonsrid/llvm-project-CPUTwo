//===-- CPUTwoMCAsmInfo.cpp - CPUTwo asm properties -----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "CPUTwoMCAsmInfo.h"
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

  CommentString = ";";

  SupportsDebugInformation = true;

  MinInstAlignment = 4;
}
