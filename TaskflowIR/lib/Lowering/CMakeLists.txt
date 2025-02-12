add_mlir_dialect_library(MLIRTaskflowLowering
    TaskflowToLLVMLowering.cpp

    DEPENDS
    MLIRTaskflowOpsIncGen
    MLIRTaskflowPassIncGen

    LINK_LIBS PUBLIC
    MLIRIR
    MLIRPass
    MLIRTaskflow
    MLIRLLVMDialect
    MLIRTargetLLVMIRExport
    MLIRTransforms
) 