#include "SharedValue.h"

using namespace llvm;

SharedValue::SharedValue(Value *value, Function *microtask) : SharedVariable(value, microtask)
{
}
