add_llvm_component_library(LLVMCPUTwoDesc
  CPUTwoAsmBackend.cpp
  CPUTwoELFObjectWriter.cpp
  CPUTwoInstPrinter.cpp
  CPUTwoMCAsmInfo.cpp
  CPUTwoMCCodeEmitter.cpp
  CPUTwoMCTargetDesc.cpp

  LINK_COMPONENTS
  CPUTwoInfo
  MC
  MCDisassembler
  Support
  TargetParser

  ADD_TO_COMPONENT
  CPUTwo
)
