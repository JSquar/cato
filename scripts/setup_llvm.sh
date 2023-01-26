#!/usr/bin/env bash
set -e

: ${PREFIX:=$HOME/.local}
: ${REPO_PATH:=$CATO_ROOT/../llvm-project}
: ${CC:=gcc}
: ${CXX:=g++}
: ${NPROCS:=$(nproc)}
: ${COUNTER:=1}
: ${BUILD_TYPE:=RelWithDebInfo}
: ${LINKER:=""}

if [[ -z ${LINKER} ]]; then
    if [[ ${CC} == "gcc" ]]; then
        LINKER=gold
    else
        LINKER=lld
    fi
fi

# PREFIX: Where shall llvm be installed to?
# REPO_PATH: Where is the git repo of llvm found (https://github.com/llvm/llvm-project.git)?

echo 1000 > /proc/self/oom_score_adj || true

echo -e "${GREEN}Init build (build type ${BUILD_TYPE}) of LLVM (clang; clang-tools; compiler-rt; lld; lldb; openmp) in $REPO_PATH${NC}"
echo -e "${GREEN}Installation will be performed in $PREFIX${NC}"
echo -e "${GREEN}CC=${CC}${NC}"
echo -e "${GREEN}CXX=${CXX}${NC}"
echo -e "${GREEN}LINKER=${LINKER}${NC}"
 
echo "Press <enter> to continue... "
read
cd $REPO_PATH

if [ -d  ${REPO_PATH} ]; then
    echo -e "${GREEN}Found llvm-project directory${REPO_PATH}${NC}"
else
    echo -e "${RED}Did not find llvm-project directory${NC}"
    echo -e "${RED}Exit${NC}"
    exit 1
fi

cd ${REPO_PATH}
if [ -d build ]; then
    echo -e "${YELLOW}Found build directory. Shall it be reused? (Will be deleted otherwise) (y/n)${NC}"
    if read; then
        case ${REPLY} in
            n|N|no|No)
                echo -e "${GREEN}Delete build directory...${NC}"
                sleep 2
                rm -rf build
                echo -e "${GREEN}...done${NC}"
                ;;
            y|Y|yes|Yes)
                echo -e "${GREEN}Reuse existing build directory${NC}"
                ;;
            *)
                echo -e "${RED}Unknown input${NC}"
                echo -e "${RED}Exit${NC}"
                exit 1
                ;;
        esac
    else
        echo -e "${RED}Unknown input${NC}"
        echo -e "${RED}Exit${NC}"
        exit 1
    fi #read user input
fi #found build dir
mkdir -p build
cd build

# https://bugs.llvm.org/show_bug.cgi?id=42446 sagt, dass -DLLVM_USE_LINKER=lld nicht funktioniert
CC=${CC} CXX=${CXX} cmake -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra;compiler-rt;lld;lldb;openmp" -DCMAKE_INSTALL_PREFIX=${PREFIX} -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -G "Unix Makefiles" ../llvm

echo -e "${GREEN}Start installation with ${NPROCS} thread(s)${NC}"
# attempt three full parallel builds to get as far as possible, start afterwards the "finally"-build with a single process

( (echo -e "\n\n${GREEN}Full parallel build (Attempt ${COUNTER})${NC}\n\n"; \
sleep 1s; \
if [[ ${COUNTER} -le 3 ]]; then make -j ${NPROCS}; else echo "Skip build attempt"; exit; fi ) || \
\
(echo -e "\n\n${GREEN}Full parallel build (Attempt ${COUNTER})${NC}\n\n"; \
sleep 1s; \
COUNTER=$((COUNTER+1)); \
if [[ ${COUNTER} -le 3 ]]; then make -j ${NPROCS}; else echo "Skip build attempt"; exit; fi ) || \
\
(echo -e "\n\n${GREEN}Full parallel build (Attempt ${COUNTER})${NC}\n\n"; \
sleep 1s; \
COUNTER=$((COUNTER+1)); \
if [[ ${COUNTER} -le 3 ]]; then make -j ${NPROCS}; else echo "Skip build attempt"; exit; fi ) || \
\
(echo -e "\n\n${GREEN}Final try: sequential build${NC}\n\n"; \
sleep 1s; \
COUNTER=$((COUNTER+1)); \
make -j 1) ) &&\
\
(echo -e "\n\n${GREEN}Install LLVM into ${PREFIX}${NC} (Took ${COUNTER} attempt(s))\n\n"; \
sleep 1s; \
make install)

echo -e "${GREEN}Installed LLVM (Took ${COUNTER} attempt(s))${NC}"

# move llvm-lit to installation dir, since it is not moved during installation...for reasons
if [[ -f $REPO_PATH/build/bin/llvm-lit ]]; then
    echo -e "${GREEN}Move llvm-lit to $PREFIX/bin${NC}"
    mv $REPO_PATH/build/bin/llvm-lit $PREFIX/bin/
fi
