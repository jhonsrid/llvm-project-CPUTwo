//===- CPUTwo.cpp ---------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "ABIInfoImpl.h"
#include "TargetInfo.h"

using namespace clang;
using namespace clang::CodeGen;

//===----------------------------------------------------------------------===//
// CPUTwo ABI Implementation
//===----------------------------------------------------------------------===//

namespace {

class CPUTwoABIInfo : public DefaultABIInfo {
  struct CCState {
    unsigned FreeRegs;
  };

public:
  CPUTwoABIInfo(CodeGen::CodeGenTypes &CGT) : DefaultABIInfo(CGT) {}

  void computeInfo(CGFunctionInfo &FI) const override {
    CCState State;
    State.FreeRegs = 4; // r0-r3

    if (!getCXXABI().classifyReturnType(FI))
      FI.getReturnInfo() = classifyReturnType(FI.getReturnType());
    for (auto &I : FI.arguments())
      I.info = classifyArgumentType(I.type, State);
  }

  ABIArgInfo classifyArgumentType(QualType Ty, CCState &State) const {
    // Promote small integers
    if (isPromotableIntegerTypeForABI(Ty)) {
      if (State.FreeRegs > 0) {
        --State.FreeRegs;
        return ABIArgInfo::getDirectInReg();
      }
      return ABIArgInfo::getExtend(Ty);
    }

    unsigned Size = getContext().getTypeSize(Ty);
    unsigned SizeInRegs = (Size + 31) / 32;

    if (isAggregateTypeForABI(Ty)) {
      if (SizeInRegs <= State.FreeRegs) {
        State.FreeRegs -= SizeInRegs;
        return ABIArgInfo::getDirect();
      }
      State.FreeRegs = 0;
      return getNaturalAlignIndirect(Ty,
                                     getDataLayout().getAllocaAddrSpace(),
                                     /*ByVal=*/true);
    }

    if (SizeInRegs <= State.FreeRegs)
      State.FreeRegs -= SizeInRegs;
    else
      State.FreeRegs = 0;

    return ABIArgInfo::getDirect();
  }
};

class CPUTwoTargetCodeGenInfo : public TargetCodeGenInfo {
public:
  CPUTwoTargetCodeGenInfo(CodeGen::CodeGenTypes &CGT)
      : TargetCodeGenInfo(std::make_unique<CPUTwoABIInfo>(CGT)) {}
};

} // end anonymous namespace

std::unique_ptr<TargetCodeGenInfo>
CodeGen::createCPUTwoTargetCodeGenInfo(CodeGenModule &CGM) {
  return std::make_unique<CPUTwoTargetCodeGenInfo>(CGM.getTypes());
}
