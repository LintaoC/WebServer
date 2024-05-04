#include "gtest/gtest.h"
#include "config_parser.h"

class NginxConfigParserTest : public ::testing::Test
{
protected:
  NginxConfigParser parser;
  NginxConfig out_config;
};

TEST_F(NginxConfigParserTest, SimpleConfig)
{

  bool success = parser.Parse("example_config", &out_config);

  EXPECT_TRUE(success);
}

// test for the case when the file does not exist
TEST_F(NginxConfigParserTest, OpenNonExistentFile)
{

  bool success = parser.Parse("none_config", &out_config);

  EXPECT_FALSE(success);
}

// test for the case when the file is empty
TEST_F(NginxConfigParserTest, EmptyConfig)
{

  bool success = parser.Parse("empty_config", &out_config);

  EXPECT_TRUE(success);
}

// test when missing left bracket
TEST_F(NginxConfigParserTest, MissLeftBracket)
{

  bool success = parser.Parse("miss_left_brac", &out_config);

  EXPECT_FALSE(success);
}

// test when missing right bracket
TEST_F(NginxConfigParserTest, MissRightBracket)
{

  bool success = parser.Parse("miss_right_brac", &out_config);

  EXPECT_FALSE(success);
}

// test when extra left bracket
TEST_F(NginxConfigParserTest, ExtraLeftBracket)
{

  bool success = parser.Parse("extra_left_brac", &out_config);

  EXPECT_FALSE(success);
}

// test when extra right bracket
TEST_F(NginxConfigParserTest, ExtraRightBracket)
{

  bool success = parser.Parse("extra_right_brac", &out_config);

  EXPECT_FALSE(success);
}

TEST_F(NginxConfigParserTest, NestLoopConfig)
{
  bool success = parser.Parse("nest_loop", &out_config);
  EXPECT_TRUE(success);
}

TEST_F(NginxConfigParserTest, InvalidLeftParenthesisConfig)
{
  bool success = parser.Parse("invalid_left_parenthesis_config", &out_config);
  EXPECT_FALSE(success);
}

TEST_F(NginxConfigParserTest, NginxConfigToString)
{
  bool success = parser.Parse("example_config_test", &out_config);                                                    // parse the config file
  std::string res = "foo bar;\nserver {\n  port 8080;\n  server_name foo.com;\n  root /home/ubuntu/sites/foo/;\n}\n"; // expected result
  std::string config_string = out_config.ToString();                                                                  // convert the config to string
  bool same = config_string.compare(res) == 0;                                                                        // compare the result with the expected result
  EXPECT_TRUE(same);
}

TEST_F(NginxConfigParserTest, NginxConfigToStringNested)
{
  bool success = parser.Parse("nested_loop_config", &out_config);                                           // parse the config file
  std::string res = "foo \"baaar\";\nserver {\n  location / {\n    proxy http://localhost:8080;\n  }\n}\n"; // expected result
  std::string config_string = out_config.ToString(0);                                                       // convert the config to string
  bool result = config_string.compare(res) == 0;                                                            // compare the result with the expected result
  EXPECT_TRUE(result);
}

TEST_F(NginxConfigParserTest, GetPortNumber)
{
  bool success = parser.Parse("port_number", &out_config); // parse the config file
  int port = out_config.GetPort();                         // get the port number from the config
  bool result = port == 80;                                // compare the result with the expected result
  EXPECT_TRUE(result);
}

TEST_F(NginxConfigParserTest, StatementEndError)
{
  bool success = parser.Parse("statement_end_error", &out_config);
  EXPECT_FALSE(success);
}

TEST_F(NginxConfigParserTest, GetPortNumberChildBlock)
{
  bool success = parser.Parse("example_config", &out_config);
  int port = out_config.GetPort();
  bool same = port == 8080;
  EXPECT_TRUE(same);
}

TEST_F(NginxConfigParserTest, GetHandlerTypeTest)
{
bool success = parser.Parse("example_config", &out_config);
std::string a = "/static1/a.txt";
std::string t = out_config.GetHandlerType(a);
bool same = t == "static";
EXPECT_TRUE(same);
}

TEST_F(NginxConfigParserTest, GetFilePathTest)
{
bool success = parser.Parse("example_config", &out_config);
std::string a = "/static1/a.txt";
std::string t = out_config.GetFilePath(a);
bool same = t == "../files1";
EXPECT_TRUE(same);
}
// tets when port is not found (return -1)
TEST_F(NginxConfigParserTest, Nullport)
{
  bool success = parser.Parse("empty_config", &out_config);
  int port = out_config.GetPort();
  bool result = port == -1;
  EXPECT_TRUE(result);
}

TEST_F(NginxConfigParserTest, SingleQuote)
{
  bool success = parser.Parse("single_quote", &out_config);
  EXPECT_TRUE(success);
}

TEST_F(NginxConfigParserTest, EofQuote)
{
  bool success = parser.Parse("EofQuote", &out_config);
  EXPECT_FALSE(success);
}
