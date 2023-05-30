/*
 * File: environment_interaction.cpp
 * Created: Wednesday, 26th April 2023 2:05:25 pm
 * Author: Jannek Squar (jannek.squar@uni-hamburg.de)
 * -----
 * 
 * -----
 * Last Modified: Tuesday, 23rd May 2023 11:18:08 pm
 * Modified By: Jannek Squar (jannek.squar@uni-hamburg.de)
 * -----
 * Copyright (c) 2023 Jannek Squar
 * 
 */
#include <algorithm>
#include <iterator>
#include "environment_interaction.h"
#include <iostream>
#include <sstream>

std::optional<std::string> parse_env(const std::string &env_var_name) {
  
  const char *env_var = std::getenv(env_var_name.c_str());
  if(env_var) {
    return env_var;
  }
  else {
    return std::nullopt;
  }
}

std::optional<std::size_t> parse_env_size_t(const std::string &env_var_name) {
  
  std::optional<std::string> env_var = parse_env(env_var_name);
  if(env_var.has_value()) {
    try {
      std::size_t result;
      std::stringstream stream(env_var.value());
      // size_t result = std::stoi(env_var);
      stream >> result;
      return result;
    }
    catch (...){
      std::cerr << "Error during parsing environmental variable " << env_var_name << "\n";
      return std::nullopt;
    }
  }
  else {
    return std::nullopt;
  }
}

std::vector<std::string> parse_env_list(const std::string &env_var_name) {
  std::vector<std::string> values;
  const char *env_var = std::getenv(env_var_name.c_str());
  std::string env_var_str = (env_var != nullptr) ? env_var : "";

  size_t pos = 0;
  std::string token;
  while ((pos = env_var_str.find(":")) != std::string::npos) {
    token = env_var_str.substr(0, pos);
    values.push_back(token);
    env_var_str.erase(0, pos + 1);
  }
  values.push_back(env_var_str);

  return values;
}

// TODO Use template
std::vector<unsigned int> parse_env_list_int(const std::string &env_var_name) {
  std::vector<unsigned int> values;
  const char *env_var = std::getenv(env_var_name.c_str());
  std::string env_var_str = (env_var != nullptr) ? env_var : "";

  size_t pos = 0;
  std::string token_string;
  int token_int;
  while ((pos = env_var_str.find(":")) != std::string::npos) {
    token_string = env_var_str.substr(0, pos);
    token_int = std::stoi(token_string);
    values.push_back(token_int);
    env_var_str.erase(0, pos + 1);
  }
  values.push_back(std::stoi(env_var_str));

  return values;
}

std::vector<std::string>::size_type
check_string_in_vector(std::string name, std::vector<std::string> values) {
  std::vector<std::string>::size_type index = -1;
  std::vector<std::string>::iterator iter =
      std::find(values.begin(), values.end(), name);
  if (iter != values.end()) {
    index = std::distance(values.begin(), iter);
  }

  return index;
}

std::optional<std::string> get_paired_value(std::string env_name_A,
                                            std::string env_name_B,
                                            std::string key) {
  std::string result;

  auto values_A = parse_env_list(env_name_A);
  auto values_B = parse_env_list(env_name_B);
  std::vector<std::string>::size_type index =
      check_string_in_vector(key, values_A);

  if (index == -1 || index >= values_B.size()) {
    return std::nullopt;
  }

  return values_B.at(index);
}