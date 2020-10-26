#ifndef _OBFUSCATION_H_
#define _OBFUSCATION_H_

#include "Transforms/Obfuscation/AntiClassDump.h"
#include "Transforms/Obfuscation/Flattening.h"
#include "Transforms/Obfuscation/StringEncryption.h"
#include "Transforms/Obfuscation/FunctionCallObfuscate.h"
#include "Transforms/Obfuscation/Substitution.h"
#include "Transforms/Obfuscation/BogusControlFlow.h"
#include "Transforms/Obfuscation/Split.h"
#include "Transforms/Obfuscation/IndirectBranch.h"
#include "Transforms/Obfuscation/FunctionWrapper.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/NoFolder.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InlineAsm.h"
#include "Transforms/Obfuscation/CryptoUtils.h"
#include "llvm/Transforms/Scalar.h"

#if __has_include("llvm/Transforms/Utils.h")
#include "llvm/Transforms/Utils.h"
#endif
using namespace std;
using namespace llvm;

// Namespace
namespace llvm {
    ModulePass *createObfuscationPass();

    void initializeObfuscationPass(PassRegistry &Registry);
}

#endif
