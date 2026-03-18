//===-- CPUTwoTargetInfo.cpp - CPUTwo Target Implementation ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "TargetInfo/CPUTwoTargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Compiler.h"

using namespace llvm;

Target &llvm::getTheCPUTwoTarget() {
  static Target TheCPUTwoTarget;
  return TheCPUTwoTarget;
}

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void
LLVMInitializeCPUTwoTargetInfo() {
  RegisterTarget<Triple::cputwo> X(getTheCPUTwoTarget(), "cputwo", "CPUTwo",
                                   "CPUTwo");
}
