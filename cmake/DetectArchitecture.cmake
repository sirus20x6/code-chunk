# Detect the target architecture
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64)$")
    set(LLVM_TARGETS_TO_BUILD "X86")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|AARCH64)$")
    set(LLVM_TARGETS_TO_BUILD "AArch64")
else()
    message(FATAL_ERROR "Unsupported target architecture: ${CMAKE_SYSTEM_PROCESSOR}")
endif()