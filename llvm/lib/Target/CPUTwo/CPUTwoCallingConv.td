//===-- CPUTwoCallingConv.td - Calling Conventions ---------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// CPUTwo C Calling Convention
def CC_CPUTwo : CallingConv<[
  // Promote i8/i16 arguments to i32
  CCIfType<[i8, i16], CCPromoteToType<i32>>,

  // First 4 arguments in r0-r3
  CCIfType<[i32], CCAssignToReg<[R0, R1, R2, R3]>>,

  // Remaining arguments on stack, 4-byte aligned
  CCIfType<[i32], CCAssignToStack<4, 4>>
]>;

// Return value convention
def RetCC_CPUTwo : CallingConv<[
  CCIfType<[i8, i16], CCPromoteToType<i32>>,
  CCIfType<[i32], CCAssignToReg<[R0, R1]>>
]>;

// Callee-saved registers
def CSR_CPUTwo : CalleeSavedRegs<(add R4, R5, R6, R7, R8, R9, R10, R11, LR)>;
