add_llvm_component_library(LLVMCPUTwoAsmParser
  CPUTwoAsmParser.cpp

  LINK_COMPONENTS
  CodeGenTypes
  CPUTwoDesc
  CPUTwoInfo
  MC
  MCParser
  Support

  ADD_TO_COMPONENT
  CPUTwo
)

add_dependencies(LLVMCPUTwoAsmParser CPUTwoCommonTableGen)
