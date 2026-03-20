//===--- CPUTwo.h - Declare CPUTwo target feature support --------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_BASIC_TARGETS_CPUTWO_H
#define LLVM_CLANG_LIB_BASIC_TARGETS_CPUTWO_H

#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TargetOptions.h"
#include "llvm/Support/Compiler.h"
#include "llvm/TargetParser/Triple.h"

namespace clang {
namespace targets {

class LLVM_LIBRARY_VISIBILITY CPUTwoTargetInfo : public TargetInfo {
  static const char *const GCCRegNames[];

public:
  CPUTwoTargetInfo(const llvm::Triple &Triple, const TargetOptions &)
      : TargetInfo(Triple) {
    // Data layout: little-endian, 32-bit pointers, 32/64-bit aligned
    resetDataLayout("e-m:e-p:32:32-i32:32-i64:64-n32-S64");

    // CPUTwo uses 4 registers (r0-r3) for argument passing
    RegParmMax = 4;

    // Type sizes
    IntWidth = IntAlign = 32;
    LongWidth = LongAlign = 32;
    LongLongWidth = LongLongAlign = 64;
    PointerWidth = PointerAlign = 32;
    SizeType = UnsignedInt;
    PtrDiffType = SignedInt;
    IntPtrType = SignedInt;
    WCharType = SignedInt;
    WIntType = SignedInt;
  }

  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;

  bool isValidCPUName(StringRef Name) const override {
    return Name == "generic";
  }

  void fillValidCPUList(SmallVectorImpl<StringRef> &Values) const override {
    Values.emplace_back("generic");
  }

  bool setCPU(const std::string &Name) override {
    return Name == "generic";
  }

  bool hasFeature(StringRef Feature) const override {
    return Feature == "cputwo";
  }

  ArrayRef<const char *> getGCCRegNames() const override;

  ArrayRef<TargetInfo::GCCRegAlias> getGCCRegAliases() const override {
    return {};
  }

  BuiltinVaListKind getBuiltinVaListKind() const override {
    return TargetInfo::VoidPtrBuiltinVaList;
  }

  llvm::SmallVector<Builtin::InfosShard> getTargetBuiltins() const override {
    return {};
  }

  bool validateAsmConstraint(const char *&Name,
                             TargetInfo::ConstraintInfo &info) const override {
    switch (*Name) {
    default:
      return false;
    case 'r': // general register
      info.setAllowsRegister();
      return true;
    case 'n': // immediate integer
    case 'i': // immediate
      return true;
    }
  }

  std::string_view getClobbers() const override { return ""; }

  bool hasBitIntType() const override { return true; }
};

} // namespace targets
} // namespace clang

#endif // LLVM_CLANG_LIB_BASIC_TARGETS_CPUTWO_H
