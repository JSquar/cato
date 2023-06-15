/*
 * File: test.cpp
 * Created: Wednesday, 26th April 2023 10:09:34 am
 * Author: Jannek Squar (jannek.squar@uni-hamburg.de)
 * -----
 *
 * -----
 * Last Modified: Monday, 22nd May 2023 2:00:22 pm
 * Modified By: Jannek Squar (jannek.squar@uni-hamburg.de)
 * -----
 * Copyright (c) 2023 Jannek Squar
 *
 */
#include "environment_interaction.h" // include the header file for your function
#include <gtest/gtest.h> // include the Google Test framework
#include <cstdlib>


TEST(EnvParseTest, parse_vector_0) {
  putenv("FOO=hallo:welt");
  auto result = parse_env_list("FOO");
  EXPECT_EQ(result.size(),2);
}

TEST(EnvParseTest, parse_vector_1) {
  putenv("FOO=hallo:welt");
  std::vector<std::string> expected = {"hallo","welt"};
  auto result = parse_env_list("FOO");
  EXPECT_EQ(expected, result);
}

TEST(EnvParseTest, parse_vector_2) {
  putenv("FOO=hallowelt");
  std::vector<std::string> expected = {"hallowelt"};
  auto result = parse_env_list("FOO");
  EXPECT_EQ(result.size(),1);
  EXPECT_EQ(expected, result);
}

TEST(EnvParseTest, parse_vector_int) {
  putenv("FOO=1:2:3:4");
  auto result = parse_env_list_int("FOO");
  EXPECT_EQ(result.size(),4);
  int result_sum = result.at(0) + result.at(1) + result.at(2) + result.at(3);
  EXPECT_EQ(10, result_sum);
}

TEST(EnvParseTest, get_index_exists) {
  putenv("FOO=hallo:welt:hello:world");
  auto result = parse_env_list("FOO");
  std::optional<std::vector<std::string>::size_type> index = check_string_in_vector("hello", result);
  EXPECT_TRUE(index.has_value());
  EXPECT_EQ(2,index.value());
}

TEST(EnvParseTest, get_index_not_exists) {
  putenv("FOO=hallo:welt:hello:world");
  auto result = parse_env_list("FOO");
  std::optional<std::vector<std::string>::size_type> index = check_string_in_vector("bonjour", result);
  EXPECT_FALSE(index.has_value());
}

TEST(EnvParseTest, get_pair_exists) {
  putenv("FOO=hallo:welt:hello:world");
  putenv("BAR=1:2::");
  std::optional<std::string> result = get_paired_value("FOO", "BAR","welt");
  EXPECT_TRUE(result.has_value());
  EXPECT_EQ("2",result.value());
}

TEST(EnvParseTest, get_pair_not_exists) {
  putenv("FOO=hallo:welt:hello:world");
  putenv("BAR=1:2::");
  std::optional<std::string> result = get_paired_value("FOO", "BAR","bla");
  EXPECT_FALSE(result.has_value());
}

TEST(EnvParseTest, get_pair_empty) {
  putenv("FOO=hallo:welt:hello:world");
  putenv("BAR=1:2::");
  std::optional<std::string> result = get_paired_value("FOO", "BAR","hello");
  EXPECT_TRUE(result.has_value());
  EXPECT_EQ("",result.value());
}

TEST(EnvParseTest, parse_size_t_not_exist) {
  putenv("FOO=42");
  std::optional<std::size_t> result = parse_env_size_t("BAR_UNDEFINED");
  EXPECT_FALSE(result.has_value()); 
}

TEST(EnvParseTest, parse_size_t_exists) {
  putenv("FOO=42");
  std::optional<std::size_t> result = parse_env_size_t("FOO");
  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(42,result.value());
}

TEST(EnvParseTest, parse_env_not_exists) {
  putenv("FOO=42");
  std::optional<std::string> result = parse_env("BAR_UNDEFINED");
  EXPECT_FALSE(result.has_value()); 
}

TEST(EnvParseTest, parse_env_exists) {
  putenv("FOO=42");
  std::optional<std::string> result = parse_env("FOO");
  EXPECT_TRUE(result.has_value());
  EXPECT_EQ("42",result.value());
}


