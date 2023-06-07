#!/usr/bin/python3
import argparse
import os
import shlex
import subprocess


def run_command(command, verbose=False):
    if verbose:
        print(command)
    process = subprocess.Popen(shlex.split(command), stdout=subprocess.PIPE)
    result = ""
    while True:
        output = process.stdout.readline().decode()
        if output == "" and process.poll() is not None:
            break
        if output:
            print(output.strip())
            result += output.strip()
    rc = process.poll()
    # Todo das muss überarbeitet werden, das ist nur quick and dirty
    return rc, result


parser = argparse.ArgumentParser(description="Compile the given program with CATO")
parser.add_argument("infile", type=str, help="input file")
parser.add_argument("-o", "--output", default="translated", help="Name of output file")
parser.add_argument(
    "-l",
    "--logging",
    action="store_true",
    help="Enables logging for the produced binary",
)

arguments = parser.parse_args()

# flags von nc-config --cflags und llvm-config-cxxflags
CXXFLAGS = " "
CXXFLAGS += " -O2 -g0 -fopenmp -Wunknown-pragmas "
# CXXFLAGS += " " + run_command("llvm-config --cflags")[1]
CXXFLAGS += " " + run_command("nc-config --cflags")[1]
PASS_PATH = os.environ["CATO_ROOT"] + "/src/build/cato/libCatoPass.so"
RTLIB_DIR = os.environ["CATO_ROOT"] + "/src/build/cato/rtlib"
LOGGING = ""

# flags aus nc-config --libs und llvm-config --libs
LIBS = ""
# LIBS = run_command("llvm-config --libs")[1]
LIBS += " " + run_command("nc-config --libs")[1]

if arguments.logging:
    LOGGING = "-mllvm --cato-logging"

compile_cmd = (
    "mpicc -cc=clang "
    + CXXFLAGS
    + " -o "
    + arguments.output
    + ".o "
    + "-Xclang -load -Xclang "
    + PASS_PATH
    + " "
    + LOGGING
    + " -c "
    + arguments.infile
    + " -flegacy-pass-manager"
)

unmodified_ir_cmd = (
    "mpicc -cc=clang "
    #+ CXXFLAGS
    + " -g0 -fopenmp -Wunknown-pragmas "
    + "  "
    + arguments.infile
    + " -flegacy-pass-manager -S -emit-llvm"
)

#modified_ir_cmd = (
#    "opt -enable-new-pm=0"
#    + " -o "
#    + arguments.output
#    + "_modified.ll"
#    + " -load "
#    + PASS_PATH
#    + " -Cato "
#    + " "
#    + arguments.infile.split(".")[0]
#    + ".ll"
#    + " -S"
#)
modified_ir_cmd = (
    "mpicc -cc=clang "
    + CXXFLAGS
    + " -o "
    + arguments.output
    + "_modified.ll "
    + " -Xclang -load -Xclang "
    + PASS_PATH
    + " -S "
    + arguments.infile
    + " -emit-llvm -flegacy-pass-manager"
)

link_cmd = (
    "mpicc -cc=clang -v "
    + CXXFLAGS
    + " -o "
    + arguments.output
    + "_modified.x "
    + arguments.output
    + ".o "
    + RTLIB_DIR
    + "/libCatoRuntime.so "
    + "-Wl,-rpath,"
    + RTLIB_DIR
    + LIBS
)

rm_cmd = "rm " + arguments.output + ".o"

# cmd_create_ir = f"mpicc -cc=clang -S -emit-llvm {CXXFLAGS} {arguments.infile} -flegacy-pass-manager"
# cmd_create_modified_ir = f"opt -load-pass-plugin={RTLIB_DIR + '/libCatoRuntime.so'} -passes=Cato {file_ir} -S -o {file_ir_modified}"
# cmd_create_modified_bc = f"llvm-as {file_ir_modified} -o {file_bc}"
# cmd_link = f"mpicc -cc=clang -o {file_output} {file_bc} {rtlib_location}"

run_command(compile_cmd)
run_command(unmodified_ir_cmd, False)
run_command(modified_ir_cmd, False)
run_command(link_cmd, False)
run_command(rm_cmd)
