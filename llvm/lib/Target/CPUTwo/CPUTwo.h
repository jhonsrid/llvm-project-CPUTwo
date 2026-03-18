//===-- CPUTwo.h - Top-level interface for CPUTwo ---------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CPUTWO_CPUTWO_H
#define LLVM_LIB_TARGET_CPUTWO_CPUTWO_H

#include "MCTargetDesc/CPUTwoMCTargetDesc.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
class CPUTwoTargetMachine;
class FunctionPass;

FunctionPass *createCPUTwoISelDag(CPUTwoTargetMachine &TM);

} // namespace llvm

#endif // LLVM_LIB_TARGET_CPUTWO_CPUTWO_H
