#include "CatoRuntimeLogger.h"

#include <cstdlib>
#include <filesystem>
#include <mpi.h>

CatoRuntimeLogger *_logger = nullptr;

CatoRuntimeLogger::CatoRuntimeLogger()
{
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (!std::filesystem::exists("./logs"))
    {
        std::filesystem::create_directory("./logs");
    }
    std::string file_path = {"./logs/cato_log_proc" + std::to_string(rank)};
    _log_file.open(file_path, std::ios::out | std::ios::trunc);
}

CatoRuntimeLogger::~CatoRuntimeLogger() { _log_file.close(); }

CatoRuntimeLogger *CatoRuntimeLogger::get_logger() { return _logger; }

CatoRuntimeLogger *CatoRuntimeLogger::start_logger()
{
    if (_logger == nullptr)
    {
        _logger = new CatoRuntimeLogger();
    }
    return _logger;
}

void CatoRuntimeLogger::stop_logger()
{
    if (_logger != nullptr)
    {
        delete _logger;
    }
}

CatoRuntimeLogger &CatoRuntimeLogger::operator<<(const std::string &message)
{
    _log_file << message << "\n";
    return *this;
}
