#include <llvm/IR/Instructions.h>
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

using namespace llvm;

namespace {
    struct SkeletonModule : public ModulePass {
        static char ID;

        SkeletonModule() : ModulePass(ID) {}

        virtual bool runOnModule(Module &M) {

            outs() << "111" << &*(M.functions().begin()) << "\n";
//            outs() << M.getName().str() << "\n";
//            for (Module::global_iterator gi = M.global_begin(), ge = M.global_end(); gi != ge; ++gi) {
//                // Loop over all global variables
////                GlobalVariable *gv = &(*gi);
//                outs() << "module: " << 1 << "\n";
//            }
            return false;
        }
    };
}

char SkeletonModule::ID = 0;

// Automatically enable the pass.
// http://adriansampson.net/blog/clangpass.html
static void registerSkeletonPass(const PassManagerBuilder &,
                                 legacy::PassManagerBase &PM) {
    PM.add(new SkeletonModule());
}

static RegisterStandardPasses
        RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible, registerSkeletonPass);
