/*
 * File: SCEVHelper.h
 * Author: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Last Modified: Mon Sep 11 2023
 * Modified By: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Copyright (c) 2023 Niclas Schroeter
 */

#ifndef CATO_SCEV_HELPER_H
#define CATO_SCEV_HELPER_H

#include <vector>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Function.h>
#include <llvm/Analysis/ScalarEvolution.h>
#include <llvm/Analysis/ScalarEvolutionExpressions.h>
#include <unordered_map>
#include "RuntimeHandler.h"


std::vector<llvm::CallInst*> get_shared_memory_loads_for_function(llvm::Function* func, RuntimeHandler& runtime);

const llvm::SCEVAddRecExpr* extract_add_rec_expression(const llvm::SCEV* index_scev);

int extract_stride_from_step_scev(const llvm::SCEV* step_scev);

void store_stride_for_mem_abstraction(std::unordered_map<llvm::Value*,std::vector<int>>& map, llvm::CallInst* call, int stride);

int average_strides(std::vector<int>& strides);

void insert_set_stride(llvm::Function* func, llvm::Value* call, int stride, RuntimeHandler& runtime);


#endif