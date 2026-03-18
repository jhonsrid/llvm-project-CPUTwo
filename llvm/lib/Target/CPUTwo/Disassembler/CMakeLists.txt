add_llvm_component_library(LLVMCPUTwoDisassembler
  CPUTwoDisassembler.cpp

  LINK_COMPONENTS
  CodeGenTypes
  CPUTwoDesc
  CPUTwoInfo
  MC
  MCDisassembler
  Support

  ADD_TO_COMPONENT
  CPUTwo
)

add_dependencies(LLVMCPUTwoDisassembler CPUTwoCommonTableGen)
