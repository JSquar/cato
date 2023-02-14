#!/bin/bash
sed -i 's/#define DEBUG_CATO_PASS 1/#define DEBUG_CATO_PASS 0/g' ${CATO_ROOT}/src/cato/debug.h
${CATO_ROOT}/scripts/build_pass.sh
cd ${CATO_ROOT}/src/test-suite
$CATO_ROOT/../llvm-project/build/bin/llvm-lit -v .
sed -i 's/#define DEBUG_CATO_PASS 0/#define DEBUG_CATO_PASS 1/g' ${CATO_ROOT}/src/cato/debug.h

