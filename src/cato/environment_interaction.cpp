/*
 * File: environment_interaction.cpp
 * Created: Wednesday, 26th April 2023 2:05:25 pm
 * Author: Jannek Squar (jannek.squar@uni-hamburg.de)
 * -----
 * 
 * -----
 * Last Modified: Wednesday, 26th April 2023 4:33:59 pm
 * Modified By: Jannek Squar (jannek.squar@uni-hamburg.de)
 * -----
 * Copyright (c) 2023 Jannek Squar
 * 
 */
#include <algorithm>
#include <iterator>
#include "environment_interaction.h"

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