#include "DerivedInstrInfo.h"
#include "InstrInfo.h"
#include "Tracer.h"
#include "Debug.h"
#include "llvm-c/Core.h"
#include <map>
#include "Cache.h"

using namespace tracer;
using namespace llvm;


bool CallInstrInfo::instrument(TracerPass* tracer, bool isFirstBlock, bool isFirstInstr){
    basicInstrument(tracer, isFirstBlock, isFirstInstr);
    //ignore the externel call
    if(tracer->getTraceMode() == TracerPass::TraceMode::TRACE_FUNC_LEVEL || tracer->getTraceMode() == TracerPass::TraceMode::TRACE_LINE_LEVEL
            || tracer->getTraceMode() == TracerPass::TraceMode::TRACE_BB_LEVEL){
        llvm::IRBuilder<> IRB(this->instr);
        llvm::Value* funcStr = getGlobalPtr(this->instr->getFunction()->getName().str(), IRB);//IRB.CreateGlobalStringPtr(this->instr->getFunction()->getName());
        llvm::StringRef* strRef = new llvm::StringRef(this->getFileName());
        std::string tmpFileName = "";
        if(this->getFileName() != NULL){
            tmpFileName = std::string(this->getFileName());
        }
        llvm::Value* fileStr = getGlobalPtr(tmpFileName, IRB);//IRB.CreateGlobalStringPtr(*strRef);
        llvm::CallInst* callInst = dyn_cast<llvm::CallInst>(this->instr);
        llvm::Function* func = callInst->getCalledFunction();
        //it seems that when func ptr is used, the getCalledFunction would return NULL
        if(func != NULL && func->getName() == "startLog"){
            return false;
        }
        if(func != NULL && func->isIntrinsic()){
            return false;
        }
        /*FIXME@DJR is external function == (getInstrunctionCount= 0)*/
		std::string basicBlockID = "";
        std::string Str;
        llvm::raw_string_ostream OS(Str);
        llvm::BasicBlock* tmpBlock = this->instr->getParent();
        tmpBlock->printAsOperand(OS, false);
        basicBlockID = OS.str();
        llvm::StringRef* bbRef = new llvm::StringRef(basicBlockID.c_str());
        Value* bbVal = getGlobalPtr(basicBlockID, IRB);
        if(func == NULL || (func != NULL && func->getInstructionCount() != 0)){
            llvm::Value* valueList[4] = {funcStr, fileStr, bbVal, IRB.getInt32(1)};
            IRB.CreateCall(tracer->setCaller, ArrayRef<Value*>(valueList, 4));
        }
        else if(func != NULL && func->getInstructionCount() == 0){
            llvm::Value* valueList[4] = {funcStr, fileStr, bbVal, IRB.getInt32(0)};
            IRB.CreateCall(tracer->setCaller, ArrayRef<Value*>(valueList, 4));
            llvm::IRBuilder<> IRB2(this->instr->getNextNode());
            llvm::Value* valueList2[1] = {funcStr};
            IRB2.CreateCall(tracer->startLog, ArrayRef<Value*>(valueList2, 1));
        }
    }
    return true;
    
}

bool RetInstrInfo::instrument(TracerPass* tracer, bool isFirstBlock, bool isFirstInstr){
    basicInstrument(tracer, isFirstBlock, isFirstInstr);
    /*if(tracer->getTraceMode() == TracerPass::TraceMode::TRACE_FUNC_LEVEL){
        llvm::IRBuilder<> IRB(this->instr);
        IRB.CreateCall(tracer->logCallDepthDec);
    }*/
    if(tracer->getTraceMode() == TracerPass::TraceMode::TRACE_LINE_LEVEL || tracer->getTraceMode() == TracerPass::TraceMode::TRACE_BB_LEVEL){
        //mark the boundary
        llvm::IRBuilder<> IRB(this->instr);
        Value* str = getGlobalPtr(this->instr->getFunction()->getName().str(), IRB);//IRB.CreateGlobalStringPtr(this->instr->getFunction()->getName());
        llvm::Value* valueList[1] = {str};
        IRB.CreateCall(tracer->logLineFuncEnd, ArrayRef<Value*>(valueList,1));
    }
    return true;
}
