//===-- CPUTwo.td - Describe the CPUTwo Target Machine -----*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

include "llvm/Target/Target.td"

//===----------------------------------------------------------------------===//
// Register definitions
//===----------------------------------------------------------------------===//

include "CPUTwoRegisterInfo.td"

//===----------------------------------------------------------------------===//
// Calling convention
//===----------------------------------------------------------------------===//

include "CPUTwoCallingConv.td"

//===----------------------------------------------------------------------===//
// Instruction definitions
//===----------------------------------------------------------------------===//

include "CPUTwoInstrInfo.td"

//===----------------------------------------------------------------------===//
// CPUTwo processors supported
//===----------------------------------------------------------------------===//

def : ProcessorModel<"generic", NoSchedModel, []>;

//===----------------------------------------------------------------------===//
// Target declaration
//===----------------------------------------------------------------------===//

def CPUTwoInstrInfo : InstrInfo;

def CPUTwoAsmParser : AsmParser {
  let ShouldEmitMatchRegisterAltName = 1;
}

def CPUTwoAsmWriter : AsmWriter;

defm : RemapAllTargetPseudoPointerOperands<GPR>;

def CPUTwo : Target {
  let InstructionSet = CPUTwoInstrInfo;
  let AssemblyParsers = [CPUTwoAsmParser];
  let AssemblyWriters = [CPUTwoAsmWriter];
}
