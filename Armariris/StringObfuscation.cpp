#define DEBUG_TYPE "objdiv"

#include <string>
#include <sstream>
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/CryptoUtils.h"
#include "Transforms/Obfuscation/StringObfuscation.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"

using namespace llvm;

STATISTIC(GlobalsEncoded,
          "Counts number of global variables encoded");

namespace llvm {

    struct encVar {
    public:
        GlobalVariable *var;
        uint8_t key;
    };

    class StringObfuscationPass : public llvm::ModulePass {
    public:
        static char ID; // pass identification
        bool is_flag = false;

        StringObfuscationPass() : ModulePass(ID) {}

        StringObfuscationPass(bool flag) : ModulePass(ID) {
            is_flag = flag;
        }

        virtual bool runOnModule(Module &M) {
            if (!is_flag) {
                return false;
            }
            std::vector<GlobalVariable *> toDelConstGlob;
            //std::vector<GlobalVariable*> encGlob;
            std::vector<encVar *> encGlob;
            for (Module::global_iterator gi = M.global_begin(), ge = M.global_end(); gi != ge; ++gi) {
                // Loop over all global variables
                GlobalVariable *gv = &(*gi);
                errs() << "name: " << gv->getName() << " section: " << gv->getSection() << " isConstant: "
                       << gv->isConstant() << " hasInitializer: " << gv->hasInitializer() << " getInitializer: "
                       << gv->getInitializer() << " type: " << gv->getType() << "\n";
                // @.str = private unnamed_addr constant [4 x i8] c"123\00", align 1
                std::string::size_type str_idx = gv->getName().str().find(".str.");
                std::string section(gv->getSection());

                // Let's encode the static ones
                if (gv->getName().str().substr(0, 4) == ".str" &&
                    gv->isConstant() &&
                    gv->hasInitializer() &&
                    //字符串
                    isa<ConstantDataSequential>(gv->getInitializer()) &&
                    section != "llvm.metadata" &&
                    section.find("__objc_methname") == std::string::npos
                    /*&&gv->getType()->getArrayElementType()->getArrayElementType()->isIntegerTy()*/) {
                    ++GlobalsEncoded;
                    //errs() << " is constant";

                    // Duplicate global variable
                    GlobalVariable *dynGV = new GlobalVariable(M,
                                                               gv->getType()->getElementType(),
                                                               !(gv->isConstant()),
                                                               gv->getLinkage(),
                                                               (Constant *) 0,
                                                               gv->getName(),
                                                               (GlobalVariable *) 0,
                                                               gv->getThreadLocalMode(),
                                                               gv->getType()->getAddressSpace());
                    // dynGV->copyAttributesFrom(gv);
                    dynGV->setInitializer(gv->getInitializer());

                    std::string tmp = gv->getName().str();
//                      errs()<<"GV tmp: "<<tmp<<"\n";

                    Constant *initializer = gv->getInitializer();

                    ConstantDataSequential *cdata = dyn_cast<ConstantDataSequential>(initializer);
                    if (cdata) {
                        const char *orig = cdata->getRawDataValues().data();
//                        errs() << "GV initializer: " << orig << "\n";

                        unsigned len = cdata->getNumElements() * cdata->getElementByteSize();

                        encVar *cur = new encVar();
                        cur->var = dynGV;
                        cur->key = llvm::cryptoutils->get_uint8_t();
                        // casting away const is undef. behavior in C++
                        // TODO a clean implementation would retrieve the data, generate a new constant
                        // set the correct type, and copy the data over.
                        //char *encr = new char[len];
                        //Constant *initnew = ConstantDataArray::getString(M.getContext(), encr, true);
                        // 对数据做处理，encr 是处理之后的数据
                        char *encr = const_cast<char *>(orig);
                        // Simple xor encoding
                        for (unsigned i = 0; i != len; ++i) {
                            encr[i] = orig[i] ^ cur->key;
                        }

                        // FIXME Second part of the unclean hack.
                        dynGV->setInitializer(initializer);

                        // Prepare to add decode function for this variable
                        encGlob.push_back(cur);
                    } else {
                        // just copying default initializer for now
                        dynGV->setInitializer(initializer);
                    }
                    // redirect references to new GV and remove old one
                    gv->replaceAllUsesWith(dynGV);
                    toDelConstGlob.push_back(gv);
                }
            }
            // actuallte delete marked globals
            for (unsigned i = 0, e = toDelConstGlob.size(); i != e; ++i) {
                toDelConstGlob[i]->eraseFromParent();
            }
            addDecodeFunction(&M, &encGlob);
            return true;
        }

    private:
        void addDecodeFunction(Module *mod, std::vector<encVar *> *gvars) {
            // Declare and add the function definition
            //errs()<<"Successful enter decode function"<<"\n";
            std::vector<Type *> FuncTy_args;
            FunctionType *FuncTy = FunctionType::get(
                    /*Result=*/Type::getVoidTy(mod->getContext()),  // returning void
                    /*Params=*/FuncTy_args,  // taking no args
                    /*isVarArg=*/false);
            uint64_t StringObfDecodeRandomName = cryptoutils->get_uint64_t();
            std::string random_str;
            std::stringstream random_stream;
            random_stream << StringObfDecodeRandomName;
            random_stream >> random_str;
            StringObfDecodeRandomName++;
#if LLVM_VERSION_MAJOR >= 9
            FunctionCallee c = mod->getOrInsertFunction(".datadiv_decode" + random_str, FuncTy);
            Function* fdecode = cast<Function>(c.getCallee());
            fdecode->setCallingConv(CallingConv::C);
#else
            Constant *c = mod->getOrInsertFunction(".datadiv_decode" + random_str, FuncTy);
            Function *fdecode = cast<Function>(c);
            fdecode->setCallingConv(CallingConv::C);
#endif
            // 新的代码加在 entry 这个 block 中
            BasicBlock *entry = BasicBlock::Create(mod->getContext(), "entry", fdecode);
            IRBuilder<> builder(mod->getContext());
            builder.SetInsertPoint(entry);
            //创建类型
//            IntegerType *integerType = Type::getInt128Ty(mod->getContext());
//            IntegerType *integerType = IntegerType::get(mod->getContext(),32);
//            FunctionType *functionType = FunctionType::get(IntegerType::get(mod->getContext(), 32), false);

            //创建value
//            ConstantExpr::getGetElementPtr()
//            Constant *constantDataArray  = ConstantDataArray::getString(mod->getContext(), StringRef(""), true);
//            ConstantDataArray *constantDataArray1 = cast<ConstantDataArray>(constantDataArray);
//            Function *function = Function::Create(functionType, GlobalValue::ExternalLinkage, Twine(""), mod);

            for (unsigned i = 0, e = gvars->size(); i != e; ++i) {
                GlobalVariable *gvar = (*gvars)[i]->var;
                uint8_t key = (*gvars)[i]->key;

                Constant *init = gvar->getInitializer();
                // ConstantDataSequential 代表字符串
                ConstantDataSequential *cdata = dyn_cast<ConstantDataSequential>(init);

                unsigned len = cdata->getNumElements() * cdata->getElementByteSize();
                --len;

                BasicBlock *preHeaderBB = builder.GetInsertBlock();
                BasicBlock *for_body = BasicBlock::Create(mod->getContext(), "for-body", fdecode);
                BasicBlock *for_end = BasicBlock::Create(mod->getContext(), "for-end", fdecode);
                // 从 entry 跳到 for_body
                builder.CreateBr(for_body);
                // 下面操作在 for_body 中
                builder.SetInsertPoint(for_body);
                // i = 2
                PHINode *variable = builder.CreatePHI(Type::getInt32Ty(mod->getContext()), 2, "i");
                Value *startValue = builder.getInt32(0);
                Value *endValue = builder.getInt32(len);
                variable->addIncoming(startValue, preHeaderBB);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

                //LoadInst *Load=builder.CreateLoad(gvar);
                //errs()<<"Load: "<<*(Load->getPointerOperand())<<"\n";
                Value *indexList[2] = {ConstantInt::get(variable->getType(), 0), variable};
                Value *const_key = builder.getInt8(key);
                Value *GEP = builder.CreateGEP(gvar, ArrayRef<Value *>(indexList, 2), "arrayIdx");
                LoadInst *loadElement = builder.CreateLoad(GEP, false);
#if LLVM_VERSION_MAJOR >= 10
                loadElement->setAlignment(MaybeAlign(1));
#else
                loadElement->setAlignment(1);
#endif
                //errs()<<"Type: "<<*loadElement<<"\n";
                //CastInst* extended = new ZExtInst(const_key, loadElement->getType(), "extended", for_body);
                //Value* extended = builder.CreateZExtOrBitCast(const_key, loadElement->getType(),"extended");
                Value *Xor = builder.CreateXor(loadElement, const_key, "xor");
                StoreInst *Store = builder.CreateStore(Xor, GEP, false);
#if LLVM_VERSION_MAJOR >= 10
                Store->setAlignment(MaybeAlign(1));
#else
                Store->setAlignment(1);
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                Value *stepValue = builder.getInt32(1);
                Value *nextValue = builder.CreateAdd(variable, stepValue, "next-value");
                Value *endCondition = builder.CreateICmpULT(variable, endValue, "end-condition");
                endCondition = builder.CreateICmpNE(endCondition, builder.getInt1(0), "loop-condition");
                BasicBlock *loopEndBB = builder.GetInsertBlock();
                builder.CreateCondBr(endCondition, loopEndBB, for_end);
                builder.SetInsertPoint(for_end);
                variable->addIncoming(nextValue, loopEndBB);
            }
            builder.CreateRetVoid();
            appendToGlobalCtors(*mod, fdecode, 0);
        }
    };

}

char StringObfuscationPass::ID = 0;
static RegisterPass<StringObfuscationPass> X("GVDiv", "Global variable (i.e., const char*) diversification pass",
                                             false, true);

Pass *llvm::createStringObfuscation(bool flag) {
    return new StringObfuscationPass(flag);
}
