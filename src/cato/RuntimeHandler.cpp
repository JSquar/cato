#include "RuntimeHandler.h"

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>
#include "llvm/ADT/SmallVector.h"

#include <cstdlib>
#include <string>

#include "helper.h"
#include "NetCDFRegion.h"

using namespace llvm;

RuntimeHandler::RuntimeHandler(Module &M)
{
    _M = &M;
    _entry_block = nullptr;
    _finalize_block = nullptr;

    if (load_rtlibs() == false)
    {
        errs() << "ERROR: Could not load rtlibs.\n";
    }

    if (load_external_functions() == false)
    {
        errs() << "ERROR: Problem with loading external functions\n";
    }
}

bool RuntimeHandler::load_rtlibs()
{
    std::string cato_root = std::getenv("CATO_ROOT");
    std::string rtlib_module = "/src/build/rtlib.bc";

    if (cato_root.empty())
    {
        errs() << "ERROR: Environment variable CATO_ROOT is not set.\n";
        return false;
    }

    SMDiagnostic rtlib_error;


    // Load main rtlib, open the rtlib*.bc file as a LLVM Module Object
    errs() << "Load rtlib " << rtlib_module << "\n";
    _rtlib_module = getLazyIRFileModule(cato_root + rtlib_module, rtlib_error, _M->getContext());
    if (_rtlib_module == nullptr)
    {
        errs() << "ERROR: Could not load the rtlib module.\n";
        return false;
    }

    return true;    
}

void RuntimeHandler::match_function(llvm::Function **function_declaration,
                                    llvm::StringRef name)
{
    Function *func = _rtlib_module->getFunction(name);

    if (func != nullptr)
    {
        *function_declaration = cast<Function>(
            _M->getOrInsertFunction(func->getName(), func->getFunctionType()).getCallee());
    }
    else
    {
        errs() << "WARNING: Function with the name " << name
               << " was not found in rtlib\n";
        *function_declaration = nullptr;
    }
}

bool RuntimeHandler::load_external_functions()
{
    // MPI library functions
    match_function(&functions.print_hello, "_Z11print_hellov");
    match_function(&functions.test_func, "_Z9test_funciz");
    match_function(&functions.cato_initialize, "_Z15cato_initializeb");
    match_function(&functions.cato_finalize, "_Z13cato_finalizev");
    match_function(&functions.get_mpi_rank, "_Z12get_mpi_rankv");
    match_function(&functions.get_mpi_size, "_Z12get_mpi_sizev");
    match_function(&functions.mpi_barrier, "_Z11mpi_barrierv");
    match_function(&functions.allocate_shared_memory, "_Z22allocate_shared_memorylii");
    match_function(&functions.shared_memory_store, "_Z19shared_memory_storePvS_iz");
    match_function(&functions.shared_memory_load, "_Z18shared_memory_loadPvS_iz");
    match_function(&functions.shared_memory_free, "_Z18shared_memory_freePv");
    match_function(&functions.shared_memory_sequential_store,
                   "_Z30shared_memory_sequential_storePvS_iz");
    match_function(&functions.shared_memory_sequential_load,
                   "_Z29shared_memory_sequential_loadPvS_iz");
    match_function(&functions.shared_memory_pointer_store,
                   "_Z27shared_memory_pointer_storePvS_l");
    match_function(&functions.allocate_shared_value, "_Z21allocate_shared_valuePvi");
    match_function(&functions.shared_value_store, "_Z18shared_value_storePvS_");
    match_function(&functions.shared_value_load, "_Z17shared_value_loadPvS_");
    match_function(&functions.shared_value_synchronize, "_Z24shared_value_synchronizePv");
    match_function(&functions.reduce_local_vars, "_Z17reduce_local_varsPvii");
    match_function(&functions.modify_parallel_for_bounds_4,
                   "_Z26modify_parallel_for_boundsPiS_i");
    match_function(&functions.modify_parallel_for_bounds_8,
                   "_Z26modify_parallel_for_boundsPlS_l");
    match_function(&functions.critical_section_init, "_Z21critical_section_initv");
    match_function(&functions.critical_section_enter, "_Z22critical_section_enterPv");
    match_function(&functions.critical_section_leave, "_Z22critical_section_leavePv");
    match_function(&functions.critical_section_finalize, "_Z25critical_section_finalizePv");

    // netCDF library functions

    match_function(&functions.io_open, "_Z7io_openPKciPi");
    // match_function(&functions.io_open_par, "_Z11io_open_parPKciiiPi");
    match_function(&functions.io_open_par, "_Z11io_open_parPKciPi");
    match_function(&functions.io_var_par_access, "_Z17io_var_par_accessiii");
    match_function(&functions.io_inq_varid, "_Z12io_inq_varidiPcPi");
    match_function(&functions.io_get_var_int, "_Z14io_get_var_intiiPi");
    // match_function(&functions.io_get_vara_int, "_Z15io_get_vara_intiiiPi");
    match_function(&functions.io_get_vara_int, "_Z15io_get_vara_intiilPi");
    // match_function(&functions.io_get_vara_int2, "_Z16io_get_vara_int2v");
    // match_function(&functions.io_get_vara_int1a, "_Z17io_get_vara_int1ai");
    // match_function(&functions.io_get_vara_int1b, "_Z17io_get_vara_int1bPi");
    match_function(&functions.io_close, "_Z8io_closei");

    return true;
}

bool RuntimeHandler::insert_cato_init_and_fin(llvm::Function *func, bool logging)
{
    // If no function is given as argument the init and finalize blocks will
    // be added to the main function of the module
    if (func == nullptr)
    {
        func = _M->getFunction("main");
    }

    IRBuilder<> builder(func->getContext());

    // Create BasicBlocks for init and finalize code
    BasicBlock *entry_block = &func->getEntryBlock();
    SplitBlock(entry_block, entry_block->getFirstNonPHI());
    _entry_block = entry_block;

    BasicBlock *finalize_block = BasicBlock::Create(func->getContext(), "cato_finalize", func);
    _finalize_block = finalize_block;

    // Place init and finalize code into the created blocks
    bool is_void = func->getReturnType()->isVoidTy();
    Value *return_value_buffer = nullptr;

    builder.SetInsertPoint(entry_block->getTerminator());

    if (!is_void)
    {
        return_value_buffer = builder.CreateAlloca(func->getReturnType());
    }

    Value *logging_value = builder.getInt1(logging);
    builder.CreateCall(functions.cato_initialize, logging_value);

    // Add finalize in front of all function exit points
    auto return_instructions = get_instruction_in_function<ReturnInst>(func);
    for (auto &ret : return_instructions)
    {
        builder.SetInsertPoint(ret);
        if (!is_void)
        {
            builder.CreateStore(ret->getReturnValue(), return_value_buffer);
        }
        builder.CreateBr(finalize_block);
        ret->eraseFromParent();
    }

    builder.SetInsertPoint(finalize_block);
    builder.CreateCall(functions.cato_finalize);

    if (is_void)
    {
        builder.CreateRetVoid();
    }
    else
    {
        // Todo createLoad with single argument has been deprecated and removed in llvm 14, see
        // https://github.com/llvm/llvm-project/blob/75e33f71c2dae584b13a7d1186ae0a038ba98838/llvm/include/llvm/IR/IRBuilder.h#L1678
        // Value *return_value = builder.CreateLoad(return_value_buffer);
        Value *return_value =
            builder.CreateLoad(return_value_buffer->getType()->getPointerElementType(),
                               return_value_buffer, "CATO: Added Return Function");
        builder.CreateRet(return_value);
    }

    return true;
}

void RuntimeHandler::adjust_netcdf_regions()
{
    errs() << "Function: adjust_netcdf_regions\n";
    std::vector<std::unique_ptr<NetCDFRegion>> netcdf_regions;

    /* ----------------------------- Replace nc_open ---------------------------- */
    std::vector<llvm::User *>  open_users = get_function_users(*_M, "nc_open");
    llvm::errs() << "Found " << open_users.size() << " many open calls\n"; //TODO
    if (open_users.size() == 0)
    {
        llvm::errs() << "Did not find any netCDF output file. Will skip therefore netCDF part\n";
        return;
    }

    for (auto &user : open_users)
    {
        if (auto *call = llvm::dyn_cast<llvm::CallInst>(user))
        {        
            std::unique_ptr<NetCDFRegion> netcdf_region = std::make_unique<NetCDFRegion>(NetCDFRegion(call));
            netcdf_regions.push_back(std::move(netcdf_region));

            //replace with nc_open_par
            call->setCalledFunction(functions.io_open_par);
        }
    }    

    /* ---------------------- inq varid and set par access ---------------------- */
    std::vector<llvm::User *>  inq_varid_users = get_function_users(*_M, "nc_inq_varid");
    llvm::errs() << "Found " << inq_varid_users.size() << " many nc_inq_varid calls\n"; //TODO
    for (auto &user : inq_varid_users)
    {
        if (auto *call = llvm::dyn_cast<llvm::CallInst>(user))
        {   
            //replace with CATO inq_varid
            call->setCalledFunction(functions.io_inq_varid);
        }
    }

    /* ----------- parallel access via netCDF partial access functions ---------- */
    std::vector<llvm::User *>  get_var_int_users = get_function_users(*_M, "nc_get_var_int");
    std::vector<llvm::User *>  shared_memory_users = get_function_users(*_M, "_Z22allocate_shared_memorylii"); //TODO
    llvm::errs() << "Found " << get_var_int_users.size() << " many nc_get_var_int calls\n"; //TODO
    llvm::errs() << "Found " << shared_memory_users.size() << " shared memory calls\n"; //TODO

    IRBuilder<> builder(_M->getContext());
    LLVMContext &Ctx = _M->getContext();

    llvm::User *memory_call_user = shared_memory_users.at(0);
    llvm::CallInst *memory_call = llvm::dyn_cast<llvm::CallInst>(memory_call_user);
    llvm::Value *num_bytes = memory_call->getArgOperand(0);

    for (auto &user : get_var_int_users)
    {

        if (auto *call = llvm::dyn_cast<llvm::CallInst>(user))
        {   


            llvm::Value *ncid = call->getArgOperand(0);
            llvm::Value *varid = call->getArgOperand(1);
            llvm::Value *buffer = call->getArgOperand(2);

            SmallVector<Value *> args;//,args2,args3,args4;
            args.push_back(ncid);
            args.push_back(varid);
            args.push_back(num_bytes);
            args.push_back(buffer);

            builder.SetInsertPoint(call);
            llvm::CallInst *new_call = builder.CreateCall(functions.io_get_vara_int, args);
            call->replaceAllUsesWith(new_call);
            call->eraseFromParent();

        }
    }
}


void RuntimeHandler::replace_omp_functions()
{
    // omp_get_thread_num
    auto get_thread_num_users = get_function_users(*_M, "omp_get_thread_num");
    for (auto *user : get_thread_num_users)
    {
        if (auto *invoke = dyn_cast<InvokeInst>(user))
        {
            invoke->setCalledFunction(functions.get_mpi_rank);
        }
        else if (auto *call = dyn_cast<CallInst>(user))
        {
            call->setCalledFunction(functions.get_mpi_rank);
        }
    }

    // omp_get_num_threads
    auto get_num_threads_users = get_function_users(*_M, "omp_get_num_threads");
    for (auto *user : get_num_threads_users)
    {
        if (auto *invoke = dyn_cast<InvokeInst>(user))
        {
            invoke->setCalledFunction(functions.get_mpi_size);
        }
        else if (auto *call = dyn_cast<CallInst>(user))
        {
            call->setCalledFunction(functions.get_mpi_size);
        }
    }

    // omp barrier
    LLVMContext &Ctx = _M->getContext();
    IRBuilder<> builder(Ctx);
    auto omp_barrier_users = get_function_users(*_M, "__kmpc_barrier");
    for (auto *user : omp_barrier_users)
    {
        if (auto *call = dyn_cast<CallInst>(user))
        {
            builder.SetInsertPoint(call);
            auto new_call = builder.CreateCall(functions.mpi_barrier);
            call->replaceAllUsesWith(new_call);
            call->eraseFromParent();
        }
    }
}

llvm::BasicBlock *RuntimeHandler::get_entry_block() { return _entry_block; }

llvm::BasicBlock *RuntimeHandler::get_finalize_block() { return _finalize_block; }
