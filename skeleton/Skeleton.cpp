#include <llvm/IR/Instructions.h>
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

using namespace llvm;

namespace {
    struct SkeletonPass : public FunctionPass {
        static char ID;

        SkeletonPass() : FunctionPass(ID) {}

        virtual bool runOnFunction(Function &F) {
//            errs() << "I saw a function called " << F.getName() << "!\n";

            Function::iterator tmp = F.begin();  //++tmp;
            BasicBlock *insert = &*tmp;

//            if (isa<BranchInst>(insert->getTerminator())) {
            Instruction *ter = &*(insert->begin());
            AllocaInst *switchVar = new AllocaInst(Type::getInt32Ty(F.getContext()), 0, "switchVar", ter);
            new StoreInst(ConstantInt::get(Type::getInt32Ty(F.getContext()), 12), switchVar,ter);
            LoadInst *load = new LoadInst(switchVar, "switchVar", ter);

//                LoadInst *loadInst= new LoadInst(switchVar, "switchVar", ter);

//            BasicBlock *swDefault = BasicBlock::Create(F.getContext(), "switchDefault", &F, insert);
//            BranchInst::Create(insert, swDefault);

            SwitchInst *switchI = SwitchInst::Create(&*(--F.end()), insert, 0, ter);
            switchI->setCondition(load);

            ConstantInt *numCase = cast<ConstantInt>(ConstantInt::get(switchI->getCondition()->getType(),1));
            switchI->addCase(numCase, insert);


//            }

//            BasicBlock *loopEntry = BasicBlock::Create(F.getContext(), "loopEntry", &F, insert);
//            LoadInst *load = new LoadInst(switchVar, "switchVar", loopEntry);

//            insert->moveBefore(loopEntry);
//            BranchInst::Create(loopEntry, insert);


//            if (isa<SwitchInst>(insert->getTerminator())) {
//                SwitchInst *br = cast<SwitchInst>(insert->getTerminator());
//                outs() << "br: " << *br << " Conditional: " << *(br->getCondition()) << "\n";
//                BasicBlock::iterator end = insert->end();
//                outs() << "end: " << *(--end) << "\n";
//
//                BasicBlock *tmpBB = insert->splitBasicBlock(--end, "first");
//                outs() << "splitBasicBlock: " << *tmpBB << "\n";
//
//                insert->getTerminator()->eraseFromParent();

//                outs() << "insert: " << *insert << "\n";
//            }

//            if (isa<BranchInst>(insert->getTerminator())) {
//                BranchInst *br = cast<BranchInst>(insert->getTerminator());
//                outs() << "br: " << *br << " Conditional: " << *(br->getCondition()) << "\n";
//                BasicBlock::iterator end = insert->end();
//                outs() << "end: " << *(--end) << "\n";
//
//                BasicBlock *tmpBB = insert->splitBasicBlock(--end, "first");
//                outs() << "splitBasicBlock: " << *tmpBB << "\n";
//
//                insert->getTerminator()->eraseFromParent();

//                outs() << "insert: " << *insert << "\n";
//            }

//            // 创建一个 BasicBlock，将它插入 insert 块前面
////            BasicBlock *loopEntry = BasicBlock::Create(F.getContext(), "loopEntry", &F, insert);
//
////            insert->moveBefore(loopEntry);
//            errs() << "block name: " << insert->getName() << "\n";
////            errs() << "block name: " << loopEntry->getName() << "\n";
//            Instruction *ter = insert->getTerminator();
//            errs() << "block name: " << ter->getOpcodeName() << "\n";
//
//            AllocaInst *switchVar = new AllocaInst(Type::getInt32Ty(F.getContext()), 0, "switchVar", insert);
//            outs() << "switchVar: " << *switchVar << "\n";
//            StoreInst *storeInst = new StoreInst(ConstantInt::get(Type::getInt32Ty(F.getContext()), 2), switchVar, insert);
//            outs() << "storeInst: " << *storeInst << "\n";


//            outs() << "before split insert block: " << *insert << "\n";
//            BasicBlock::iterator end = insert->end();
//            --end;
//            BasicBlock *tmpBB = insert->splitBasicBlock(end, "first");
//            outs() << "after split insert block: " << *insert << "\n";
//            outs() << "splitBasicBlock: " << *tmpBB << "\n";

            for (Function::iterator i = F.begin(); i != F.end(); ++i) {
                BasicBlock *bb = &*i;
                outs() << *i << "\n";
//                for (BasicBlock::iterator j = bb->begin(), e = bb->end(); j != e; ++j)  //获取每个basic block中的instruction
//                {
//                    outs() << *j << "\n";
//                    Instruction *inst = &(*j);
//                }

//                BasicBlock *loopEntry = BasicBlock::Create(F.getContext(), "loopEntry", &F, insert);

//                bb->moveBefore(loopEntry);
                errs() << "block name: " << bb->getName() << " numSuccessors: "
                       << i->getTerminator()->getNumSuccessors() << "\n";
            }


            return false;
        }
    };
}

char SkeletonPass::ID = 0;

// Automatically enable the pass.
// http://adriansampson.net/blog/clangpass.html
static void registerSkeletonPass(const PassManagerBuilder &,
                                 legacy::PassManagerBase &PM) {
    PM.add(new SkeletonPass());
}

static RegisterStandardPasses
        RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible, registerSkeletonPass);
