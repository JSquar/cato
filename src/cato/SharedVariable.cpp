#include "SharedVariable.h"

using namespace llvm;

SharedVariable::SharedVariable(Value *value, Function *microtask)
{
    _value = value;
    _microtask = microtask;
}
