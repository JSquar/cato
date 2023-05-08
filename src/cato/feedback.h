/*
 * File: Feedback.h
 * Created: Wednesday, 26th April 2023 9:54:29 am
 * Author: Jannek Squar (jannek.squar@uni-hamburg.de)
 * -----
 * 
 * -----
 * Last Modified: Friday, 5th May 2023 2:15:40 pm
 * Modified By: Jannek Squar (jannek.squar@uni-hamburg.de)
 * -----
 * Copyright (c) 2023 Jannek Squar
 * 
 */
#ifndef FEEDBACK
#define FEEDBACK

/*
 * File: UserControl.cpp
 * Created: Wednesday, 26th April 2023 9:52:47 am
 * Author: Jannek Squar (jannek.squar@uni-hamburg.de)
 * -----
 * 
 * -----
 * Last Modified: Friday, 5th May 2023 2:14:42 pm
 * Modified By: Jannek Squar (jannek.squar@uni-hamburg.de)
 * -----
 * Copyright (c) 2023 Jannek Squar
 * 
 */
#include "feedback.h"
#include <llvm/Support/raw_ostream.h>

bool check_usage_requests();

void print_general_usage();

void print_usage_netcdf();

void print_hints_lustre();

void print_hints_mpi();

void print_hints_netcdf();

#endif /* FEEDBACK */
