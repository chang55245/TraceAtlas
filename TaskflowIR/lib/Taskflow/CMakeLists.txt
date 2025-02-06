add_mlir_dialect_library(MLIRTaskflow
    TaskflowDialect.cpp
    TaskflowOps.cpp

    DEPENDS
    MLIRTaskflowOpsIncGen

    LINK_LIBS PUBLIC
    MLIRIR
    MLIRInferTypeOpInterface
) 