#include "Microtask.h"

#include "SharedPointer.h"
#include "SharedValue.h"
#include "helper.h"

using namespace llvm;

Microtask::Microtask(CallInst *fork_call)
{
    _fork_call = fork_call;

    // Get the microtask function from the fork calls arguments
    Value *microtask_arg = fork_call->getArgOperand(2);
    _function = dyn_cast<Function>(microtask_arg->stripPointerCasts());

    // Collect the shared variables
    for (Argument &argument : _function->args())
    {
        // The first shared variable has index 2
        if (argument.getArgNo() > 1)
        {
            //_shared_variables.push_back(new SharedVariable(&argument, _function));
            int pointer_depth = get_pointer_depth(&argument);

            assert(pointer_depth > 0 && "Shared Variable is not of pointer type");
            _shared_variables.push_back(&argument);
            // if (pointer_depth == 1)
            // {
            //     _shared_variables.push_back(&argument);
            // }
            // else
            // {
            //     _shared_variables.push_back(&argument);
            // }
        }
    }

    auto call_instructions = get_instruction_in_function<CallInst>(_function);

    // Look for a parallel for inside the microtask;
    ParallelForData tmp_parallel_for;
    tmp_parallel_for.init = nullptr;
    tmp_parallel_for.fini = nullptr;
    for (auto &call : call_instructions)
    {
        std::vector<std::string> function_list = {
            "__kmpc_for_static_init_4", "__kmpc_for_static_init_4u",
            "__kmpc_for_static_init_8", "__kmpc_for_static_init_8u"};

        if (is_in_list(call->getCalledFunction()->getName(), function_list))
        {
            tmp_parallel_for.init = call;
        }
        else if (call->getCalledFunction()->getName().equals("__kmpc_for_static_fini"))
        {
            tmp_parallel_for.fini = call;
        }

        if (tmp_parallel_for.init != nullptr && tmp_parallel_for.fini != nullptr)
        {
            _parallel_for.push_back(tmp_parallel_for);
            tmp_parallel_for.init = nullptr;
            tmp_parallel_for.fini = nullptr;
        }
    }

    // Look for a reduction inside the microtask;
    ReductionData tmp_reduction_data;
    tmp_reduction_data.reduce = nullptr;
    tmp_reduction_data.end_reduce = nullptr;
    for (auto &call : call_instructions)
    {
        std::vector<std::string> function_list_start = {"__kmpc_reduce_nowait",
                                                        "__kmpc_reduce"};
        std::vector<std::string> function_list_end = {"__kmpc_end_reduce_nowait",
                                                      "__kmpc_end_reduce"};

        if (is_in_list(call->getCalledFunction()->getName(), function_list_start))
        {
            tmp_reduction_data.reduce = call;
        }
        else if (is_in_list(call->getCalledFunction()->getName(), function_list_end))
        {
            tmp_reduction_data.end_reduce = call;
        }

        if (tmp_reduction_data.reduce != nullptr && tmp_reduction_data.end_reduce != nullptr)
        {
            _reduction.push_back(tmp_reduction_data);
            tmp_reduction_data.reduce = nullptr;
            tmp_reduction_data.end_reduce = nullptr;
        }
    }

    // Look for a critical inside the microtask
    CriticalData tmp_critical_data;
    tmp_critical_data.critical = nullptr;
    tmp_critical_data.end_critical = nullptr;
    for (auto &call : call_instructions)
    {
        if (call->getCalledFunction()->getName().equals("__kmpc_critical"))
        {
            tmp_critical_data.critical = call;
        }
        else if (call->getCalledFunction()->getName().equals("__kmpc_end_critical"))
        {
            tmp_critical_data.end_critical = call;
        }

        if (tmp_critical_data.critical != nullptr && tmp_critical_data.end_critical != nullptr)
        {
            _critical.push_back(tmp_critical_data);
            tmp_critical_data.critical = nullptr;
            tmp_critical_data.end_critical = nullptr;
        }
    }
}

Microtask::~Microtask() {}

CallInst *Microtask::get_fork_call() { return _fork_call; }

Function *Microtask::get_function() { return _function; }

std::vector<ParallelForData> *Microtask::get_parallel_for()
{
    if (_parallel_for.size() > 0)
    {
        return &_parallel_for;
    }
    else
    {
        return nullptr;
    }
}

std::vector<ReductionData> *Microtask::get_reductions()
{
    if (_reduction.size() > 0)
    {
        return &_reduction;
    }
    else
    {
        return nullptr;
    }
}

std::vector<CriticalData> *Microtask::get_critical()
{
    if (_critical.size() > 0)
    {
        return &_critical;
    }
    else
    {
        return nullptr;
    }
}

bool Microtask::has_shared_variables() { return _shared_variables.size(); }

std::vector<Value *> &Microtask::get_shared_variables() { return _shared_variables; }
