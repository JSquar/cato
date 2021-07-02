#include "SharedPointer.h"

using namespace llvm;

SharedPointer::SharedPointer(Value *value, Function *microtask, int depth)
    : SharedVariable(value, microtask)
{
    _pointer_depth = depth;
}
