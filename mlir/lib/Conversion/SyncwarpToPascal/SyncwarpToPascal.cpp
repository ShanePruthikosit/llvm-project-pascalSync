//===- SyncwarpToPascal.cpp - Lower syncwarp for Pascal GPUs -------------===//

#include "mlir/Conversion/Passes.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Dialect/LLVMIR/NVVMDialect.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

namespace mlir {
#define GEN_PASS_DEF_CONVERTSYNCWARPTOPASCAL
#include "mlir/Conversion/Passes.h.inc"

namespace {

//===----------------------------------------------------------------------===//
// Helper to get/create barrier0 declaration
//===----------------------------------------------------------------------===//

static LLVM::LLVMFuncOp getOrCreateBarrier0(ModuleOp module, 
                                             PatternRewriter &rewriter) {
  StringRef funcName = "llvm.nvvm.barrier0";
  
  if (auto existingFunc = module.lookupSymbol<LLVM::LLVMFuncOp>(funcName))
    return existingFunc;
  
  OpBuilder::InsertionGuard guard(rewriter);
  rewriter.setInsertionPointToStart(module.getBody());
  
  auto voidType = LLVM::LLVMVoidType::get(rewriter.getContext());
  auto funcType = LLVM::LLVMFunctionType::get(voidType, {});
  
  auto funcOp = rewriter.create<LLVM::LLVMFuncOp>(
      module.getLoc(), funcName, funcType);
  funcOp.setLinkage(LLVM::Linkage::External);
  
  return funcOp;
}

//===----------------------------------------------------------------------===//
// Rewrite Patterns
//===----------------------------------------------------------------------===//

/// Convert LLVM syncwarp calls to syncthreads
struct LLVMSyncwarpCallPattern : public OpRewritePattern<LLVM::CallOp> {
  using OpRewritePattern<LLVM::CallOp>::OpRewritePattern;

  LogicalResult matchAndRewrite(LLVM::CallOp callOp,
                                PatternRewriter &rewriter) const override {
    auto callee = callOp.getCalleeAttr();
    if (!callee)
      return failure();

    StringRef calleeName = callee.getValue();
    
    // Match various forms of syncwarp intrinsics
    if (!calleeName.contains("syncwarp") && 
        !calleeName.contains("__syncwarp"))
      return failure();
      
    // Replace with syncthreads barrier
    auto module = callOp->getParentOfType<ModuleOp>();
    auto syncthreadsFn = getOrCreateBarrier0(module, rewriter);
    
    rewriter.replaceOpWithNewOp<LLVM::CallOp>(
        callOp, TypeRange{}, SymbolRefAttr::get(syncthreadsFn));
    
    return success();
  }
};

/// Convert LLVM intrinsic calls for bar.warp.sync
struct LLVMIntrinsicSyncwarpPattern : public OpRewritePattern<LLVM::CallIntrinsicOp> {
  using OpRewritePattern<LLVM::CallIntrinsicOp>::OpRewritePattern;

  LogicalResult matchAndRewrite(LLVM::CallIntrinsicOp intrinsicOp,
                                PatternRewriter &rewriter) const override {
    StringRef intrinsicName = intrinsicOp.getIntrinAttr().getValue();
    
    // Match bar.warp.sync intrinsic
    if (intrinsicName != "llvm.nvvm.bar.warp.sync")
      return failure();
    
    // Replace with barrier0 CALL (not intrinsic)
    auto module = intrinsicOp->getParentOfType<ModuleOp>();
    auto barrier0Fn = getOrCreateBarrier0(module, rewriter);
    
    rewriter.replaceOpWithNewOp<LLVM::CallOp>(
        intrinsicOp, 
        TypeRange{}, 
        SymbolRefAttr::get(barrier0Fn));
    
    return success();
  }
};

//===----------------------------------------------------------------------===//
// Pass Implementation
//===----------------------------------------------------------------------===//

struct ConvertSyncwarpToPascalPass
    : public impl::ConvertSyncwarpToPascalBase<ConvertSyncwarpToPascalPass> {
  
  void runOnOperation() override {
    RewritePatternSet patterns(&getContext());
    patterns.add<LLVMSyncwarpCallPattern>(&getContext());
    patterns.add<LLVMIntrinsicSyncwarpPattern>(&getContext());
    
    if (failed(applyPatternsGreedily(getOperation(), 
                                     std::move(patterns))))
      signalPassFailure();
  }
};

} // namespace

std::unique_ptr<Pass> createConvertSyncwarpToPascalPass() {
  return std::make_unique<ConvertSyncwarpToPascalPass>();
}

} // namespace mlir
