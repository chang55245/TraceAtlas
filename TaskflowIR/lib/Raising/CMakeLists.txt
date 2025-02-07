add_mlir_dialect_library(MLIRTaskflowRaising
    TaskflowInsertion.cpp

    DEPENDS
    MLIRTaskflowOpsIncGen
    MLIRTaskflowPassIncGen

    LINK_LIBS PUBLIC
    MLIRIR
    MLIRPass
    MLIRTaskflow
    MLIRLLVMDialect
    MLIRTargetLLVMIRExport
) 