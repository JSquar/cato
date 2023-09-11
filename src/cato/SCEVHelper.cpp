/*
 * File: SCEVHelper.cpp
 * Author: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Last Modified: Mon Sep 11 2023
 * Modified By: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Copyright (c) 2023 Niclas Schroeter
 */

#include <numeric>
#include <llvm/IR/IRBuilder.h>
#include "SCEVHelper.h"
#include "debug.h"

using namespace llvm;

std::vector<CallInst*> get_shared_memory_loads_for_function(Function* func, RuntimeHandler& runtime)
{
    std::vector<CallInst*> shared_loads;
    for (BasicBlock& BB : *func)
    {
        for (Instruction& I: BB)
        {
            if (auto* call = dyn_cast<CallInst>(&I))
            {
                if (call->getCalledFunction() == runtime.functions.shared_memory_load)
                {
                    shared_loads.push_back(call);
                }
            }
        }
    }
    return shared_loads;
}

const SCEVAddRecExpr* extract_add_rec_expression(const SCEV* index_scev)
{
    const SCEVAddRecExpr* add_rec_expr;
    if ((add_rec_expr = dyn_cast<SCEVAddRecExpr>(index_scev)))
    {
        Debug(errs() << "Extracted AddRec directly from index SCEV:";);
        Debug(add_rec_expr->dump(););
    }
    else if (auto* cast_expr = dyn_cast<SCEVCastExpr>(index_scev))
    {
        Debug(errs() << "SCEV inherits from CastExpr\n";);
        for (auto* operand : cast_expr->operands())
        {
            if ((add_rec_expr = dyn_cast<SCEVAddRecExpr>(operand)))
            {
                Debug(errs() << "Extracted AddRec:";);
                Debug(add_rec_expr->dump(););
                break;
            }
        }
    }
    else
    {
        Debug(errs() << "Didn't find AddRec, skipping instruction\n";);
        add_rec_expr = nullptr;
    }

    return add_rec_expr;
}


int extract_stride_from_step_scev(const SCEV* step_scev)
{
    int stride_of_access = 0;

    if (auto* scev_constant = dyn_cast<SCEVConstant>(step_scev))
    {
        Debug(errs() << "Extracting the int constant\n";);
        ConstantInt* constant = scev_constant->getValue();
        Debug(constant->dump(););
        stride_of_access = constant->getSExtValue();
    }

    return stride_of_access;
}

void store_stride_for_mem_abstraction(std::unordered_map<Value*,std::vector<int>>& map, CallInst* call, int stride)
{
    BitCastInst* target_addr = dyn_cast<BitCastInst>(call->getOperand(0));
    Value* load_inst = target_addr->getOperand(0);

    auto map_entry = map.find(load_inst);
    if (map_entry == map.end())
    {
        map.insert({load_inst, {stride}});
    }
    else
    {
        map_entry->second.push_back(stride);
    }

    Debug(errs() << "Stride stored for target load:";);
    Debug(load_inst->dump(););
}

int average_strides(std::vector<int>& strides)
{
    long sum = std::accumulate(strides.begin(), strides.end(), 0L);
    if (sum == 0)
    {
        Debug(errs() << "Sum of strides is somehow 0, skipping this instruction\n";);
        return 0;
    }
    return std::round((1.0*sum) / strides.size());
}

void insert_set_stride(Function* func, Value* call, int stride, RuntimeHandler& runtime)
{
    LLVMContext& ctx = func->getContext();
    IRBuilder<> builder(ctx);

    LoadInst* load_inst = dyn_cast<LoadInst>(call);
    Debug(load_inst->dump(););
    builder.SetInsertPoint(func->getEntryBlock().getFirstNonPHI());
    Value* first_load = builder.CreateLoad(load_inst->getPointerOperand());
    Value* bitcast = builder.CreateBitCast(first_load, Type::getInt8PtrTy(ctx));
    std::vector<Value*> args {bitcast};
    args.push_back(ConstantInt::getSigned(Type::getInt32Ty(ctx), stride));
    builder.CreateCall(runtime.functions.set_read_ahead_stride, args);
}