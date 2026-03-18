//===-- CPUTwoInstrFormats.td - CPUTwo Instruction Formats --*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// CPUTwo instruction format base class
//===----------------------------------------------------------------------===//

class CPUTwoInst<dag outs, dag ins, string asmstr, list<dag> pattern>
    : Instruction {
  field bits<32> Inst;
  field bits<32> SoftFail = 0;

  let Namespace = "CPUTwo";
  let Size = 4;

  dag OutOperandList = outs;
  dag InOperandList = ins;
  let AsmString = asmstr;
  let Pattern = pattern;
}

//===----------------------------------------------------------------------===//
// R-type: [ opcode 8 | rd 4 | rs1 4 | rs2 4 | shift 5 | func 7 ]
//===----------------------------------------------------------------------===//

class CPUTwoInstR<bits<8> opcode, dag outs, dag ins, string asmstr,
                  list<dag> pattern>
    : CPUTwoInst<outs, ins, asmstr, pattern> {
  bits<4> rd;
  bits<4> rs1;
  bits<4> rs2;
  bits<5> shift;
  bits<7> func;

  let Inst{31-24} = opcode;
  let Inst{23-20} = rd;
  let Inst{19-16} = rs1;
  let Inst{15-12} = rs2;
  let Inst{11-7}  = shift;
  let Inst{6-0}   = func;
}

//===----------------------------------------------------------------------===//
// I-type: [ opcode 8 | rd 4 | rs1 4 | imm16 16 ]
//===----------------------------------------------------------------------===//

class CPUTwoInstI<bits<8> opcode, dag outs, dag ins, string asmstr,
                  list<dag> pattern>
    : CPUTwoInst<outs, ins, asmstr, pattern> {
  bits<4> rd;
  bits<4> rs1;
  bits<16> imm16;

  let Inst{31-24} = opcode;
  let Inst{23-20} = rd;
  let Inst{19-16} = rs1;
  let Inst{15-0}  = imm16;
}

//===----------------------------------------------------------------------===//
// B-type: [ opcode 8 | cond 4 | offset20 20 ]
//===----------------------------------------------------------------------===//

class CPUTwoInstB<bits<8> opcode, bits<4> cond, dag outs, dag ins,
                  string asmstr, list<dag> pattern>
    : CPUTwoInst<outs, ins, asmstr, pattern> {
  bits<20> offset;

  let Inst{31-24} = opcode;
  let Inst{23-20} = cond;
  let Inst{19-0}  = offset;
}

//===----------------------------------------------------------------------===//
// M-type: [ opcode 8 | rd 4 | rs1 4 | imm16 16 ]
// Structurally identical to I-type, used for load/store
//===----------------------------------------------------------------------===//

class CPUTwoInstM<bits<8> opcode, dag outs, dag ins, string asmstr,
                  list<dag> pattern>
    : CPUTwoInst<outs, ins, asmstr, pattern> {
  bits<4> rd;
  bits<4> rs1;
  bits<16> imm16;

  let Inst{31-24} = opcode;
  let Inst{23-20} = rd;
  let Inst{19-16} = rs1;
  let Inst{15-0}  = imm16;
}

//===----------------------------------------------------------------------===//
// J-type: [ opcode 8 | rd 4 | offset20 20 ]
//===----------------------------------------------------------------------===//

class CPUTwoInstJ<bits<8> opcode, dag outs, dag ins, string asmstr,
                  list<dag> pattern>
    : CPUTwoInst<outs, ins, asmstr, pattern> {
  bits<4> rd;
  bits<20> offset;

  let Inst{31-24} = opcode;
  let Inst{23-20} = rd;
  let Inst{19-0}  = offset;
}

//===----------------------------------------------------------------------===//
// System instructions: [ opcode 8 | 0...0 24 ]
//===----------------------------------------------------------------------===//

class CPUTwoInstSys<bits<8> opcode, string asmstr>
    : CPUTwoInst<(outs), (ins), asmstr, []> {
  let Inst{31-24} = opcode;
  let Inst{23-0}  = 0;
}

//===----------------------------------------------------------------------===//
// Pseudo instructions (no encoding)
//===----------------------------------------------------------------------===//

class CPUTwoPseudo<dag outs, dag ins, string asmstr, list<dag> pattern>
    : CPUTwoInst<outs, ins, asmstr, pattern> {
  let isPseudo = 1;
  let isCodeGenOnly = 1;
}
