//===-- CPUTwoAsmParser.cpp - Parse CPUTwo assembly to MCInst --------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/CPUTwoMCTargetDesc.h"
#include "TargetInfo/CPUTwoTargetInfo.h"
#include "llvm/MC/MCParser/MCAsmParser.h"
#include "llvm/MC/MCParser/MCParsedAsmOperand.h"
#include "llvm/MC/MCParser/MCTargetAsmParser.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Compiler.h"

using namespace llvm;

namespace {

class CPUTwoAsmParser : public MCTargetAsmParser {
public:
  CPUTwoAsmParser(const MCSubtargetInfo &STI, MCAsmParser &Parser,
                  const MCInstrInfo &MII, const MCTargetOptions &Options)
      : MCTargetAsmParser(Options, STI, MII) {}

  bool parseRegister(MCRegister &Reg, SMLoc &StartLoc,
                     SMLoc &EndLoc) override {
    return true;
  }

  ParseStatus tryParseRegister(MCRegister &Reg, SMLoc &StartLoc,
                               SMLoc &EndLoc) override {
    return ParseStatus::NoMatch;
  }

  bool parseInstruction(ParseInstructionInfo &Info, StringRef Name,
                        SMLoc NameLoc, OperandVector &Operands) override {
    return true;
  }

  bool matchAndEmitInstruction(SMLoc IDLoc, unsigned &Opcode,
                               OperandVector &Operands, MCStreamer &Out,
                               uint64_t &ErrorInfo,
                               bool MatchingInlineAsm) override {
    return true;
  }

  void convertToMapAndConstraints(unsigned Kind,
                                  const OperandVector &Operands) override {}
};

} // end anonymous namespace

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void
LLVMInitializeCPUTwoAsmParser() {
  RegisterMCAsmParser<CPUTwoAsmParser> X(getTheCPUTwoTarget());
}
