//
// Created by LeadroyaL on 2018/10/10.
//

#include "Transforms/Obfuscation/Flattening.h"
#include "Transforms/Obfuscation/StringObfuscation.h"
#include "Transforms/Obfuscation/Substitution.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

using namespace llvm;

static void registerArmaririsModulePass(const PassManagerBuilder &,
                                        legacy::PassManagerBase &PM) {
    PM.add(createStringObfuscation(true));
}

static void registerArmaririsFunctionPass(const PassManagerBuilder &,
                                          legacy::PassManagerBase &PM) {
#if LLVM_VERSION_MAJOR >= 9
    PM.add(createLowerSwitchPass());
#endif
    PM.add(createFlattening(true));
    PM.add(createSubstitution(true));
}

static RegisterStandardPasses
        RegisterMyPass(PassManagerBuilder::EP_EnabledOnOptLevel0,
                       registerArmaririsModulePass);
static RegisterStandardPasses
        RegisterMyPass0(PassManagerBuilder::EP_OptimizerLast,
                        registerArmaririsModulePass);
static RegisterStandardPasses
        RegisterMyPass1(PassManagerBuilder::EP_EarlyAsPossible,
                        registerArmaririsFunctionPass);