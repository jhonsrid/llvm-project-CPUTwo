add_llvm_component_library(LLVMCPUTwoInfo
  CPUTwoTargetInfo.cpp

  LINK_COMPONENTS
  MC
  Support

  ADD_TO_COMPONENT
  CPUTwo
  )
