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
    # Todo das muss überarbeitet werden, das ist nur quicky and dirty
    return rc, result


# ---------------------------------------------------------------------------- #
#                                  PARSE ARGS                                  #
# ---------------------------------------------------------------------------- #

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
if arguments.logging:
    arg_logging = "-mllvm --cato-logging"

# ------------------------------ CFLAGS/CXXFLAGS ----------------------------- #

if "CXXFLAGS" in os.environ:
    arg_cxxflags = os.environ["CXXFLAGS"]
else:
    arg_cxxflags = ""
if "CFLAGS" in os.environ:
    arg_cxxflags += os.environ["CFLAGS"]
arg_cxxflags += " -ggdb -Og -fopenmp -Wunknown-pragmas "
arg_cxxflags += " " + run_command("nc-config --cflags")[1]

# --------------------------------- LIBRARIES -------------------------------- #

if "LIBS" in os.environ:
    arg_libs = os.environ["LIBS"]
else:
    arg_libs = ""
arg_libs += " " + run_command("nc-config --libs")[1]

# ----------------------------------- PATHS ---------------------------------- #
arg_pass_dir = os.environ["CATO_ROOT"] + "/build/src/cato/libCatoPass.so"
arg_rtlib_dir = os.environ["CATO_ROOT"] + "/build/src/cato/rtlib"


# ---------------------------------------------------------------------------- #
#                          DEFINE COMPILATION COMMANDS                         #
# ---------------------------------------------------------------------------- #

compile_cmd = (
    "mpicc -cc=clang "
    + arg_cxxflags
    + " -o "
    + arguments.output
    + ".o "
    + "-Xclang -load -Xclang "
    + arg_pass_dir
    + " "
    + arg_logging
    + " -c "
    + arguments.infile
    + " -flegacy-pass-manager"
)

unmodified_ir_cmd = (
    "mpicc -cc=clang "
    + arg_cxxflags
    + "  "
    + arguments.infile
    + " -flegacy-pass-manager -S -emit-llvm"
)

modified_ir_cmd = (
    "opt -enable-new-pm=0"
    + " -o "
    + arguments.output
    + "_modified.ll"
    + " -load "
    + arg_pass_dir
    + " -Cato "
    + " "
    + arguments.infile.split(".")[0]
    + ".ll"
    + " -S"
)

link_cmd = (
    "mpicc -cc=clang -v "
    + arg_cxxflags
    + " -o "
    + arguments.output
    + "_modified.x "
    + arguments.output
    + ".o "
    + arg_rtlib_dir
    + "/libCatoRuntime.so "
    + "-Wl,-rpath,"
    + arg_rtlib_dir
    + " "
    + arg_libs
)

rm_cmd = "rm " + arguments.output + ".o"

# cmd_create_ir = f"mpicc -cc=clang -S -emit-llvm {arg_cxxflags} {arguments.infile} -flegacy-pass-manager"
# cmd_create_modified_ir = f"opt -load-pass-plugin={arg_rtlib_dir + '/libCatoRuntime.so'} -passes=Cato {file_ir} -S -o {file_ir_modified}"
# cmd_create_modified_bc = f"llvm-as {file_ir_modified} -o {file_bc}"
# cmd_link = f"mpicc -cc=clang -o {file_output} {file_bc} {rtlib_location}"

# ---------------------------------------------------------------------------- #
#                                RUN COMPILATION                               #
# ---------------------------------------------------------------------------- #

run_command(compile_cmd)
run_command(unmodified_ir_cmd, False)
run_command(modified_ir_cmd, False)
run_command(link_cmd, False)
run_command(rm_cmd)
