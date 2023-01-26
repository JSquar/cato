#!/usr/bin/env python3
# -*- coding:utf-8 -*-
###
#File: cexecute_pass.py
#Author: Michael Blesel
#-----
#
#-----
#Last Modified: Monday, 23rd January 2023 8:41:47 pm
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
import logging
import itertools

def run_command(command, dry_run=False):
    if not dry_run:
        process = subprocess.Popen(shlex.split(command), stdout=subprocess.PIPE)
        while True:
            output = process.stdout.readline().decode()
            if output == "" and process.poll() is not None:
                break
            if output:
                print(output.strip())
        rc = process.poll()
        return rc
    else:
        print(command)


parser = argparse.ArgumentParser(description="Compile the given program with CATO")
parser.add_argument("input_file", type=Path, help="Input C file")
parser.add_argument("-o", "--output", type=Path, default=Path("translated"), help="Name of output file")

args_feedback = parser.add_argument_group("Feedback", "Flags do not alter internal behaviour of CATO but provide more output for users and developers")
args_internal = parser.add_argument_group("CATO controls", "Options to influence the semantical behaviour of CATO")
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
# remove debug-pass flag, because it belongs to the legacy pass manager
# args_feedback.add_argument("--debug-pass", choices=["Arguments","Structure","Executions","Details"], help="Additional debugging information regarding pass execution")
args_feedback.add_argument("--verbose", action="store_true", help="Print calls to build final executable")

# ---------------------------- CATO control flags ---------------------------- #

args_internal.add_argument("--dry-run", action="store_true", help="Show only lines, which would be executed to build appliaction with CATO")
args_internal.add_argument("--enable-openmp", action="store_true", help="Enable detection and insertion of OpenMP in original code")
args_internal.add_argument("--enable-netcdf", action="store_true", help="Performs replacement of netCDF operations to enable parallel IO")

# ------------------------------ Compiler Flags ------------------------------ #

args_compilation.add_argument("--cxx-flags", nargs="+", action="append", help="Add flags, which are prepended to CXXFLAGS environment variable")


# ---------------------------------------------------------------------------- #
#                               Evaluate options                               #
# ---------------------------------------------------------------------------- #
arguments = parser.parse_args()

flag_logging = "--cato-logging" if arguments.logging else ""
flag_debug_pm = "--debug-pass-manager" if arguments.debug_pm else ""
flag_debug = "--debug" if arguments.debug else ""
# flag_debug_pass = f"--debug-pass={arguments.debug_pass}" if arguments.debug_pass else ""


# ------------------------------- Set variables ------------------------------ #

cxx_flags = "-O2 -g0 -fopenmp -Wunknown-pragmas"
if "CXXFLAGS" in os.environ:
    cxx_flags = cxx_flags + " " + os.environ["CXXFLAGS"]
if arguments.cxx_flags:
    cxx_flags = cxx_flags + " " + " ".join(list(itertools.chain.from_iterable(arguments.cxx_flags)))
pass_location = Path(os.environ["CATO_ROOT"]) / Path("src/build/cato/libCatoPass.so")
rtlib_dir = Path(os.environ["CATO_ROOT"]) / Path("src/build/cato/rtlib")

# ----------------------------- internal controls ---------------------------- #

if arguments.enable_netcdf:
    logging.warning("netCDF replacement control flag has not been implemented yet")
if arguments.enable_openmp:
    if not "openmp" in cxx_flags:
        logging.warning("Openmp control flag in CATO has been enabled but not added to CXXFLAGS")
    logging.warning("openmp replacement control flag has not been implemented yet")


# ---------------------------------------------------------------------------- #
#                             Execute build process                            #
# ---------------------------------------------------------------------------- #


cmd_create_ir = f"clang {cxx_flags} -o {arguments.output.with_suffix('.ll')} -S -emit-llvm {arguments.input_file}"
# cmd_modify_ir = f"opt {flag_debug_pm} {flag_debug} {arguments.output.with_suffix('.ll')} -o {arguments.output.parent / Path(arguments.output.stem + '_modified.ll')} -load-pass-plugin {pass_location} -passes=Cato -S"
cmd_modify_ir = f"mpicc -cc=clang {arguments.output.with_suffix('.ll')} -o {arguments.output.parent / Path(arguments.output.stem + '_modified.ll')} -Xclang -load-pass-plugin {pass_location} -Xclang -passes=Cato -S"
cmd_compile = f"mpicc -cc=clang {cxx_flags} -o {arguments.output.with_suffix('.o')} -Xclang -load-pass-plugin -Xclang {pass_location}  -c {arguments.input_file}"
# create_ir = f"mpicc -cc=clang {cxx_flags} -o {arguments.output.with_suffix('.ll')} -Xclang -load-pass-plugin -Xclang {pass_location} -S -emit-llvm"
cmd_link_cmd = f"mpicc -cc=clang {cxx_flags} -o {arguments.output} {arguments.output.with_suffix('.o')}  {rtlib_dir/Path('libCatoRuntime.so')} -Wl,-rpath,{rtlib_dir}"
cmd_rm = f"rm {arguments.output.with_suffix('.o')}"


run_command(cmd_create_ir, arguments.dry_run)
run_command(cmd_modify_ir, arguments.dry_run)
run_command(cmd_compile, arguments.dry_run)
# run_command(cmd_link,args.dry_run)
# run_command(cmd_rm,args.dry_run)
