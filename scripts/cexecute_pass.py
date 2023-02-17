#!/usr/bin/python3
import argparse
import os
import shlex
import subprocess


def run_command(command, verbose=False):
    if verbose:
        print(command)
    process = subprocess.Popen(shlex.split(command), stdout=subprocess.PIPE)
    while True:
        output = process.stdout.readline().decode()
        if output == '' and process.poll() is not None:
            break
        if output:
            print(output.strip())
    rc = process.poll()
    return rc

parser = argparse.ArgumentParser(description="Compile the given program with CATO")
parser.add_argument("infile", type=str, help="input file")
parser.add_argument("-o", "--output", default="translated", help="Name of output file")
parser.add_argument("-l", "--logging", action="store_true", help="Enables logging for the produced binary")

arguments = parser.parse_args()

# flags von nc-config --cflags und llvm-config-cxxflags
CXXFLAGS = "-O2 -g0 -fopenmp -Wunknown-pragmas -I/opt/spack/20220821/opt/spack/linux-ubuntu20.04-x86_64/gcc-12.1.0/netcdf-c-4.8.1-mrzzu4i2goofwbb2hzxmh2ik6kmowtmx/include -I/opt/spack/20220821/opt/spack/linux-ubuntu20.04-x86_64/gcc-12.1.0/hdf5-1.12.2-nlwdxpu2dqru6ic6ihfeiprfdubfrnvn/include -I/home/squar/opt/llvm-RelWithDebInfo_V-MB/include  -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS"
PASS_PATH = os.environ["CATO_ROOT"] + "/src/build/cato/libCatoPass.so"
RTLIB_DIR = os.environ["CATO_ROOT"] + "/src/build/cato/rtlib"
LOGGING = ""

# flags aus nc-config --libs und llvm-config --libs
LIBS="-L/opt/spack/20220821/opt/spack/linux-ubuntu20.04-x86_64/gcc-12.1.0/netcdf-c-4.8.1-mrzzu4i2goofwbb2hzxmh2ik6kmowtmx/lib -lnetcdf -lLLVMWindowsManifest -lLLVMXRay -lLLVMLibDriver -lLLVMDlltoolDriver -lLLVMCoverage -lLLVMLineEditor -lLLVMXCoreDisassembler -lLLVMXCoreCodeGen -lLLVMXCoreDesc -lLLVMXCoreInfo -lLLVMX86Disassembler -lLLVMX86AsmParser -lLLVMX86CodeGen -lLLVMX86Desc -lLLVMX86Info -lLLVMWebAssemblyDisassembler -lLLVMWebAssemblyAsmParser -lLLVMWebAssemblyCodeGen -lLLVMWebAssemblyDesc -lLLVMWebAssemblyUtils -lLLVMWebAssemblyInfo -lLLVMSystemZDisassembler -lLLVMSystemZAsmParser -lLLVMSystemZCodeGen -lLLVMSystemZDesc -lLLVMSystemZInfo -lLLVMSparcDisassembler -lLLVMSparcAsmParser -lLLVMSparcCodeGen -lLLVMSparcDesc -lLLVMSparcInfo -lLLVMRISCVDisassembler -lLLVMRISCVAsmParser -lLLVMRISCVCodeGen -lLLVMRISCVDesc -lLLVMRISCVInfo -lLLVMPowerPCDisassembler -lLLVMPowerPCAsmParser -lLLVMPowerPCCodeGen -lLLVMPowerPCDesc -lLLVMPowerPCInfo -lLLVMNVPTXCodeGen -lLLVMNVPTXDesc -lLLVMNVPTXInfo -lLLVMMSP430Disassembler -lLLVMMSP430AsmParser -lLLVMMSP430CodeGen -lLLVMMSP430Desc -lLLVMMSP430Info -lLLVMMipsDisassembler -lLLVMMipsAsmParser -lLLVMMipsCodeGen -lLLVMMipsDesc -lLLVMMipsInfo -lLLVMLanaiDisassembler -lLLVMLanaiCodeGen -lLLVMLanaiAsmParser -lLLVMLanaiDesc -lLLVMLanaiInfo -lLLVMHexagonDisassembler -lLLVMHexagonCodeGen -lLLVMHexagonAsmParser -lLLVMHexagonDesc -lLLVMHexagonInfo -lLLVMBPFDisassembler -lLLVMBPFAsmParser -lLLVMBPFCodeGen -lLLVMBPFDesc -lLLVMBPFInfo -lLLVMAVRDisassembler -lLLVMAVRAsmParser -lLLVMAVRCodeGen -lLLVMAVRDesc -lLLVMAVRInfo -lLLVMARMDisassembler -lLLVMARMAsmParser -lLLVMARMCodeGen -lLLVMARMDesc -lLLVMARMUtils -lLLVMARMInfo -lLLVMAMDGPUDisassembler -lLLVMAMDGPUAsmParser -lLLVMAMDGPUCodeGen -lLLVMAMDGPUDesc -lLLVMAMDGPUUtils -lLLVMAMDGPUInfo -lLLVMAArch64Disassembler -lLLVMAArch64AsmParser -lLLVMAArch64CodeGen -lLLVMAArch64Desc -lLLVMAArch64Utils -lLLVMAArch64Info -lLLVMOrcJIT -lLLVMMCJIT -lLLVMJITLink -lLLVMInterpreter -lLLVMExecutionEngine -lLLVMRuntimeDyld -lLLVMOrcTargetProcess -lLLVMOrcShared -lLLVMSymbolize -lLLVMDebugInfoPDB -lLLVMDebugInfoGSYM -lLLVMOption -lLLVMObjectYAML -lLLVMMCA -lLLVMMCDisassembler -lLLVMLTO -lLLVMPasses -lLLVMCFGuard -lLLVMCoroutines -lLLVMObjCARCOpts -lLLVMipo -lLLVMVectorize -lLLVMLinker -lLLVMInstrumentation -lLLVMFrontendOpenMP -lLLVMFrontendOpenACC -lLLVMExtensions -lLLVMDWARFLinker -lLLVMGlobalISel -lLLVMMIRParser -lLLVMAsmPrinter -lLLVMDebugInfoMSF -lLLVMDebugInfoDWARF -lLLVMSelectionDAG -lLLVMCodeGen -lLLVMIRReader -lLLVMAsmParser -lLLVMInterfaceStub -lLLVMFileCheck -lLLVMFuzzMutate -lLLVMTarget -lLLVMScalarOpts -lLLVMInstCombine -lLLVMAggressiveInstCombine -lLLVMTransformUtils -lLLVMBitWriter -lLLVMAnalysis -lLLVMProfileData -lLLVMObject -lLLVMTextAPI -lLLVMMCParser -lLLVMMC -lLLVMDebugInfoCodeView -lLLVMBitReader -lLLVMCore -lLLVMRemarks -lLLVMBitstreamReader -lLLVMBinaryFormat -lLLVMTableGen -lLLVMSupport -lLLVMDemangle"
if(arguments.logging):
    LOGGING = "-mllvm --cato-logging"

compile_cmd = "mpicc -cc=clang " + CXXFLAGS + " -o " + arguments.output + ".o " + \
        "-Xclang -load -Xclang " + PASS_PATH + " " + LOGGING + " -c " + arguments.infile + " -flegacy-pass-manager"

unmodified_ir_cmd = "mpicc -cc=clang " + CXXFLAGS + "  " + arguments.infile + " -flegacy-pass-manager -S -emit-llvm"

modified_ir_cmd = "opt -enable-new-pm=0" + " -o " + arguments.output + ".ll" + \
        " -load " + PASS_PATH + " -Cato " + " "  +  arguments.infile.split(".")[0]+".ll" + " -S"

link_cmd = "mpicc -cc=clang " + CXXFLAGS + " -o " + arguments.output + " " + \
        arguments.output + ".o " + LIBS + " " + RTLIB_DIR + "/libCatoRuntime.so " + "-Wl,-rpath," + RTLIB_DIR

rm_cmd = "rm " + arguments.output + ".o"

# cmd_create_ir = f"mpicc -cc=clang -S -emit-llvm {CXXFLAGS} {arguments.infile} -flegacy-pass-manager"
# cmd_create_modified_ir = f"opt -load-pass-plugin={RTLIB_DIR + '/libCatoRuntime.so'} -passes=Cato {file_ir} -S -o {file_ir_modified}"
# cmd_create_modified_bc = f"llvm-as {file_ir_modified} -o {file_bc}"
# cmd_link = f"mpicc -cc=clang -o {file_output} {file_bc} {rtlib_location}"

run_command(compile_cmd)
run_command(unmodified_ir_cmd, True)
run_command(modified_ir_cmd, True)
# run_command(link_cmd)
# run_command(rm_cmd)



