#!/usr/bin/env bash
set -e

: ${PREFIX:=$HOME/.local}
: ${REPO_PATH:=$DAS_TOOL_ROOT/../llvm-project}
: ${CC:=gcc}
: ${CXX:=g++}

# PREFIX: Where shall llvm be installed to?
# REPO_PATH: Where is the git repo of llvm found (https://github.com/llvm/llvm-project.git)?

echo -e "${GREEN}Init build of LLVM (clang; clang-tools; compiler-rt; lld; openmp) in $REPO_PATH${NC}"
echo -e "${GREEN}Installation will be performed in $PREFIX${NC}"
echo -e "${GREEN}CC=${CC}${NC}"
echo -e "${GREEN}CXX=${CXX}${NC}"
 
echo "Press <enter> to continue... "
read
cd $REPO_PATH

: ${CC:=gcc}
: ${CXX:=g++}

if [ -d  ${REPO_PATH} ]; then
    echo -e "${GREEN}Found llvm-project directory${REPO_PATH}${NC}"
else
    echo -e "${RED}Did not find llvm-project directory${NC}"
    echo -e "${RED}Exit${NC}"
    exit 1
fi

# earlyoom should be running! (to kill installation freeze due to oom, optional)
if [[ $(ps aux|grep earlyoom|grep -v grep|wc -l ) -eq 0 ]]; then
    echo -e "${YELLOW}Earlyoom is not running! Continue? (y/n)${NC}"
    if read; then
        case ${REPLY} in
            y|Y|yes|Yes)
                ;;
            *)
                echo -e "${RED}Exit${NC}"
                exit 1
                ;;
        esac
    else
        echo -e "${RED}Unknown input${NC}"
        echo -e "${RED}Exit${NC}"
        exit 1
    fi #read user input
fi
sleep 1s

cd ${REPO_PATH}
if [ -d build ]; then
    echo -e "${YELLOW}Found build directory. Shall it be deleted? (Will be reused otherwise) (y/n)${NC}"
    if read; then
        case ${REPLY} in
            y|Y|yes|Yes)
                rm -rf build
                echo -e "${GREEN}Deleted build directory${NC}"
                ;;
            n|N|no|No)
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

CC=${CC} CXX=${CXX} cmake -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra;compiler-rt;lld;openmp" -DCMAKE_INSTALL_PREFIX=${PREFIX} -DLLVM_USE_LINKER=gold -G "Unix Makefiles" ../llvm

# attempt three full parallel builds to get as far as possible, start afterwards the "finally"-build with a single process
( (echo -e "\n\n${GREEN}Full parallel build (Attempt 1)${NC}\n\n"; \
sleep 1s; \
make -j $(nproc) ) || \
\
(echo -e "\n\n${GREEN}Full parallel build (Attempt 2)${NC}\n\n"; \
sleep 1s; \
make -j $(nproc) ) || \
\
(echo -e "\n\n${GREEN}Full parallel build (Attempt 3)${NC}\n\n"; \
sleep 1s; \
make -j $(nproc) ) || \
\
(echo -e "\n\n${GREEN}Sequential build${NC}\n\n"; \
sleep 1s; \
make -j 1) ) &&\
\
(echo -e "\n\n${GREEN}Install LLVM into ${PREFIX}${NC}\n\n"; \
sleep 1s; \
make install)

echo -e "${GREEN}Installed LLVM${NC}"

# move llvm-lit to installation dir, since it is not moved during installation...for reasons
if [[ -f $REPO_PATH/build/bin/llvm-lit ]]; then
    echo -e "${GREEN}Move llvm-lit to $PREFIX/bin${NC}"
    mv $REPO_PATH/build/bin/llvm-lit $PREFIX/bin/
fi
