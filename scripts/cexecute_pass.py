#!/usr/bin/env python3
# -*- coding:utf-8 -*-
###
#File: cexecute_pass.py
#Author: Michael Blesel
#-----
#
#-----
#Last Modified: Wednesday, 8th February 2023 10:58:07 am
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

def run_command(command, verbose=False, dry_run=False):
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
        print(command + "\n")

def check_ec(error_code, desc="NA", stop_on_error=False):
    msg=f"An error occured (EC: {error_code}) during phase '{desc}"
    if error_code:
        if stop_on_error:
            logging.error(msg)
            raise RuntimeError(msg)
        else:
            logging.warning(msg)


if __name__ == "__main__":


    parser = argparse.ArgumentParser(description="Compile the given program with CATO")
    parser.add_argument("input_file", type=Path, help="Input C file")
    parser.add_argument("-o", "--output", type=Path, default=Path.cwd() / Path("translated.x"), help="Name of output file")

    args_feedback = parser.add_argument_group("Feedback", "Flags do not alter internal behaviour of CATO but provide more output for users and developers")
    args_internal = parser.add_argument_group("CATO controls", "Options to influence the semantical behaviour of CATO")
    args_compilation = parser.add_argument_group("Compiler flags", "Set compiler flags, which are used to build input application")

    # ------------------------------- Feedback args ------------------------------ #

    args_feedback.add_argument("-l","--logging",action="store_true",help="Enable logging for the produced binary",
    )

    args_feedback.add_argument("--debug-pm", action="store_true", help="Pass `--debug-pass-manager` flag to opt")
    args_feedback.add_argument("--debug", action="store_true", help="Pass `--debug` flag to opt")
    # remove debug-pass flag, because it belongs to the legacy pass manager
    # args_feedback.add_argument("--debug-pass", choices=["Arguments","Structure","Executions","Details"], help="Additional debugging information regarding pass execution")
    args_feedback.add_argument("--verbose", action="store_true", help="Print calls to build final executable")

    # ---------------------------- CATO control flags ---------------------------- #

    args_internal.add_argument("--dry-run", action="store_true", help="Show only lines, which would be executed to build appliaction with CATO")
    args_internal.add_argument("--enable-openmp", action="store_true", help="Enable detection and insertion of OpenMP in original code")
    args_internal.add_argument("--enable-netcdf", action="store_true", help="Perform replacement of netCDF operations to enable parallel IO")

    # ------------------------------ Compiler Flags ------------------------------ #

    args_compilation.add_argument("--cflags", nargs="+", action="append", help="Add flags, which are prepended to CFLAGS environment variable")


    # ---------------------------------------------------------------------------- #
    #                               Evaluate options                               #
    # ---------------------------------------------------------------------------- #
    arguments = parser.parse_args()

    flag_logging = "--cato-logging" if arguments.logging else ""
    flag_debug_pm = "--debug-pass-manager" if arguments.debug_pm else ""
    flag_debug = "--debug" if arguments.debug else ""
    # flag_debug_pass = f"--debug-pass={arguments.debug_pass}" if arguments.debug_pass else ""


    # ------------------------------- Set variables ------------------------------ #

    cflags = "-O2 -g0 -fopenmp -Wunknown-pragmas"
    if "CFLAGS" in os.environ:
        cflags = cflags + " " + os.environ["CFLAGS"]
    if arguments.cflags:
        cflags = cflags + " " + " ".join(list(itertools.chain.from_iterable(arguments.cflags)))
    if not "openmp" in cflags:
            logging.warning("Openmp control flag in CATO has been enabled but not added to CFLAGS")
            
    pass_location = Path(os.environ["CATO_ROOT"]) / Path("src/") / Path("build") / Path("cato") / Path("libCatoPass.so")
    rtlib_location = Path(os.environ["CATO_ROOT"]) / Path("src/") / Path("build") / Path("cato") / Path("rtlib") / Path('libCatoRuntime.so')

    # ----------------------------- internal controls ---------------------------- #

    if arguments.enable_netcdf:
        logging.warning("netCDF replacement control flag has not been implemented yet")
    if arguments.enable_openmp:    
        logging.warning("openmp replacement control flag has not been implemented yet")


    # ---------------------------------------------------------------------------- #
    #                             Execute build process                            #
    # ---------------------------------------------------------------------------- #

    file_input = arguments.input_file.resolve()
    file_ir = file_input.with_suffix('.ll')
    file_ir_modified = file_ir.parent / Path(file_ir.stem + '_modified.ll')
    file_bc = file_ir_modified.with_suffix(".bc")
    file_output = arguments.output

    cmd_create_ir = f"mpicc -cc=clang -S -emit-llvm {cflags} {file_input} -o {file_ir}"
    cmd_create_modified_ir = f"opt -load-pass-plugin={pass_location} -passes=Cato {file_ir} -S -o {file_ir_modified}"
    cmd_create_modified_bc = f"llvm-as {file_ir_modified} -o {file_bc}"
    cmd_link = f"mpicc -cc=clang -o {file_output} {file_bc} {rtlib_location}"


    # PASS_PATH="${CATO_ROOT}/src/build/cato/libCatoPass.so"
    # RTLIB_PATH="${CATO_ROOT}/src/build/cato/rtlib/libCatoRuntime.so"

    # clang -S -emit-llvm -fopenmp ${1}.c
    # opt -load-pass-plugin=${PASS_PATH} -passes="Cato" ${1}.ll -S > ${1}_modified.ll
    # llvm-as ${1}_modified.ll -o ${1}_modified.bc
    # mpicc -cc=clang -o ${1}_modified.x ${1}_modified.bc ${RTLIB_PATH}

    ec = run_command(cmd_create_ir, arguments.verbose, arguments.dry_run)
    check_ec(ec, "Create IR", stop_on_error=True)

    ec = run_command(cmd_create_modified_ir, arguments.verbose, arguments.dry_run)
    check_ec(ec, "Modify IR", stop_on_error=True)

    ec = run_command(cmd_create_modified_bc, arguments.verbose, arguments.dry_run)
    check_ec(ec, "Create modified BC", stop_on_error=True)

    ec = run_command(cmd_link, arguments.verbose, arguments.dry_run)
    check_ec(ec, "Link final binary", stop_on_error=True)
