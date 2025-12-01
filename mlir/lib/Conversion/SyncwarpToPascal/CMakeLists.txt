add_mlir_conversion_library(MLIRSyncwarpToPascal
  SyncwarpToPascal.cpp

  ADDITIONAL_HEADER_DIRS
  ${MLIR_MAIN_INCLUDE_DIR}/mlir/Conversion

  DEPENDS
  MLIRConversionPassIncGen

  LINK_LIBS PUBLIC
  MLIRIR
  MLIRLLVMDialect
  MLIRNVVMDialect
  MLIRPass
  MLIRSupport
  MLIRTransforms
)
