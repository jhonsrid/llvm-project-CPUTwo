add_llvm_component_group(CPUTwo)

set(LLVM_TARGET_DEFINITIONS CPUTwo.td)

tablegen(LLVM CPUTwoGenAsmMatcher.inc -gen-asm-matcher)
tablegen(LLVM CPUTwoGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM CPUTwoGenCallingConv.inc -gen-callingconv)
tablegen(LLVM CPUTwoGenDAGISel.inc -gen-dag-isel)
tablegen(LLVM CPUTwoGenDisassemblerTables.inc -gen-disassembler)
tablegen(LLVM CPUTwoGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM CPUTwoGenMCCodeEmitter.inc -gen-emitter)
tablegen(LLVM CPUTwoGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM CPUTwoGenSubtargetInfo.inc -gen-subtarget)

add_public_tablegen_target(CPUTwoCommonTableGen)

add_llvm_target(CPUTwoCodeGen
  CPUTwoAsmPrinter.cpp
  CPUTwoFrameLowering.cpp
  CPUTwoInstrInfo.cpp
  CPUTwoISelDAGToDAG.cpp
  CPUTwoISelLowering.cpp
  CPUTwoMCInstLower.cpp
  CPUTwoRegisterInfo.cpp
  CPUTwoSubtarget.cpp
  CPUTwoTargetMachine.cpp

  LINK_COMPONENTS
  Analysis
  AsmPrinter
  CodeGen
  CodeGenTypes
  Core
  CPUTwoDesc
  CPUTwoInfo
  MC
  SelectionDAG
  Support
  Target
  TargetParser
  TransformUtils

  ADD_TO_COMPONENT
  CPUTwo
  )

add_subdirectory(AsmParser)
add_subdirectory(Disassembler)
add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)
