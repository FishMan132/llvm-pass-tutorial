//
// Created by LeadroyaL on 2018/10/10.
//

#include "Transforms/Obfuscation/Obfuscation.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

using namespace llvm;

static void registerHikariModulePass(const PassManagerBuilder &,
                                     legacy::PassManagerBase &PM) {
//    PM.add(createFunctionWrapperPass(true)); /*broken*/
    PM.add(createStringEncryptionPass(true));

}

static void registerHikariFunctionPass(const PassManagerBuilder &,
                                       legacy::PassManagerBase &PM) {
    PM.add(createBogusControlFlowPass(true));
#if LLVM_VERSION_MAJOR >= 9
    PM.add(createLowerSwitchPass());
#endif
    PM.add(createFlatteningPass(true));
    PM.add(createFunctionCallObfuscatePass(true));
    PM.add(createIndirectBranchPass(true));
    PM.add(createSplitBasicBlockPass(true));
    PM.add(createSubstitutionPass(true));
}

static RegisterStandardPasses
        RegisterMyPass(PassManagerBuilder::EP_EnabledOnOptLevel0,
                       registerHikariModulePass);
static RegisterStandardPasses
        RegisterMyPass0(PassManagerBuilder::EP_OptimizerLast,
                        registerHikariModulePass);
static RegisterStandardPasses
        RegisterMyPass1(PassManagerBuilder::EP_EarlyAsPossible,
                        registerHikariFunctionPass);