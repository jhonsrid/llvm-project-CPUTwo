//===--- CPUTwo.cpp - Implement CPUTwo target feature support --------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "CPUTwo.h"
#include "clang/Basic/MacroBuilder.h"

using namespace clang;
using namespace clang::targets;

const char *const CPUTwoTargetInfo::GCCRegNames[] = {
    "r0",  "r1",  "r2",  "r3",  "r4",  "r5",  "r6",  "r7",
    "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15"};

ArrayRef<const char *> CPUTwoTargetInfo::getGCCRegNames() const {
  return llvm::ArrayRef(GCCRegNames);
}

void CPUTwoTargetInfo::getTargetDefines(const LangOptions &Opts,
                                         MacroBuilder &Builder) const {
  Builder.defineMacro("__cputwo__");
  Builder.defineMacro("__CPUTWO__");

  // NetBSD headers require __WCHAR_MIN__ and __WINT_MIN__
  Builder.defineMacro("__WCHAR_MIN__", "(-__WCHAR_MAX__-1)");
  Builder.defineMacro("__WINT_MIN__", "(-__WINT_MAX__-1)");

  // NetBSD signal.h needs __SIG_ATOMIC_TYPE__
  Builder.defineMacro("__SIG_ATOMIC_TYPE__", "int");
  Builder.defineMacro("__SIG_ATOMIC_MAX__", "2147483647");
}
