//===-- CPUTwoCondCode.h - CPUTwo Condition Codes ---------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CPUTWO_CPUTWOCONDCODE_H
#define LLVM_LIB_TARGET_CPUTWO_CPUTWOCONDCODE_H

#include "llvm/Support/ErrorHandling.h"

namespace llvm {
namespace CPUTwoCC {

// Condition codes matching the B-type cond field encoding
enum CondCode {
  CC_EQ  = 0,   // Equal / zero (Z=1)
  CC_NE  = 1,   // Not equal (Z=0)
  CC_LT  = 2,   // Less than signed (N!=V)
  CC_GE  = 3,   // Greater or equal signed (N=V)
  CC_LTU = 4,   // Less than unsigned (C=1)
  CC_GEU = 5,   // Greater or equal unsigned (C=0)
  CC_AL  = 6,   // Always
  CC_GT  = 7,   // Greater than signed (Z=0 && N=V)
  CC_LE  = 8,   // Less or equal signed (Z=1 || N!=V)
  CC_GTU = 9,   // Greater than unsigned (C=0 && Z=0)
  CC_LEU = 10,  // Less or equal unsigned (C=1 || Z=1)
};

inline CondCode getOppositeCondition(CondCode CC) {
  switch (CC) {
  case CC_EQ:  return CC_NE;
  case CC_NE:  return CC_EQ;
  case CC_LT:  return CC_GE;
  case CC_GE:  return CC_LT;
  case CC_LTU: return CC_GEU;
  case CC_GEU: return CC_LTU;
  case CC_GT:  return CC_LE;
  case CC_LE:  return CC_GT;
  case CC_GTU: return CC_LEU;
  case CC_LEU: return CC_GTU;
  case CC_AL:  llvm_unreachable("Cannot reverse always condition");
  }
  llvm_unreachable("Invalid condition code");
}

} // namespace CPUTwoCC
} // namespace llvm

#endif // LLVM_LIB_TARGET_CPUTWO_CPUTWOCONDCODE_H
