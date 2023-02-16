/*
 * File: debug.cpp
 * Created: Thursday, 16th February 2023 10:10:31 am
 * Author: Jannek Squar (jannek.squar@uni-hamburg.de)
 * -----
 *
 * -----
 * Last Modified: Thursday, 16th February 2023 10:51:20 am
 * Modified By: Jannek Squar (jannek.squar@uni-hamburg.de)
 * -----
 * Copyright (c) 2023 Jannek Squar
 *
 */
#include "debug.h"

void check_error_code(int err, const std::string &location)
{
    if (err)
    {
        llvm::errs() << "Error in " << location << " with error code: " << err << "\n";
    }
}
