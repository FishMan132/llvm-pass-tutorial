#include <llvm/IR/IRBuilder.h>
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/LegacyPassManager.h"

#include "llvm/Support/raw_ostream.h"


using namespace llvm;

void makeLLVMModule2() {

    LLVMContext Context;

    // Create some module to put our function into it.
    std::unique_ptr<Module> Owner = make_unique<Module>("tut2", Context);
    Module *mod = Owner.get();

    std::vector<Type *> FuncTy_args;
    FuncTy_args.push_back(Type::getInt32Ty(mod->getContext()));
    FuncTy_args.push_back(Type::getInt32Ty(mod->getContext()));

    FunctionType *FuncTy = FunctionType::get(
            /*Result=*/Type::getInt32Ty(mod->getContext()),  // returning void
            /*Params=*/FuncTy_args,  // taking no args
            /*isVarArg=*/false);

    Constant *c = mod->getOrInsertFunction("gcd", FuncTy);
    Function *gcd = cast<Function>(c);

    Function::arg_iterator args = gcd->arg_begin();
    Value *x = args++;
    x->setName("x");
    Value *y = args++;
    y->setName("y");

    BasicBlock *entry = BasicBlock::Create(mod->getContext(), "entry", gcd);
    BasicBlock *ret = BasicBlock::Create(mod->getContext(), "return", gcd);
    BasicBlock *cond_false = BasicBlock::Create(mod->getContext(), "cond_false", gcd);
    BasicBlock *cond_true = BasicBlock::Create(mod->getContext(), "cond_true", gcd);
    BasicBlock *cond_false_2 = BasicBlock::Create(mod->getContext(), "cond_false", gcd);

    IRBuilder<> builder(entry);
    Value *xEqualsY = builder.CreateICmpEQ(x, y, "tmp");
    builder.CreateCondBr(xEqualsY, ret, cond_false);

    builder.SetInsertPoint(ret);
    builder.CreateRet(x);

    builder.SetInsertPoint(cond_false);
    Value *xLessThanY = builder.CreateICmpULT(x, y, "tmp");
    builder.CreateCondBr(xLessThanY, cond_true, cond_false_2);

    builder.SetInsertPoint(cond_true);
    Value *yMinusX = builder.CreateSub(y, x, "tmp");
    std::vector<Value *> args1;
    args1.push_back(x);
    args1.push_back(yMinusX);
    Value *recur_1 = builder.CreateCall(gcd, args1, "tmp");
    builder.CreateRet(recur_1);

    builder.SetInsertPoint(cond_false_2);
    Value *xMinusY = builder.CreateSub(x, y, "tmp");
    std::vector<Value *> args2;
    args2.push_back(xMinusY);
    args2.push_back(y);
    Value *recur_2 = builder.CreateCall(gcd, args2, "tmp");
    builder.CreateRet(recur_2);
    outs() << "We just constructed this LLVM module:\n\n" << *mod;
    outs() << "\n\nRunning foo: ";
    outs().flush();
}

void makeLLVMModule() {

    LLVMContext Context;

    // Create some module to put our function into it.
    std::unique_ptr<Module> Owner = make_unique<Module>("test", Context);
    Module *mod = Owner.get();

    std::vector<Type *> FuncTy_args;
    FuncTy_args.push_back(Type::getInt32Ty(mod->getContext()));
    FuncTy_args.push_back(Type::getInt32Ty(mod->getContext()));
    FuncTy_args.push_back(Type::getInt32Ty(mod->getContext()));

    FunctionType *FuncTy = FunctionType::get(
            /*Result=*/Type::getInt32Ty(mod->getContext()),  // returning void
            /*Params=*/FuncTy_args,  // taking no args
            /*isVarArg=*/false);

    Constant *c = mod->getOrInsertFunction("mul_add", FuncTy);

    Function *mul_add = cast<Function>(c);
    mul_add->setCallingConv(CallingConv::C);

    Function::arg_iterator args = mul_add->arg_begin();
    Value *x = args++;
    x->setName("x");
    Value *y = args++;
    y->setName("y");
    Value *z = args++;
    z->setName("z");

    BasicBlock *block = BasicBlock::Create(mod->getContext(), "entry", mul_add);
    IRBuilder<> builder(block);

    Value *tmp = builder.CreateBinOp(Instruction::Mul, x, y, "tmp");
    Value *tmp2 = builder.CreateBinOp(Instruction::Add, tmp, z, "tmp2");

    builder.CreateRet(tmp2);

    outs() << "We just constructed this LLVM module:\n\n" << *mod;
    outs() << "\n\nRunning foo: ";
    outs().flush();
}

int main(int argc, char **argv) {
//    Module *Mod = makeLLVMModule();
//    Module *Mod2 = makeLLVMModule2();

//    outs() << "We just constructed this LLVM module:\n\n" << *Mod;
//    outs() << "We just constructed this LLVM module:\n\n" << *Mod2;
//    outs().flush();

//    delete Mod;
    makeLLVMModule();
    makeLLVMModule2();
    return 0;
}