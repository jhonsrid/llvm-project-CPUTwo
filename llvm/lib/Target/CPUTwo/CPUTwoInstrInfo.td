//===-- CPUTwoInstrInfo.td - CPUTwo Instruction defs -------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

include "CPUTwoInstrFormats.td"

//===----------------------------------------------------------------------===//
// CPUTwo-specific SDNodes
//===----------------------------------------------------------------------===//

def SDT_CPUTwoCallSeqStart : SDCallSeqStart<[SDTCisVT<0, i32>,
                                             SDTCisVT<1, i32>]>;
def SDT_CPUTwoCallSeqEnd   : SDCallSeqEnd<[SDTCisVT<0, i32>,
                                            SDTCisVT<1, i32>]>;
def CallSeqStart : SDNode<"ISD::CALLSEQ_START", SDT_CPUTwoCallSeqStart,
                          [SDNPHasChain, SDNPOutGlue]>;
def CallSeqEnd   : SDNode<"ISD::CALLSEQ_END", SDT_CPUTwoCallSeqEnd,
                          [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;

def SDT_CPUTwoCall    : SDTypeProfile<0, -1, [SDTCisVT<0, iPTR>]>;
def SDT_CPUTwoRet     : SDTypeProfile<0, 0, []>;
def SDT_CPUTwoSetFlag : SDTypeProfile<0, 2, [SDTCisSameAs<0, 1>]>;
def SDT_CPUTwoBrCC    : SDTypeProfile<0, 2, [SDTCisVT<0, OtherVT>,
                                              SDTCisVT<1, i32>]>;
def SDT_CPUTwoSelectCC : SDTypeProfile<1, 3, [SDTCisSameAs<0, 1>,
                                               SDTCisSameAs<1, 2>,
                                               SDTCisVT<3, i32>]>;
def SDT_CPUTwoHi16    : SDTypeProfile<1, 1, [SDTCisInt<0>]>;
def SDT_CPUTwoLo16    : SDTypeProfile<1, 2, [SDTCisInt<0>]>;
def SDT_CPUTwoWrapper : SDTypeProfile<1, 1, [SDTCisSameAs<0, 1>,
                                              SDTCisPtrTy<0>]>;

def CPUTwo_ret       : SDNode<"CPUTwoISD::RET", SDT_CPUTwoRet,
                              [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;
def CPUTwo_call      : SDNode<"CPUTwoISD::CALL", SDT_CPUTwoCall,
                              [SDNPHasChain, SDNPOutGlue, SDNPOptInGlue,
                               SDNPVariadic]>;
def CPUTwo_cmp       : SDNode<"CPUTwoISD::CMP", SDT_CPUTwoSetFlag,
                              [SDNPOutGlue]>;
def CPUTwo_brcc      : SDNode<"CPUTwoISD::BRCOND", SDT_CPUTwoBrCC,
                              [SDNPHasChain, SDNPInGlue]>;
def CPUTwo_selectcc  : SDNode<"CPUTwoISD::SELECT_CC", SDT_CPUTwoSelectCC,
                              [SDNPInGlue]>;
def CPUTwo_hi16      : SDNode<"CPUTwoISD::HI16", SDT_CPUTwoHi16>;
def CPUTwo_lo16      : SDNode<"CPUTwoISD::LO16", SDT_CPUTwoLo16>;
def CPUTwo_wrapper   : SDNode<"CPUTwoISD::WRAPPER", SDT_CPUTwoWrapper>;

//===----------------------------------------------------------------------===//
// Operand definitions
//===----------------------------------------------------------------------===//

// 16-bit signed immediate
def simm16 : Operand<i32>, ImmLeaf<i32, [{return isInt<16>(Imm);}]> {
  let DecoderMethod = "decodeSImmOperand<16>";
}

// 16-bit unsigned immediate
def uimm16 : Operand<i32>, ImmLeaf<i32, [{return isUInt<16>(Imm);}]> {
  let DecoderMethod = "decodeUImmOperand<16>";
}

// 20-bit branch target (PC-relative)
def brtarget20 : Operand<OtherVT> {
  let EncoderMethod = "encodeBranchTarget";
  let DecoderMethod = "decodeBranchTarget";
}

// Memory operand: base + imm16 offset
def memsrc : Operand<i32> {
  let MIOperandInfo = (ops GPR, simm16);
  let PrintMethod = "printMemOperand";
  let EncoderMethod = "encodeMemOperand";
}

// 5-bit shift amount (for R-type shift field)
def shamt5 : Operand<i32>, ImmLeaf<i32, [{return isUInt<5>(Imm);}]> {
  let DecoderMethod = "decodeUImmOperand<5>";
}

// Condition code operand
def CCOp : Operand<i32> {
  let PrintMethod = "printCondCodeOperand";
}

//===----------------------------------------------------------------------===//
// Instruction format classes
//===----------------------------------------------------------------------===//

// R-type ALU instructions: rd = rs1 op rs2
class ALU_RR<bits<8> opcode, string asmstr, SDPatternOperator opnode>
    : CPUTwoInstR<opcode,
                  (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2),
                  asmstr # "\t$rd, $rs1, $rs2",
                  [(set GPR:$rd, (opnode GPR:$rs1, GPR:$rs2))]> {
  let shift = 0;
  let func = 0;
}

// I-type ALU instructions: rd = rs1 op imm16 (sign-extended)
class ALU_RI<bits<8> opcode, string asmstr, SDPatternOperator opnode>
    : CPUTwoInstI<opcode,
                  (outs GPR:$rd), (ins GPR:$rs1, simm16:$imm16),
                  asmstr # "\t$rd, $rs1, $imm16",
                  [(set GPR:$rd, (opnode GPR:$rs1, simm16:$imm16))]>;

// I-type ALU with unsigned immediate
class ALU_RIu<bits<8> opcode, string asmstr, SDPatternOperator opnode>
    : CPUTwoInstI<opcode,
                  (outs GPR:$rd), (ins GPR:$rs1, uimm16:$imm16),
                  asmstr # "\t$rd, $rs1, $imm16",
                  [(set GPR:$rd, (opnode GPR:$rs1, uimm16:$imm16))]>;

//===----------------------------------------------------------------------===//
// Arithmetic instructions
//===----------------------------------------------------------------------===//

def ADD  : ALU_RR<0x00, "add",  add>;
def SUB  : ALU_RR<0x01, "sub",  sub>;
def AND  : ALU_RR<0x02, "and",  and>;
def OR   : ALU_RR<0x03, "or",   or>;
def XOR  : ALU_RR<0x04, "xor",  xor>;

def NOT : CPUTwoInstR<0x05,
  (outs GPR:$rd), (ins GPR:$rs1),
  "not\t$rd, $rs1",
  [(set GPR:$rd, (not GPR:$rs1))]> {
  let rs2 = 0;
  let shift = 0;
  let func = 0;
}

// Shifts with immediate in shift field (R-type)
def LSL : CPUTwoInstR<0x06,
  (outs GPR:$rd), (ins GPR:$rs1, shamt5:$shift),
  "lsl\t$rd, $rs1, $shift",
  [(set GPR:$rd, (shl GPR:$rs1, shamt5:$shift))]> {
  let rs2 = 0;
  let func = 0;
}

def LSR : CPUTwoInstR<0x07,
  (outs GPR:$rd), (ins GPR:$rs1, shamt5:$shift),
  "lsr\t$rd, $rs1, $shift",
  [(set GPR:$rd, (srl GPR:$rs1, shamt5:$shift))]> {
  let rs2 = 0;
  let func = 0;
}

def ASR : CPUTwoInstR<0x08,
  (outs GPR:$rd), (ins GPR:$rs1, shamt5:$shift),
  "asr\t$rd, $rs1, $shift",
  [(set GPR:$rd, (sra GPR:$rs1, shamt5:$shift))]> {
  let rs2 = 0;
  let func = 0;
}

def MUL  : ALU_RR<0x09, "mul",  mul>;
def DIV  : ALU_RR<0x0A, "div",  sdiv>;

def MULH : CPUTwoInstR<0x22,
  (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2),
  "mulh\t$rd, $rs1, $rs2",
  [(set GPR:$rd, (mulhs GPR:$rs1, GPR:$rs2))]> {
  let shift = 0;
  let func = 0;
}

def MULHU : CPUTwoInstR<0x23,
  (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2),
  "mulhu\t$rd, $rs1, $rs2",
  [(set GPR:$rd, (mulhu GPR:$rs1, GPR:$rs2))]> {
  let shift = 0;
  let func = 0;
}

def DIVU : CPUTwoInstR<0x24,
  (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2),
  "divu\t$rd, $rs1, $rs2",
  [(set GPR:$rd, (udiv GPR:$rs1, GPR:$rs2))]> {
  let shift = 0;
  let func = 0;
}

def MOD : CPUTwoInstR<0x25,
  (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2),
  "mod\t$rd, $rs1, $rs2",
  [(set GPR:$rd, (srem GPR:$rs1, GPR:$rs2))]> {
  let shift = 0;
  let func = 0;
}

def MODU : CPUTwoInstR<0x26,
  (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2),
  "modu\t$rd, $rs1, $rs2",
  [(set GPR:$rd, (urem GPR:$rs1, GPR:$rs2))]> {
  let shift = 0;
  let func = 0;
}

// MOV (no flag update)
def MOV : CPUTwoInstR<0x27,
  (outs GPR:$rd), (ins GPR:$rs1),
  "mov\t$rd, $rs1",
  []> {
  let rs2 = 0;
  let shift = 0;
  let func = 0;
}

// MOV with PC as destination/source (for asm: mov pc, lr etc.)
// Same encoding as MOV but allows PC in operands.
let isAsmParserOnly = 1 in
def MOV_all : CPUTwoInstR<0x27,
  (outs GPRall:$rd), (ins GPRall:$rs1),
  "mov\t$rd, $rs1",
  []> {
  let rs2 = 0;
  let shift = 0;
  let func = 0;
}

// ADDC, SUBC
def ADDC : CPUTwoInstR<0x2B,
  (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2),
  "addc\t$rd, $rs1, $rs2",
  []> {
  let shift = 0;
  let func = 0;
}

def SUBC : CPUTwoInstR<0x2C,
  (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2),
  "subc\t$rd, $rs1, $rs2",
  []> {
  let shift = 0;
  let func = 0;
}

// Register shift amount variants
def LSLR : CPUTwoInstR<0x2D,
  (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2),
  "lslr\t$rd, $rs1, $rs2",
  [(set GPR:$rd, (shl GPR:$rs1, GPR:$rs2))]> {
  let shift = 0;
  let func = 0;
}

def LSRR : CPUTwoInstR<0x2E,
  (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2),
  "lsrr\t$rd, $rs1, $rs2",
  [(set GPR:$rd, (srl GPR:$rs1, GPR:$rs2))]> {
  let shift = 0;
  let func = 0;
}

def ASRR : CPUTwoInstR<0x2F,
  (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2),
  "asrr\t$rd, $rs1, $rs2",
  [(set GPR:$rd, (sra GPR:$rs1, GPR:$rs2))]> {
  let shift = 0;
  let func = 0;
}

// Rotates
def ROLR : CPUTwoInstR<0x39,
  (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2),
  "rolr\t$rd, $rs1, $rs2",
  [(set GPR:$rd, (rotl GPR:$rs1, GPR:$rs2))]> {
  let shift = 0;
  let func = 0;
}

def RORR : CPUTwoInstR<0x3A,
  (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2),
  "rorr\t$rd, $rs1, $rs2",
  [(set GPR:$rd, (rotr GPR:$rs1, GPR:$rs2))]> {
  let shift = 0;
  let func = 0;
}

def ROLI : CPUTwoInstR<0x3B,
  (outs GPR:$rd), (ins GPR:$rs1, shamt5:$shift),
  "roli\t$rd, $rs1, $shift",
  []> {
  let rs2 = 0;
  let func = 0;
}

def RORI : CPUTwoInstR<0x3C,
  (outs GPR:$rd), (ins GPR:$rs1, shamt5:$shift),
  "rori\t$rd, $rs1, $shift",
  []> {
  let rs2 = 0;
  let func = 0;
}

//===----------------------------------------------------------------------===//
// I-type ALU instructions
//===----------------------------------------------------------------------===//

def ADDI : ALU_RI<0x14, "addi", add>;
def SUBI : ALU_RI<0x15, "subi", sub>;
def ANDI : ALU_RIu<0x16, "andi", and>;
def ORI  : ALU_RIu<0x17, "ori",  or>;
def XORI : ALU_RIu<0x18, "xori", xor>;

// I-type shifts
def LSLI : CPUTwoInstI<0x19,
  (outs GPR:$rd), (ins GPR:$rs1, uimm16:$imm16),
  "lsli\t$rd, $rs1, $imm16",
  [(set GPR:$rd, (shl GPR:$rs1, uimm16:$imm16))]>;

def LSRI : CPUTwoInstI<0x1A,
  (outs GPR:$rd), (ins GPR:$rs1, uimm16:$imm16),
  "lsri\t$rd, $rs1, $imm16",
  [(set GPR:$rd, (srl GPR:$rs1, uimm16:$imm16))]>;

def ASRI : CPUTwoInstI<0x1B,
  (outs GPR:$rd), (ins GPR:$rs1, uimm16:$imm16),
  "asri\t$rd, $rs1, $imm16",
  [(set GPR:$rd, (sra GPR:$rs1, uimm16:$imm16))]>;

//===----------------------------------------------------------------------===//
// Data movement instructions
//===----------------------------------------------------------------------===//

// MOVI rd, imm16 (zero-extended)
def MOVI : CPUTwoInstI<0x0F,
  (outs GPR:$rd), (ins uimm16:$imm16),
  "movi\t$rd, $imm16",
  []> {
  let rs1 = 0;
}

// MOVHI rd, imm16 (rd = (rd & 0xFFFF) | (imm16 << 16))
// This is read-modify-write, uses a constraint
def MOVHI : CPUTwoInstI<0x13,
  (outs GPR:$rd), (ins GPR:$rd_in, uimm16:$imm16),
  "movhi\t$rd, $imm16",
  []> {
  let rs1 = 0;
  let Constraints = "$rd = $rd_in";
}

// LUI rd, imm16 (rd = imm16 << 16, low bits zeroed)
def LUI : CPUTwoInstI<0x38,
  (outs GPR:$rd), (ins uimm16:$imm16),
  "lui\t$rd, $imm16",
  [(set GPR:$rd, (CPUTwo_hi16 uimm16:$imm16))]> {
  let rs1 = 0;
}

//===----------------------------------------------------------------------===//
// Comparison instructions
//===----------------------------------------------------------------------===//

let Defs = [SR] in {
// CMP sets flags only, rd must be 0
def CMP : CPUTwoInstR<0x28,
  (outs), (ins GPR:$rs1, GPR:$rs2),
  "cmp\t$rs1, $rs2",
  [(CPUTwo_cmp GPR:$rs1, GPR:$rs2)]> {
  let rd = 0;
  let shift = 0;
  let func = 0;
  let isCompare = 1;
}

// CMPI sets flags only (zero-extended imm16)
def CMPI : CPUTwoInstI<0x29,
  (outs), (ins GPR:$rs1, uimm16:$imm16),
  "cmpi\t$rs1, $imm16",
  [(CPUTwo_cmp GPR:$rs1, uimm16:$imm16)]> {
  let rd = 0;
  let isCompare = 1;
}
} // Defs = [SR]

//===----------------------------------------------------------------------===//
// Load instructions
//===----------------------------------------------------------------------===//

let mayLoad = 1 in {
def LW : CPUTwoInstM<0x0B,
  (outs GPR:$rd), (ins GPR:$rs1, simm16:$imm16),
  "lw\t$rd, $rs1, $imm16",
  [(set GPR:$rd, (load (add GPR:$rs1, simm16:$imm16)))]>;

def LH : CPUTwoInstM<0x1C,
  (outs GPR:$rd), (ins GPR:$rs1, simm16:$imm16),
  "lh\t$rd, $rs1, $imm16",
  [(set GPR:$rd, (sextloadi16 (add GPR:$rs1, simm16:$imm16)))]>;

def LHU : CPUTwoInstM<0x1D,
  (outs GPR:$rd), (ins GPR:$rs1, simm16:$imm16),
  "lhu\t$rd, $rs1, $imm16",
  [(set GPR:$rd, (zextloadi16 (add GPR:$rs1, simm16:$imm16)))]>;

def LB : CPUTwoInstM<0x1E,
  (outs GPR:$rd), (ins GPR:$rs1, simm16:$imm16),
  "lb\t$rd, $rs1, $imm16",
  [(set GPR:$rd, (sextloadi8 (add GPR:$rs1, simm16:$imm16)))]>;

def LBU : CPUTwoInstM<0x1F,
  (outs GPR:$rd), (ins GPR:$rs1, simm16:$imm16),
  "lbu\t$rd, $rs1, $imm16",
  [(set GPR:$rd, (zextloadi8 (add GPR:$rs1, simm16:$imm16)))]>;

// Indexed loads (R-type)
def LWX : CPUTwoInstR<0x30,
  (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2),
  "lwx\t$rd, $rs1, $rs2",
  [(set GPR:$rd, (load (add GPR:$rs1, GPR:$rs2)))]> {
  let shift = 0;
  let func = 0;
}

def LBX : CPUTwoInstR<0x31,
  (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2),
  "lbx\t$rd, $rs1, $rs2",
  [(set GPR:$rd, (sextloadi8 (add GPR:$rs1, GPR:$rs2)))]> {
  let shift = 0;
  let func = 0;
}

def LBUX : CPUTwoInstR<0x32,
  (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2),
  "lbux\t$rd, $rs1, $rs2",
  [(set GPR:$rd, (zextloadi8 (add GPR:$rs1, GPR:$rs2)))]> {
  let shift = 0;
  let func = 0;
}

def LHX : CPUTwoInstR<0x35,
  (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2),
  "lhx\t$rd, $rs1, $rs2",
  [(set GPR:$rd, (sextloadi16 (add GPR:$rs1, GPR:$rs2)))]> {
  let shift = 0;
  let func = 0;
}

def LHUX : CPUTwoInstR<0x36,
  (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2),
  "lhux\t$rd, $rs1, $rs2",
  [(set GPR:$rd, (zextloadi16 (add GPR:$rs1, GPR:$rs2)))]> {
  let shift = 0;
  let func = 0;
}
} // mayLoad

//===----------------------------------------------------------------------===//
// Store instructions
//===----------------------------------------------------------------------===//

let mayStore = 1 in {
def SW : CPUTwoInstM<0x0C,
  (outs), (ins GPR:$rd, GPR:$rs1, simm16:$imm16),
  "sw\t$rd, $rs1, $imm16",
  [(store GPR:$rd, (add GPR:$rs1, simm16:$imm16))]>;

def SH : CPUTwoInstM<0x20,
  (outs), (ins GPR:$rd, GPR:$rs1, simm16:$imm16),
  "sh\t$rd, $rs1, $imm16",
  [(truncstorei16 GPR:$rd, (add GPR:$rs1, simm16:$imm16))]>;

def SB : CPUTwoInstM<0x21,
  (outs), (ins GPR:$rd, GPR:$rs1, simm16:$imm16),
  "sb\t$rd, $rs1, $imm16",
  [(truncstorei8 GPR:$rd, (add GPR:$rs1, simm16:$imm16))]>;

// Indexed stores (R-type)
def SWX : CPUTwoInstR<0x33,
  (outs), (ins GPR:$rd, GPR:$rs1, GPR:$rs2),
  "swx\t$rd, $rs1, $rs2",
  [(store GPR:$rd, (add GPR:$rs1, GPR:$rs2))]> {
  let shift = 0;
  let func = 0;
}

def SBX : CPUTwoInstR<0x34,
  (outs), (ins GPR:$rd, GPR:$rs1, GPR:$rs2),
  "sbx\t$rd, $rs1, $rs2",
  [(truncstorei8 GPR:$rd, (add GPR:$rs1, GPR:$rs2))]> {
  let shift = 0;
  let func = 0;
}

def SHX : CPUTwoInstR<0x37,
  (outs), (ins GPR:$rd, GPR:$rs1, GPR:$rs2),
  "shx\t$rd, $rs1, $rs2",
  [(truncstorei16 GPR:$rd, (add GPR:$rs1, GPR:$rs2))]> {
  let shift = 0;
  let func = 0;
}
} // mayStore

//===----------------------------------------------------------------------===//
// Branch instructions (B-type)
//===----------------------------------------------------------------------===//

class CPUTwoBranch<bits<4> condcode, string asmstr>
    : CPUTwoInstB<0x0D, condcode,
                  (outs), (ins brtarget20:$offset),
                  asmstr # "\t$offset",
                  []>;

// Conditional branches read the status register
let isBranch = 1, isTerminator = 1, Uses = [SR] in {
def BEQ  : CPUTwoBranch<0,  "beq">;
def BNE  : CPUTwoBranch<1,  "bne">;
def BLT  : CPUTwoBranch<2,  "blt">;
def BGE  : CPUTwoBranch<3,  "bge">;
def BLTU : CPUTwoBranch<4,  "bltu">;
def BGEU : CPUTwoBranch<5,  "bgeu">;
def BGT  : CPUTwoBranch<7,  "bgt">;
def BLE  : CPUTwoBranch<8,  "ble">;
def BGTU : CPUTwoBranch<9,  "bgtu">;
def BLEU : CPUTwoBranch<10, "bleu">;
} // isBranch, isTerminator, Uses = [SR]

// Unconditional branch does NOT use SR
let isBranch = 1, isTerminator = 1, isBarrier = 1 in
def BA   : CPUTwoBranch<6,  "ba">;

// Unconditional branch pattern
def : Pat<(br bb:$offset), (BA brtarget20:$offset)>;

// Conditional branch pseudo — takes a condition code operand, selected
// by custom lowering and expanded by InstrInfo::insertBranch
let isBranch = 1, isTerminator = 1, isCodeGenOnly = 1, isPseudo = 1,
    Uses = [SR] in
def BRCC : CPUTwoPseudo<(outs), (ins brtarget20:$offset, CCOp:$cc),
                         "brcc\t$offset, $cc",
                         [(CPUTwo_brcc bb:$offset, imm:$cc)]>;

// SELECT_CC pseudo — expanded post-RA
let usesCustomInserter = 1, Uses = [SR] in
def SELECT_CC : CPUTwoPseudo<(outs GPR:$dst),
                              (ins GPR:$truev, GPR:$falsev, CCOp:$cc),
                              "# SELECT_CC",
                              [(set GPR:$dst,
                                (CPUTwo_selectcc GPR:$truev, GPR:$falsev,
                                                 imm:$cc))]>;

//===----------------------------------------------------------------------===//
// Jump instructions (J-type)
//===----------------------------------------------------------------------===//

let isCall = 1, Defs = [LR] in
def JMP : CPUTwoInstJ<0x0E,
  (outs), (ins brtarget20:$offset),
  "jmp\tlr, $offset",
  []> {
  let rd = 14; // LR
}

// CALLR - register indirect call (R-type)
let isCall = 1, Defs = [LR] in
def CALLR : CPUTwoInstR<0x2A,
  (outs), (ins GPR:$rs1),
  "callr\tlr, $rs1",
  []> {
  let rd = 14; // LR
  let rs2 = 0;
  let shift = 0;
  let func = 0;
}

// Call patterns are handled by custom ISelDAGToDAG::Select

//===----------------------------------------------------------------------===//
// System instructions
//===----------------------------------------------------------------------===//

let hasSideEffects = 1 in {
def SYSCALL : CPUTwoInstSys<0x10, "syscall">;
def SYSRET  : CPUTwoInstSys<0x11, "sysret">;
def HALT    : CPUTwoInstSys<0x12, "halt">;
def SFENCE  : CPUTwoInstSys<0x3E, "sfence">;
def KRET    : CPUTwoInstSys<0x3F, "kret">;
}

//===----------------------------------------------------------------------===//
// Atomic instructions
//===----------------------------------------------------------------------===//

def CAS : CPUTwoInstR<0x3D,
  (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2),
  "cas\t$rd, $rs1, $rs2",
  []> {
  let shift = 0;
  let func = 0;
  let mayLoad = 1;
  let mayStore = 1;
  let hasSideEffects = 1;
}

//===----------------------------------------------------------------------===//
// Pseudo-instructions
//===----------------------------------------------------------------------===//

let isReturn = 1, isTerminator = 1, isBarrier = 1 in
def RET : CPUTwoPseudo<(outs), (ins), "ret", [(CPUTwo_ret)]>;

// Indirect branch (for switch tables)
let isBranch = 1, isTerminator = 1, isBarrier = 1, isIndirectBranch = 1 in
def BRIND : CPUTwoPseudo<(outs), (ins GPR:$target), "br\t$target",
                          [(brind GPR:$target)]>;

// Call frame setup/destroy pseudo-instructions
let Defs = [SP], Uses = [SP], hasSideEffects = 1 in {
def ADJCALLSTACKDOWN : CPUTwoPseudo<(outs), (ins i32imm:$amt, i32imm:$amt2),
                                     "# ADJCALLSTACKDOWN $amt, $amt2",
                                     [(CallSeqStart timm:$amt, timm:$amt2)]>;
def ADJCALLSTACKUP : CPUTwoPseudo<(outs), (ins i32imm:$amt1, i32imm:$amt2),
                                   "# ADJCALLSTACKUP $amt1, $amt2",
                                   [(CallSeqEnd timm:$amt1, timm:$amt2)]>;
}

//===----------------------------------------------------------------------===//
// Additional patterns (must come after instruction definitions)
//===----------------------------------------------------------------------===//

// Pattern: ORI for lo16 part of address materialization
def : Pat<(CPUTwo_lo16 GPR:$hi, uimm16:$lo),
          (ORI GPR:$hi, uimm16:$lo)>;

// Pattern: CPUTwo_wrapper for global address
def : Pat<(CPUTwo_wrapper tglobaladdr:$addr), (LUI tglobaladdr:$addr)>;
def : Pat<(CPUTwo_wrapper tblockaddress:$addr), (LUI tblockaddress:$addr)>;
def : Pat<(CPUTwo_wrapper tconstpool:$addr), (LUI tconstpool:$addr)>;
def : Pat<(CPUTwo_wrapper tjumptable:$addr), (LUI tjumptable:$addr)>;

// Load/store with zero offset patterns
def : Pat<(load GPR:$rs1), (LW GPR:$rs1, 0)>;
def : Pat<(store GPR:$rd, GPR:$rs1), (SW GPR:$rd, GPR:$rs1, 0)>;
def : Pat<(sextloadi16 GPR:$rs1), (LH GPR:$rs1, 0)>;
def : Pat<(zextloadi16 GPR:$rs1), (LHU GPR:$rs1, 0)>;
def : Pat<(sextloadi8 GPR:$rs1), (LB GPR:$rs1, 0)>;
def : Pat<(zextloadi8 GPR:$rs1), (LBU GPR:$rs1, 0)>;
def : Pat<(truncstorei16 GPR:$rd, GPR:$rs1), (SH GPR:$rd, GPR:$rs1, 0)>;
def : Pat<(truncstorei8 GPR:$rd, GPR:$rs1), (SB GPR:$rd, GPR:$rs1, 0)>;

// extload -> zextload patterns
def : Pat<(extloadi8 (add GPR:$rs1, simm16:$off)),
          (LBU GPR:$rs1, simm16:$off)>;
def : Pat<(extloadi16 (add GPR:$rs1, simm16:$off)),
          (LHU GPR:$rs1, simm16:$off)>;
def : Pat<(extloadi8 GPR:$rs1), (LBU GPR:$rs1, 0)>;
def : Pat<(extloadi16 GPR:$rs1), (LHU GPR:$rs1, 0)>;

// Helper transforms for 32-bit immediate splitting
def Hi16Xform : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant((N->getZExtValue() >> 16) & 0xFFFF,
                                    SDLoc(N), MVT::i32);
}]>;

def Lo16Xform : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant(N->getZExtValue() & 0xFFFF,
                                    SDLoc(N), MVT::i32);
}]>;

// Constant materialization patterns
def : Pat<(i32 uimm16:$imm), (MOVI uimm16:$imm)>;

// 32-bit constant: LUI + ORI
def : Pat<(i32 imm:$imm),
          (ORI (LUI (Hi16Xform imm:$imm)), (Lo16Xform imm:$imm))>;
