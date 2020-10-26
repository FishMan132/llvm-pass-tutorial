//
// Created by LeadroyaL on 2018/10/10.
//

#include "Transforms/Obfuscation/BogusControlFlow.h"
#include "Transforms/Obfuscation/Flattening.h"
#include "Transforms/Obfuscation/Split.h"
#include "Transforms/Obfuscation/Substitution.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

using namespace llvm;

static void registerOllvmPass(const PassManagerBuilder &,
                              legacy::PassManagerBase &PM) {
    PM.add(createBogus(true));
#if LLVM_VERSION_MAJOR >= 9
    PM.add(createLowerSwitchPass());
#endif
    PM.add(createFlattening(true));
    PM.add(createSplitBasicBlock(true));
    PM.add(createSubstitution(true));
}

static RegisterStandardPasses
        RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
                       registerOllvmPass);
