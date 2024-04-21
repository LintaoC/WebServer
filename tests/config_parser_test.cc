#include "gtest/gtest.h"
#include "config_parser.h"

class NginxConfigParserTest : public ::testing::Test {
protected:
  NginxConfigParser parser;
  NginxConfig out_config;
};

TEST_F(NginxConfigParserTest, SimpleConfig) {

  bool success = parser.Parse("example_config", &out_config);

  EXPECT_TRUE(success);
}

//test for the case when the file does not exist
TEST_F(NginxConfigParserTest, OpenNonExistentFile) {

  bool success = parser.Parse("none_config", &out_config);

  EXPECT_FALSE(success);
}

//test for the case when the file is empty
TEST_F(NginxConfigParserTest, EmptyConfig) {

  bool success = parser.Parse("empty_config", &out_config);

  EXPECT_TRUE(success);
}

//test when missing left bracket
TEST_F(NginxConfigParserTest, MissLeftBracket) {

  bool success = parser.Parse("miss_left_brac", &out_config);

  EXPECT_FALSE(success);
}

//test when missing right bracket
TEST_F(NginxConfigParserTest, MissRightBracket) {

  bool success = parser.Parse("miss_right_brac", &out_config);

  EXPECT_FALSE(success);
}

//test when extra left bracket
TEST_F(NginxConfigParserTest, ExtraLeftBracket) {

  bool success = parser.Parse("extra_left_brac", &out_config);

  EXPECT_FALSE(success);
}

//test when extra right bracket
TEST_F(NginxConfigParserTest, ExtraRightBracket) {

  bool success = parser.Parse("extra_right_brac", &out_config);

  EXPECT_FALSE(success);
}

