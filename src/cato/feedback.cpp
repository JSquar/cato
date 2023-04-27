/*
 * File: UserControl.cpp
 * Created: Wednesday, 26th April 2023 9:52:47 am
 * Author: Jannek Squar (jannek.squar@uni-hamburg.de)
 * -----
 * 
 * -----
 * Last Modified: Wednesday, 26th April 2023 4:34:52 pm
 * Modified By: Jannek Squar (jannek.squar@uni-hamburg.de)
 * -----
 * Copyright (c) 2023 Jannek Squar
 * 
 */
#include "feedback.h"
#include <llvm/Support/raw_ostream.h>

void print_usage() {
  llvm::errs() << "################################";
  llvm::errs() << "Available environment variables to control CATO\n\n";
  
  llvm::errs() << "OpenMP:\n";
  
  llvm::errs() << "netCDF:\n";
  llvm::errs() << "\tCATO_NC_PAR_MODE";
  llvm::errs() << "\tChoose collective mode for parallel netCDF4. Available options are:\n";
  llvm::errs() << "\t\tCOLLECTIVE\tIO is performed collectively by all processes (potential better I/O performance)\n";
  llvm::errs() << "\t\tINDEPENDENT\tEach process can independently perform I/O (not recommend, currently no use case in replacement code)\n";
  llvm::errs() << "\tCATO_NC_DEFLATE TODO";

}