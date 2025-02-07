set(LLVM_TARGET_DEFINITIONS Passes.td)

mlir_tablegen(Passes.h.inc -gen-pass-decls -name Taskflow)
add_public_tablegen_target(MLIRTaskflowPassIncGen)
add_mlir_doc(Passes TaskflowPasses ./ -gen-pass-doc)