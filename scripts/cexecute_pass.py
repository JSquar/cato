#!/usr/bin/env python3
# -*- coding:utf-8 -*-
###
#File: cexecute_pass.py
#Author: Michael Blesel
#-----
#
#-----
#Last Modified: Saturday, 21st January 2023 12:25:58 am
#Modified By: Jannek Squar (jannek.squar@uni-hamburg.de)
#-----
#Copyright (c) 2023 Jannek Squar
#Copyright (c) 2020 Michael Blesel
#
###
import os
import subprocess
import argparse
import shlex
from pathlib import Path

def run_command(command):
    process = subprocess.Popen(shlex.split(command), stdout=subprocess.PIPE)
    while True:
        output = process.stdout.readline().decode()
        if output == "" and process.poll() is not None:
            break
        if output:
            print(output.strip())
    rc = process.poll()
    return rc


parser = argparse.ArgumentParser(description="Compile the given program with CATO")
parser.add_argument("infile", type=str, help="Input C file")
parser.add_argument("-o", "--output", type=Path, default=Path("translated"), help="Name of output file")

args_feedback = parser.add_argument_group("Feedback", "Flags do not alter internal behaviour of CATO but provide more output for users and developers")
args_internal = parser.add_argument_group("Cato controls", "Options to influence the semantical behaviour of CATO")
args_compilation = parser.add_argument_group("Compiler flags", "Set compiler flags, which are used to build input application")

# ------------------------------- Feedback args ------------------------------ #

args_feedback.add_argument(
    "-l",
    "--logging",
    action="store_true",
    help="Enables logging for the produced binary",
)

args_feedback.add_argument("--debug-pm", action="store_true", help="Pass `--debug-pass-manager` flag to opt")
args_feedback.add_argument("--debug", action="store_true", help="Pass `--debug` flag to opt")
args_feedback.add_argument("--verbose", action="store_true", help="Print calls to build final executable")

# ---------------------------- CATO control flags ---------------------------- #

args_internal.add_argument("--disable-openmp", action="store_true", help="Disable detection and insertion of OpenMP in original code")


# ------------------------------ Compiler Flags ------------------------------ #

args_compilation.add_argument("-cxx-flags", action=)



cxx_flags = "-O2 -g0 -fopenmp -Wunknown-pragmas"
pass_dir = Path(os.environ["CATO_ROOT"]) / Path("src/build/cato/libCatoPass.so")
rtlib_dir = Path(os.environ["CATO_ROOT"]) / Path("src/build/cato/rtlib")

# ---------------------------------------------------------------------------- #
#                               Evaluate options                               #
# ---------------------------------------------------------------------------- #
arguments = parser.parse_args()

logging = "-mllvm --cato-logging" if arguments.logging else ""
debug_pm = "--debug-pass-manager" if arguments.debug_pm else ""
debug = "--debug" if arguments.debug else ""


# ---------------------------------------------------------------------------- #
#                             Execute build process                            #
# ---------------------------------------------------------------------------- #


create_ir_cmd = f"opt {cxx_flags} -o {arguments.output.with_suffix('.ll')} -Xclang -load-pass-plugin -Xclang {pass_dir} {LOGGING} -S -emit-llvm"
-load-pass-plugin ${PASS}.so -passes=${PASS%_pass} ${TARGET}.ll -S -o ${TARGET}_modified.ll"
compile_cmd = f"mpicc -cc=clang {cxx_flags} -o {arguments.output.with_suffix('.o')} -Xclang -load-pass-plugin -Xclang {pass_dir} {LOGGING} -c {arguments.infile}"
# create_ir_cmd = f"mpicc -cc=clang {cxx_flags} -o {arguments.output.with_suffix('.ll')} -Xclang -load-pass-plugin -Xclang {pass_dir} {LOGGING} -S -emit-llvm"
link_cmd = f"mpicc -cc=clang {cxx_flags} -o {arguments.output} {arguments.output.with_suffix('.o')}  {rtlib_dir/Path('libCatoRuntime.so')} -Wl,-rpath,{rtlib_dir}"
rm_cmd = f"rm {arguments.output.with_suffix('.o')}"

run_command(compile_cmd)
run_command(create_ir_cmd)
run_command(link_cmd)
run_command(rm_cmd)
