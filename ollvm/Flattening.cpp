//===- Flattening.cpp - Flattening Obfuscation pass------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the flattening pass
//
//===----------------------------------------------------------------------===//

#include "Transforms/Obfuscation/Flattening.h"
#include "Transforms/Obfuscation/Utils.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/CryptoUtils.h"

#define DEBUG_TYPE "flattening"

using namespace llvm;

// Stats
STATISTIC(Flattened,
          "Functions flattened");

namespace {
    struct Flattening : public FunctionPass {
        static char ID;  // Pass identification, replacement for typeid
        bool flag;

        Flattening() : FunctionPass(ID) {}

        Flattening(bool flag) : FunctionPass(ID) { this->flag = flag; }

        bool runOnFunction(Function &F);

        bool flatten(Function *f);
    };
}

char Flattening::ID = 0;
static RegisterPass<Flattening> X("flattening", "Call graph flattening");

Pass *llvm::createFlattening(bool flag) { return new Flattening(flag); }

bool Flattening::runOnFunction(Function &F) {
    Function *tmp = &F;
    // Do we obfuscate
    // 1.1 函数是否存在atrribute((__annotate(("fla"))))这种标记
    // 1.2 编译命令行是否传入了 -mllvm -fla参数
    if (toObfuscate(flag, tmp, "fla")) {
        if (flatten(tmp)) {
            ++Flattened;
        }
    }

    return false;
}

bool Flattening::flatten(Function *f) {
    vector<BasicBlock *> origBB;
    BasicBlock *loopEntry;
    BasicBlock *loopEnd;
    LoadInst *load;
    SwitchInst *switchI;
    AllocaInst *switchVar;

    // SCRAMBLER
    char scrambling_key[16];
    llvm::cryptoutils->get_bytes(scrambling_key, 16);
    // END OF SCRAMBLER

    // Lower switch
#if LLVM_VERSION_MAJOR >= 9
    // >=9.0, LowerSwitchPass depends on LazyValueInfoWrapperPass, which cause AssertError.
    // So I move LowerSwitchPass into register function, just before FlatteningPass.
#else
    // 消除了当前函数中的switch方式组织的代码，抓换成if/else这种分支调用
    FunctionPass *lower = createLowerSwitchPass();
    lower->runOnFunction(*f);
#endif

    /**
Module类，Module可以理解为一个完整的编译单元。一般来说，这个编译单元就是一个源码文件，如一个后缀为cpp的源文件。
Function类，这个类顾名思义就是对应于一个函数单元。Function可以描述两种情况，分别是函数定义和函数声明。
BasicBlock类，这个类表示一个基本代码块，“基本代码块”就是一段没有控制流逻辑的基本流程，相当于程序流程图中的基本过程（矩形表示）。
Instruction类，指令类就是LLVM中定义的基本操作，比如加减乘除这种算数指令、函数调用指令、跳转指令、返回指令等等。
     */
    // Save all original BB
    for (Function::iterator i = f->begin(); i != f->end(); ++i) {
        BasicBlock *tmp = &*i;
        origBB.push_back(tmp);

        BasicBlock *bb = &*i;
        // https://llvm.org/docs/LangRef.html#terminator-instructions
        // The terminator instructions are: ‘ret’, ‘br’, ‘switch’, ‘indirectbr’, ‘invoke’, ‘callbr’ ‘resume’, ‘catchswitch’, ‘catchret’, ‘cleanupret’, and ‘unreachable’.
        // 解释invoke指令，在函数调用有异常处理时候使用，https://zhuanlan.zhihu.com/p/165890173
        // 因为invoke指令比较复杂，所以直接跳过？
        if (isa<InvokeInst>(bb->getTerminator())) {
            return false;
        }
    }

    // Nothing to flatten
    if (origBB.size() <= 1) {
        return false;
    }

    // Remove first BB
    origBB.erase(origBB.begin());

    // Get a pointer on the first BB
    Function::iterator tmp = f->begin();  //++tmp;
    BasicBlock *insert = &*tmp;

    // If main begin with an if
    BranchInst *br = NULL;
    // br ins
    if (isa<BranchInst>(insert->getTerminator())) {
        br = cast<BranchInst>(insert->getTerminator());
    }
    // https://stackoverflow.com/questions/41604907/finding-the-list-of-successors-of-an-instruction-in-llvm
    // insert->getTerminator()->getNumSuccessors() 跳转分支数
    if ((br != NULL && br->isConditional()) || insert->getTerminator()->getNumSuccessors() > 1) {
        BasicBlock::iterator i = insert->end();
        --i;// 最后一条指令

        if (insert->size() > 1) {
            --i;// 倒数第二条指令
        }

        //  %cmp = icmp sgt i32 %0, %1
        //  br i1 %cmp, label %if.then, label %if.end
        BasicBlock *tmpBB = insert->splitBasicBlock(i, "first");
        origBB.insert(origBB.begin(), tmpBB);
    }

    // Remove jump
    // 去掉这个 br label %first
    insert->getTerminator()->eraseFromParent();

    // Create switch variable and set as it
    // int switchVar; 放到 insert 块后面
    switchVar = new AllocaInst(Type::getInt32Ty(f->getContext()), 0, "switchVar", insert);
    // switchVar = 123; 放进 insert 块后面
    new StoreInst(
            ConstantInt::get(Type::getInt32Ty(f->getContext()), llvm::cryptoutils->scramble32(0, scrambling_key)),
            switchVar, insert);

    // Create main loop
    loopEntry = BasicBlock::Create(f->getContext(), "loopEntry", f, insert);
    loopEnd = BasicBlock::Create(f->getContext(), "loopEnd", f, insert);
    // int switchVar1 = switchVar; 放进 loopEntry 块中
    load = new LoadInst(switchVar, "switchVar", loopEntry);

    // Move first BB on top
    insert->moveBefore(loopEntry);
    // insert 到 loopEntry 的跳转
    BranchInst::Create(loopEntry, insert);

    // loopEnd jump to loopEntry
    BranchInst::Create(loopEntry, loopEnd);
    // switchDefault 基本块，与 loopEnd 链接
    BasicBlock *swDefault = BasicBlock::Create(f->getContext(), "switchDefault", f, loopEnd);
    BranchInst::Create(loopEnd, swDefault);

    // Create switch instruction itself and set condition
    // switch(switchVar)
    switchI = SwitchInst::Create(&*f->begin(), swDefault, 0, loopEntry);
    switchI->setCondition(load);

    // Remove branch jump from 1st BB and make a jump to the while
    f->begin()->getTerminator()->eraseFromParent();

    BranchInst::Create(loopEntry, &*f->begin());

    // Put all BB in the switch
    /**
     * case 1:
     *      block 1
     * case 2:
     *      block 2
     */
    for (vector<BasicBlock *>::iterator b = origBB.begin(); b != origBB.end(); ++b) {
        BasicBlock *i = *b;
        ConstantInt *numCase = NULL;

        // Move the BB inside the switch (only visual, no code logic)
        i->moveBefore(loopEnd);

        // Add case to switch
        numCase = cast<ConstantInt>(
                ConstantInt::get(switchI->getCondition()->getType(),
                                 llvm::cryptoutils->scramble32(switchI->getNumCases(), scrambling_key)));
        switchI->addCase(numCase, i);
    }

    // Recalculate switchVar
    for (vector<BasicBlock *>::iterator b = origBB.begin(); b != origBB.end(); ++b) {
        BasicBlock *i = *b;
        ConstantInt *numCase = NULL;

        // Ret BB
        // 例子: ret i32 0
        if (i->getTerminator()->getNumSuccessors() == 0) {
            continue;
        }

        // If it's a non-conditional jump
        // 例子: br label %10
        if (i->getTerminator()->getNumSuccessors() == 1) {
            // Get successor and delete terminator
            BasicBlock *succ = i->getTerminator()->getSuccessor(0);
            i->getTerminator()->eraseFromParent();
            // Get next case
            numCase = switchI->findCaseDest(succ);

            // If next case == default case (switchDefault)
            if (numCase == NULL) {
                numCase = cast<ConstantInt>(
                        ConstantInt::get(switchI->getCondition()->getType(),
                                         llvm::cryptoutils->scramble32(switchI->getNumCases() - 1, scrambling_key))
                );
            }

            // Update switchVar and jump to the end of loop
            new StoreInst(numCase, load->getPointerOperand(), i);
            BranchInst::Create(loopEnd, i);
            continue;
        }

        // If it's a conditional jump
        // 例子: br i1 %6, label %7, label %10
        if (i->getTerminator()->getNumSuccessors() == 2) {
            // Get next cases
            ConstantInt *numCaseTrue = switchI->findCaseDest(i->getTerminator()->getSuccessor(0));
            ConstantInt *numCaseFalse = switchI->findCaseDest(i->getTerminator()->getSuccessor(1));

            // Check if next case == default case (switchDefault)
            if (numCaseTrue == NULL) {
                numCaseTrue = cast<ConstantInt>(
                        ConstantInt::get(switchI->getCondition()->getType(),
                                         llvm::cryptoutils->scramble32(switchI->getNumCases() - 1, scrambling_key)));
            }

            if (numCaseFalse == NULL) {
                numCaseFalse = cast<ConstantInt>(
                        ConstantInt::get(switchI->getCondition()->getType(),
                                         llvm::cryptoutils->scramble32(switchI->getNumCases() - 1, scrambling_key)));
            }

            // Create a SelectInst
            BranchInst *br = cast<BranchInst>(i->getTerminator());
            SelectInst *sel =
                    SelectInst::Create(br->getCondition(), numCaseTrue, numCaseFalse, "", i->getTerminator());

            // Erase terminator
            i->getTerminator()->eraseFromParent();

            // Update switchVar and jump to the end of loop
            new StoreInst(sel, load->getPointerOperand(), i);
            BranchInst::Create(loopEnd, i);
            continue;
        }
    }

    fixStack(f);

    return true;
}
