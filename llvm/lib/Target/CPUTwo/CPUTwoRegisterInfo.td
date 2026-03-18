//===-- CPUTwoRegisterInfo.td - CPUTwo Register defs -------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Declarations that describe the CPUTwo register file
//===----------------------------------------------------------------------===//

class CPUTwoReg<bits<4> Enc, string n, list<string> alt = []>
    : Register<n> {
  let HWEncoding{3-0} = Enc;
  let AltNames = alt;
  let Namespace = "CPUTwo";
}

// General purpose registers
def R0  : CPUTwoReg<0,  "r0">;
def R1  : CPUTwoReg<1,  "r1">;
def R2  : CPUTwoReg<2,  "r2">;
def R3  : CPUTwoReg<3,  "r3">;
def R4  : CPUTwoReg<4,  "r4">;
def R5  : CPUTwoReg<5,  "r5">;
def R6  : CPUTwoReg<6,  "r6">;
def R7  : CPUTwoReg<7,  "r7">;
def R8  : CPUTwoReg<8,  "r8">;
def R9  : CPUTwoReg<9,  "r9">;
def R10 : CPUTwoReg<10, "r10">;
def R11 : CPUTwoReg<11, "r11">;
def R12 : CPUTwoReg<12, "r12">;

// Special purpose registers
def SP  : CPUTwoReg<13, "sp",  ["r13"]>;
def LR  : CPUTwoReg<14, "lr",  ["r14"]>;
def PC  : CPUTwoReg<15, "pc",  ["r15"]>;

// Register classes
// GPR - all allocatable registers (excludes PC)
def GPR : RegisterClass<"CPUTwo", [i32], 32,
  (add R0, R1, R2, R3,                       // args / caller-saved
   R12,                                       // scratch / caller-saved
   R4, R5, R6, R7, R8, R9, R10, R11,         // callee-saved
   SP, LR)>;                                  // special

// GPRnoSP - general allocation, excludes SP and PC
def GPRnoSP : RegisterClass<"CPUTwo", [i32], 32,
  (add R0, R1, R2, R3,
   R12,
   R4, R5, R6, R7, R8, R9, R10, R11,
   LR)>;
