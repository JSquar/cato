#!/bin/bash
sed -i 's/#define DEBUG_CATO_PASS 1/#define DEBUG_CATO_PASS 0/g' ${DAS_TOOL_ROOT}/src/cato/debug.h
${DAS_TOOL_ROOT}/src/scripts/build_pass.sh
cd ${DAS_TOOL_ROOT}/src/test-suite
llvm-lit -v .
sed -i 's/#define DEBUG_CATO_PASS 0/#define DEBUG_CATO_PASS 1/g' ${DAS_TOOL_ROOT}/src/cato/debug.h

