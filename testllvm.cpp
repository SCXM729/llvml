
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/StandardInstrumentations.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Scalar/Reassociate.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"
#include <memory>
using namespace llvm;
int main() {
  auto TheContext = std::make_unique<LLVMContext>();
  auto TheModule = std::make_unique<Module>("testllvm", *TheContext);
  auto TheBuilder = std::make_unique<IRBuilder<>>(*TheContext);

  // double(double)
  std::vector<Type *> Doubles(1, Type::getDoubleTy(*TheContext));
  FunctionType *FT =
      FunctionType::get(Type::getDoubleTy(*TheContext), Doubles, false);
  Function *F =
      Function::Create(FT, Function::ExternalLinkage, "test", TheModule.get());
  // signle argument
  auto Arg = F->getArg(0);
  // for (auto &Arg : F->args())
  //   Arg.setName("x");
  Arg->setName("x");

  BasicBlock *BB = BasicBlock::Create(*TheContext, "entry", F);
  TheBuilder->SetInsertPoint(BB);

  Value *x = ConstantFP::get(*TheContext, APFloat(3.0));
  auto addtmp = TheBuilder->CreateFAdd(Arg, x);
  auto addtmp1 = TheBuilder->CreateFAdd(Arg, x);
  auto multmp = TheBuilder->CreateFMul(addtmp, addtmp1);
  TheBuilder->CreateRet(multmp);

  // optimization
  auto TheFPM = std::make_unique<FunctionPassManager>();
  auto TheFAM = std::make_unique<FunctionAnalysisManager>();
  auto TheLAM = std::make_unique<LoopAnalysisManager>();
  auto TheCGAM = std::make_unique<CGSCCAnalysisManager>();
  auto TheMAM = std::make_unique<ModuleAnalysisManager>();

  auto ThePIC = std::make_unique<PassInstrumentationCallbacks>();
  auto TheSI = std::make_unique<StandardInstrumentations>(*TheContext, true);
  TheSI->registerCallbacks(*ThePIC, TheMAM.get());

  // TheFPM->addPass(ReassociatePass());
  // TheFPM->addPass(InstCombinePass());
  TheFPM->addPass(GVNPass());

  // register analysis
  PassBuilder PB;
  PB.registerModuleAnalyses(*TheMAM);
  PB.registerFunctionAnalyses(*TheFAM);
  PB.crossRegisterProxies(*TheLAM, *TheFAM, *TheCGAM, *TheMAM);

  TheFPM->run(*F, *TheFAM);

  TheModule->print(errs(), nullptr);
}
