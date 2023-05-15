#!/bin/bash

MODIFICATION=0

if grep -q "#define DEBUG_CATO_PASS 1" ${CATO_ROOT}/src/cato/debug.h; then
  echo "Unset debug mode"
  sed -i 's/#define DEBUG_CATO_PASS 1/#define DEBUG_CATO_PASS 0/g' ${CATO_ROOT}/src/cato/debug.h    
  MODIFICATION=1
fi


${CATO_ROOT}/scripts/build_pass.sh
cd ${CATO_ROOT}/src/test-suite
#$CATO_ROOT/../llvm-project/build/bin/llvm-lit -v .

if [[ "${MODIFICATION}" == 1 ]]; then
  echo "Restore debug mode"
  sed -i 's/#define DEBUG_CATO_PASS 0/#define DEBUG_CATO_PASS 1/g' ${CATO_ROOT}/src/cato/debug.h
fi

