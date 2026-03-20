//===- CPUTwo.cpp ---------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// CPUTwo is a 32-bit little-endian RISC architecture with a 64MB address space.
// Fixed 32-bit instruction width with 5 encoding formats.
//
//===----------------------------------------------------------------------===//

#include "Symbols.h"
#include "Target.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/Support/Endian.h"

using namespace llvm;
using namespace llvm::object;
using namespace llvm::support::endian;
using namespace llvm::ELF;
using namespace lld;
using namespace lld::elf;

namespace {
class CPUTwo final : public TargetInfo {
public:
  CPUTwo(Ctx &);
  RelExpr getRelExpr(RelType type, const Symbol &s,
                     const uint8_t *loc) const override;
  void relocate(uint8_t *loc, const Relocation &rel,
                uint64_t val) const override;
  bool needsThunk(RelExpr expr, RelType type, const InputFile *file,
                  uint64_t branchAddr, const Symbol &s,
                  int64_t a) const override;
  uint32_t getThunkSectionSpacing() const override;
  bool inBranchRange(RelType type, uint64_t src, uint64_t dst) const override;
};
} // namespace

CPUTwo::CPUTwo(Ctx &ctx) : TargetInfo(ctx) {
  // NOP = ADD r0, r0, r0 (opcode 0x00, all fields 0)
  trapInstr = {0x00, 0x00, 0x00, 0x00};
  defaultMaxPageSize = 4096;
  defaultCommonPageSize = 4096;
  needsThunks = true;
}

RelExpr CPUTwo::getRelExpr(RelType type, const Symbol &s,
                           const uint8_t *loc) const {
  switch (type) {
  case R_CPUTWO_PC20:
    return R_PC;
  case R_CPUTWO_32:
  case R_CPUTWO_HI16:
  case R_CPUTWO_LO16:
    return R_ABS;
  default:
    return R_NONE;
  }
}

void CPUTwo::relocate(uint8_t *loc, const Relocation &rel,
                      uint64_t val) const {
  switch (rel.type) {
  case R_CPUTWO_32:
    write32le(loc, val);
    break;
  case R_CPUTWO_PC20: {
    // PC-relative 20-bit offset in bits [19:0] of the instruction
    checkInt(ctx, loc, (int64_t)val, 20, rel);
    uint32_t insn = read32le(loc);
    insn = (insn & 0xFFF00000) | (val & 0xFFFFF);
    write32le(loc, insn);
    break;
  }
  case R_CPUTWO_HI16: {
    // Upper 16 bits in imm16 field (bits [15:0] of instruction)
    uint32_t insn = read32le(loc);
    insn = (insn & 0xFFFF0000) | ((val >> 16) & 0xFFFF);
    write32le(loc, insn);
    break;
  }
  case R_CPUTWO_LO16: {
    // Lower 16 bits in imm16 field (bits [15:0] of instruction)
    uint32_t insn = read32le(loc);
    insn = (insn & 0xFFFF0000) | (val & 0xFFFF);
    write32le(loc, insn);
    break;
  }
  default:
    Err(ctx) << getErrorLoc(ctx, loc) << "unrecognized relocation " << rel.type;
  }
}

bool CPUTwo::needsThunk(RelExpr expr, RelType type, const InputFile *file,
                        uint64_t branchAddr, const Symbol &s,
                        int64_t a) const {
  if (type != R_CPUTWO_PC20)
    return false;
  if (s.isUndefined() && !s.isInPlt(ctx))
    return false;
  uint64_t dst = expr == R_PLT_PC ? s.getPltVA(ctx) : s.getVA(ctx, a);
  return !inBranchRange(type, branchAddr, dst);
}

uint32_t CPUTwo::getThunkSectionSpacing() const {
  // PC20 has range +/- 512KB. Leave margin for thunk sections.
  return (512 * 1024) - 0x4000;
}

bool CPUTwo::inBranchRange(RelType type, uint64_t src, uint64_t dst) const {
  if (type != R_CPUTWO_PC20)
    return true;
  // 20-bit signed offset: range is [-524288, 524287]
  if (dst > src)
    return dst - src <= 524284; // 524288 - 4
  return src - dst <= 524288;
}

void elf::setCPUTwoTargetInfo(Ctx &ctx) {
  ctx.target.reset(new CPUTwo(ctx));
}
