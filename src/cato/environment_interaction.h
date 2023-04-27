/*
 * File: environment_interaction.h
 * Created: Wednesday, 26th April 2023 2:06:01 pm
 * Author: Jannek Squar (jannek.squar@uni-hamburg.de)
 * -----
 * 
 * -----
 * Last Modified: Wednesday, 26th April 2023 2:06:01 pm
 * Modified By: Jannek Squar (jannek.squar@uni-hamburg.de)
 * -----
 * Copyright (c) 2023 Jannek Squar
 * 
 */
#ifndef ENVIRONMENT_INTERACTION
#define ENVIRONMENT_INTERACTION

#include <optional>
#include <string>
#include <vector>

// Parse environment variable into vector of strings
std::vector<std::string> parse_env_list(const std::string &);

// check if a string key is in a vector of strings and get the index
std::vector<std::string>::size_type
    check_string_in_vector(std::string, std::vector<std::string>);

// check if a string is in the first environment variable and get the value of
// the second environment variable at the same position
std::optional<std::string> get_paired_value(std::string, std::string,
                                            std::string);

#endif /* ENVIRONMENT_INTERACTION */
