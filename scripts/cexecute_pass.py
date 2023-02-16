#!/usr/bin/python
import argparse
import os
import shlex
import subprocess


def run_command(command):
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

CXXFLAGS = "-O2 -g0 -fopenmp -Wunknown-pragmas"
PASS_PATH = os.environ["CATO_ROOT"] + "/src/build/cato/libCatoPass.so"
RTLIB_DIR = os.environ["CATO_ROOT"] + "/src/build/cato/rtlib"
LOGGING = ""
if(arguments.logging):
    LOGGING = "-mllvm --cato-logging"

compile_cmd = "mpicc -cc=clang " + CXXFLAGS + " -o " + arguments.output + ".o " + \
        "-Xclang -load -Xclang " + PASS_PATH + " " + LOGGING + " -c " + arguments.infile + " -flegacy-pass-manager"

link_cmd = "mpicc -cc=clang " + CXXFLAGS + " -o " + arguments.output + " " + \
        arguments.output + ".o " + RTLIB_DIR + "/libCatoRuntime.so " + RTLIB_DIR + "/libCatoRuntime_IO.so " + "-Wl,-rpath," + RTLIB_DIR

rm_cmd = "rm " + arguments.output + ".o"

run_command(compile_cmd)
run_command(link_cmd)
run_command(rm_cmd)



