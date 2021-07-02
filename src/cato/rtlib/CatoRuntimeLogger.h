#ifndef CATO_RTLIB_CATO_RUNTIME_LOGGER_H
#define CATO_RTLIB_CATO_RUNTIME_LOGGER_H

#include <fstream>
#include <iostream>
#include <string>

/**
 *  Logging class that can be added to the translated program.
 *  Each MPI process has its own logging file in the src/logs/ directory.
 *
 *  Use example:
 *      if (auto *logger = CatoRuntimeLogger::get_logger())
 *      {
 *          std::string message = "TEST LOG";
 *          *logger << message;
 *      }
 *
 *  An instance of the logger is only added to the program if it is
 *  compiled with the --cato-logging option. Therefore each use
 *  of the logger should be inside an if statement that tests if there
 *  is an active logger like in the given example.
 */
class CatoRuntimeLogger
{
  private:
    CatoRuntimeLogger();

    ~CatoRuntimeLogger();

    std::ofstream _log_file;

  public:
    /**
     * Returns a pointer to the logger or nullptr if none has been created.
     * This should be used in an if statement check for nullptr before
     * doing any logging operations. If nullptr is returned this means
     * that logging was not enabled and therefore no instance of the logger
     * was created.
     **/
    static CatoRuntimeLogger *get_logger();

    /**
     * Creates an instance of the logger. A pointer to the logger
     * can be retrieved by calling the get_logger function.
     * This is called once for the program in the cato init block
     * if the program was compiled with --cato-logging.
     * If this compile argument is not given no logger will be created.
     **/
    static CatoRuntimeLogger *start_logger();

    /**
     * Deletes the instance of the logger.
     **/
    static void stop_logger();

    /**
     * Logs the given message in the logfile of the mpi process.
     * This function should only be called after testing
     * if there is an active logger by calling the get_logger method.
     **/
    CatoRuntimeLogger &operator<<(const std::string &message);
};

#endif
