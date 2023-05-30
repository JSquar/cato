/*
 * File: environment_interaction.h
 * Created: Wednesday, 26th April 2023 2:06:01 pm
 * Author: Jannek Squar (jannek.squar@uni-hamburg.de)
 * -----
 * 
 * -----
 * Last Modified: Tuesday, 23rd May 2023 11:17:58 pm
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

// Return string from env variable if it is set
std::optional<std::string> parse_env(const std::string &env_var_name);

// Parse environment variable into vector of strings
std::vector<std::string> parse_env_list(const std::string &);

// Parse environment variable into vector of ints
std::vector<unsigned int> parse_env_list_int(const std::string &);

// check if a string key is in a vector of strings and get the index
std::vector<std::string>::size_type
    check_string_in_vector(std::string, std::vector<std::string>);

// check if a string is in the first environment variable and get the value of
// the second environment variable at the same position
std::optional<std::string> get_paired_value(std::string, std::string,
                                            std::string);

// Check if env var is set and try to parse it into int
std::optional<std::size_t> parse_env_size_t(const std::string &);

#endif /* ENVIRONMENT_INTERACTION */
