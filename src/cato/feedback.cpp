/*
 * File: UserControl.cpp
 * Created: Wednesday, 26th April 2023 9:52:47 am
 * Author: Jannek Squar (jannek.squar@uni-hamburg.de)
 * -----
 * 
 * -----
 * Last Modified: Monday, 15th May 2023 5:40:41 pm
 * Modified By: Jannek Squar (jannek.squar@uni-hamburg.de)
 * -----
 * Copyright (c) 2023 Jannek Squar
 * 
 */
#include "feedback.h"
#include <llvm/Support/raw_ostream.h>

bool check_usage_requests() {
  bool help_requested = false;
  const char* env_cato_help = std::getenv("CATO_HELP");
  const char* env_cato_use_netcdf = std::getenv("CATO_USE_NETCDF");
  const char* env_cato_hint_netcdf = std::getenv("CATO_HINT_NETCDF");
  const char* env_cato_hint_mpi = std::getenv("CATO_HINT_MPI");
  const char* env_cato_hint_lustre = std::getenv("CATO_HINT_LUSTRE");
  
  if(env_cato_help) {
    print_general_usage();
  }

  if(env_cato_use_netcdf) {
    print_usage_netcdf();
  }

  if(env_cato_hint_netcdf) {
    print_hints_netcdf();
  }

  if(env_cato_hint_mpi) {
    print_hints_mpi();
  }

  if(env_cato_hint_lustre) {
    print_hints_lustre();
  }  

  if(env_cato_help || env_cato_use_netcdf || env_cato_hint_netcdf || env_cato_hint_mpi || env_cato_hint_lustre) {
    help_requested = true;
  }
  return help_requested;
}

void print_general_usage() {
  llvm::errs() << "################################\n";
  llvm::errs() << "CATO offers optional behaviour, which can be enabled by the user, because it depends on the use case if it is useful.\n";
  llvm::errs() << "There are instructions available for the following topics:\n\n";  
  llvm::errs() << "netCDF:\t (Set CATO_USE_NETCDF environment variable)\n";
  llvm::errs() << "\n################################\n\n";
  llvm::errs() << "There are continuative hints for the following topics:\n";
  llvm::errs() << "MPI:\t (Set CATO_HINT_MPI environment variable)\n";
  llvm::errs() << "netCDF:\t (Set CATO_HINT_NETCDF environment variable)\n";
  llvm::errs() << "Lustre:\t (Set CATO_HINT_LUSTRE environment variable)\n";
  llvm::errs() << "\n################################\n\n";
  llvm::errs() << "\nTODO: Verweis darauf, dass Asuführung sich ändert! Ist wichtig und sollte ganz oben stehen\n\n";
}

void print_usage_netcdf() {
  llvm::errs() << "Environment variables to enable optional netCDF features\n";
  llvm::errs() << "################################\n\n";
  llvm::errs() << "IO mode:\n";
  llvm::errs() << "\tCATO_NC_PAR_MODE\n";
  llvm::errs() << "\tChoose collective mode for parallel netCDF4. Available options are:\n";
  llvm::errs() << "\t\tCOLLECTIVE\tIO is performed collectively by all processes, which potentially offers better I/O performance (default)\n";
  llvm::errs() << "\t\tINDEPENDENT\tEach process can independently perform I/O (not recommend, currently no use case in replacement code)\n";
  llvm::errs() << "\n################################\n\n";
  llvm::errs() << "Compression\n";
  llvm::errs() << "\tCATO_NC_DEFLATE TODO";
  llvm::errs() << "\n################################\n\n";
  llvm::errs() << "Sanitizer:\n";
  llvm::errs() << "\tCATO_SANITIZER_NC_ERROR";
  llvm::errs() << "\tSet how additional error checks shall be performed. Available options are:\n";
  llvm::errs() << "\t\tWARN\tIf the error value of a netCDF function is not 0, a warning is printed (default)\n";
  llvm::errs() << "\t\tERROR\tIf the error value of a netCDF function is not 0, the application is terminated\n";
  llvm::errs() << "\t\tOFF\tResults of erreor checking are ignored\n";
  llvm::errs() << "\t\tDISABLE\tNo error checking functionality is added (needs to be set before the compilation!)\n";
}

void print_hints_lustre() {
  llvm::errs() << "TODO: Hinweise zu Lustre (insb. striping mit lfs getstripe/setstripe sowie generelle Hinweise (beachte default Lustre chunk size))\n";
}

void print_hints_mpi() {
  llvm::errs() << "TODO: Verweis auf MPI-3.1 one-sided operations sowie Beispiel-Repos\n";
  llvm::errs() << "TODO: Hinweis auf optimale Verwendung von Prozess-Zahl\n";
}

void print_hints_netcdf() {
  llvm::errs() << "Tipps bzgl. alignment, lossy und lossless compression\n";
  llvm::errs() << "Tipps aus Compression 101 (https://www.unidata.ucar.edu/blogs/developer/entry/netcdf_compression)\n";
  llvm::errs() << "Auszug mit den wichtigsten Punkten aus Compression 101\n";
  llvm::errs() << "Verweis auf interessante Kompressoren sowie zugehörige Links\n";
}

void print_usage_decompilation() {
  llvm::errs() << "Übersicht Tools: retdec, llvm-cbe, llvm2c";
  llvm::errs() << "Links zu Tools, kurze Tipps zur Verwendung";

}
// Genereller Hint-Aufbau:
// 1. Generelle Beschreibung
// 2. Verweis auf offizielle Doku
// 3. Evtl. Verwendungstipps
// 4. Verweis auf Code-Beispiele, wo man nachgucken kann