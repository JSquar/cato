/*
 * File: debug.h
 * Created: Thursday, 16th February 2023 9:34:07 am
 * Author: Jannek Squar (jannek.squar@uni-hamburg.de)
 * -----
 *
 * -----
 * Last Modified: Friday, 17th February 2023 12:20:58 am
 * Modified By: Jannek Squar (jannek.squar@uni-hamburg.de)
 * -----
 * Copyright (c) 2019 Tim Jammer
 * Copyright (c) 2020 Michael Blesel
 * Copyright (c) 2023 Jannek Squar
 *
 */
/**
 * Include this if you want to use the Debug macro
 * All errs() and dump() statements that are not explicit Error or Warning messages
 * should be wrapped inside this macro.
 * Especially all non Error message related output from runtime library functions
 * should use this macro to not interfere with the test-suite, which expects program
 * output without debug messages.
 *
 * Each Use of the macro should have a semicolon at the end because the clang-format
 * tool produces bad results without this.
 *
 * Example use:
 *  Debug(errs() << "This is a debug message\n");
 *  Debug(value->dump());
 */

#include <llvm/Support/raw_ostream.h>
#include <string>

#ifndef CATO_DEBUG_H
#define CATO_DEBUG_H

#define DEBUG_CATO_PASS 0

#if DEBUG_CATO_PASS == 1
#define Debug(x) x
#else
#define Debug(x)
#endif

// void check_error_code(int, const std::string &);

#endif
