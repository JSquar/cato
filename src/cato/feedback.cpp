/*
 * File: UserControl.cpp
 * Created: Wednesday, 26th April 2023 9:52:47 am
 * Author: Jannek Squar (jannek.squar@uni-hamburg.de)
 * -----
 * 
 * -----
 * Last Modified: Sunday, 30th April 2023 5:53:44 pm
 * Modified By: Jannek Squar (jannek.squar@uni-hamburg.de)
 * -----
 * Copyright (c) 2023 Jannek Squar
 * 
 */
#include "feedback.h"
#include <llvm/Support/raw_ostream.h>

void print_usage() {
  llvm::errs() << "################################\n";
  llvm::errs() << "Available environment variables to control CATO\n\n";
  
  llvm::errs() << "OpenMP:\n";
  
  llvm::errs() << "netCDF:\n";
  llvm::errs() << "\tCATO_NC_PAR_MODE";
  llvm::errs() << "\tChoose collective mode for parallel netCDF4. Available options are:\n";
  llvm::errs() << "\t\tCOLLECTIVE\tIO is performed collectively by all processes (potential better I/O performance)\n";
  llvm::errs() << "\t\tINDEPENDENT\tEach process can independently perform I/O (not recommend, currently no use case in replacement code)\n";
  llvm::errs() << "\tCATO_NC_DEFLATE TODO";

  llvm::errs() << "################################\n\n";
  llvm::errs() << "There are continuative hints for the following topics:\n";
  llvm::errs() << "MPI:\t (Set CATO_HELP_MPI environment variable)\n";
  llvm::errs() << "netCDF:\t (Set CATO_HELP_NETCDF environment variable)\n";
  llvm::errs() << "Lustre:\t (Set CATO_HELP_LUSTRE environment variable)\n";
}

void print_hints_lustre() {
  llvm::errs() << "TODO: Hinweise zu Lustre (insb. striping mit lfs getstripe/setstripe sowie generelle Hinweise (beachte default Lustre chunk size))\n";
}

void print_hints_mpi() {
  llvm::errs() << "TODO: Verweis auf MPI-3.1 one-sided operations sowie Beispiel-Repos\n";
}

void print_hints_netcdf() {
  llvm::errs() << "Tipps bzgl. alignment, lossy und lossless compression\n";
}