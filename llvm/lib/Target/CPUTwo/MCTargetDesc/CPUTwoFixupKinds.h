//===-- CPUTwoFixupKinds.h - CPUTwo Fixup Entries ---------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CPUTWO_MCTARGETDESC_CPUTWOFIXUPKINDS_H
#define LLVM_LIB_TARGET_CPUTWO_MCTARGETDESC_CPUTWOFIXUPKINDS_H

#include "llvm/MC/MCFixup.h"

namespace llvm {
namespace CPUTwo {

enum Fixups {
  // 20-bit PC-relative branch target (B-type and J-type)
  fixup_cputwo_pc20 = FirstTargetFixupKind,
  // Upper 16 bits for LUI
  fixup_cputwo_hi16,
  // Lower 16 bits for ORI
  fixup_cputwo_lo16,

  fixup_cputwo_invalid,
  NumTargetFixupKinds = fixup_cputwo_invalid - FirstTargetFixupKind,
};

} // namespace CPUTwo
} // namespace llvm

#endif // LLVM_LIB_TARGET_CPUTWO_MCTARGETDESC_CPUTWOFIXUPKINDS_H
