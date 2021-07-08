#!/bin/bash
sed -i 's/#define DEBUG_CATO_PASS 1/#define DEBUG_CATO_PASS 0/g' ${CATO_ROOT}/src/cato/debug.h
${CATO_ROOT}/src/scripts/build_pass.sh
cd ${CATO_ROOT}/src/test-suite
llvm-lit -v .
sed -i 's/#define DEBUG_CATO_PASS 0/#define DEBUG_CATO_PASS 1/g' ${CATO_ROOT}/src/cato/debug.h

