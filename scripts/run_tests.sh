#!/bin/bash

#: ${LLVM_LIT:=$(dirname $(which clang))/llvm-lit}
: ${LLVM_LIT:=${CATO_ROOT}/../llvm-project/build/bin/llvm-lit}

sed -i 's/#define DEBUG_CATO_PASS 1/#define DEBUG_CATO_PASS 0/g' ${CATO_ROOT}/src/cato/debug.h
${CATO_ROOT}/scripts/build_pass.sh
cd ${CATO_ROOT}/src/test-suite
#llvm-lit -v .
#$CATO_ROOT/../llvm-project/build/bin/llvm-lit -v .
echo "Use ${LLVM_LIT}"
${LLVM_LIT} -v .
sed -i 's/#define DEBUG_CATO_PASS 0/#define DEBUG_CATO_PASS 1/g' ${CATO_ROOT}/src/cato/debug.h

