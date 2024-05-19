// An nginx config file parser.
//
// See:
//   http://wiki.nginx.org/Configuration
//   http://blog.martinfjordvald.com/2010/07/nginx-primer/
//
// How Nginx does it:
//   http://lxr.nginx.org/source/src/core/ngx_conf_file.c

#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <stack>
#include <string>
#include <vector>
#include <map>
#include <boost/log/trivial.hpp>
#include <boost/log/attributes.hpp>
#include "RequestHandlerFactory.h"
#include "../include/config_parser.h"
#include <stdexcept>
#include <algorithm>

// constructs a string representation of the entire configuration
// It recursively calls ToString on each NginxConfigStatement within its statements_ vector,
// building a formatted text output that reflects the nested structure of the configuration.
std::string NginxConfig::ToString(int depth)
{
  std::string serialized_config;
  for (const auto &statement : statements_)
  {
    serialized_config.append(statement->ToString(depth));
  }
  return serialized_config;
}

std::map<std::string, RequestHandlerFactory*>* NginxConfig::getPathMap() const {
    auto* map = new std::map<std::string, RequestHandlerFactory*>; // Allocate map on the heap
    for (const auto& statement : statements_) {
        // Check if the statement is a 'location'
        if (statement->tokens_.size() > 1 && statement->tokens_[0] == "location") {
            try {
                std::string url = statement->tokens_[1]; // Find url

                // Ensure the URL starts with a '/'
                if (!url.empty() &&  url[0] != '/') {
                    url = '/' + url;
                }

                // Trim trailing slashes, but ensure not to remove the initial '/' for root paths
                while (url.size() > 1 && url.back() == '/') {
                    url.pop_back();
                }

                if (map->find(url) != map->end()) {
                    // If the URL already exists in the map, throw an exception indicating a configuration error
                    throw std::runtime_error("Duplicate location found in configuration: " + url);
                }

                std::string handlerType = statement->tokens_[2]; // Find type
                std::map<std::string, std::string> handler_params;
                if (statement->child_block_) {
                    handler_params = statement->child_block_->getParams();
                }
                RequestHandlerFactory* factory = new RequestHandlerFactory(handlerType, handler_params);
                (*map)[url] = factory;

            } catch (const std::invalid_argument &e) {
            }
        }
        // Recursively search in child blocks if present
        if (statement->child_block_) {
            // We merge the child map into the parent map
            std::map<std::string, RequestHandlerFactory*>* childMap = statement->child_block_->getPathMap();
            map->insert(childMap->begin(), childMap->end());
            delete childMap; // Clean up the child map to avoid memory leaks
        }
    }
    return map; // Return the pointer to the map
}


int NginxConfig::GetPort() const {
    for (const auto& statement : statements_) {
        // Check if the statement is a 'port'
        if (statement->tokens_.size() > 1 && statement->tokens_[0] == "port") {
            try {
                return std::stoi(statement->tokens_[1]);
            } catch (const std::invalid_argument &e) {
                BOOST_LOG_TRIVIAL(error) <<"Invalid port value: " << statement->tokens_[1]
                <<", using default 80 port."<< std::endl;
                return 80;
            }
        }
        // Recursively search in child blocks if present
        if (statement->child_block_) {
            int port = statement->child_block_->GetPort();
            if (port != -1) return port;
        }
    }
    BOOST_LOG_TRIVIAL(error) <<"No port specified, using default 80 port."<< std::endl;
    return 80; // Return default 80 if no valid port is found
}

std::map<std::string, std::string> NginxConfig::getParams() const {
    std::map<std::string, std::string> handler_params;
    for (const auto &statement: statements_) {
        auto tokens = statement->tokens_;

        // The argument has to be specified as a key-value pair
        if (tokens.size() != 2) {
            BOOST_LOG_TRIVIAL(fatal) << "Invalid handler argument";
            exit(1);
        }

        // Remove trailing slashes from the root path
        if (tokens[0] == "root") {
            while (!tokens[1].empty() && tokens[1].back() == '/') {
                tokens[1].pop_back();
            }
        }

        handler_params[tokens[0]] = tokens[1];
    }
    return handler_params;
}


// Generates a string for a single configuration statement.
// It handles both simple statements (like key-value pairs) and blocks (statements that contain child blocks).
// The method formats the statement with proper indentation based on its depth in the configuration hierarchy.
std::string NginxConfigStatement::ToString(int depth)
{
  std::string serialized_statement;
  for (int i = 0; i < depth; ++i)
  {
    serialized_statement.append("  ");
  }
  for (unsigned int i = 0; i < tokens_.size(); ++i)
  {
    if (i != 0)
    {
      serialized_statement.append(" ");
    }
    serialized_statement.append(tokens_[i]);
  }
  if (child_block_.get() != nullptr)
  {
    serialized_statement.append(" {\n");
    serialized_statement.append(child_block_->ToString(depth + 1));
    for (int i = 0; i < depth; ++i)
    {
      serialized_statement.append("  ");
    }
    serialized_statement.append("}");
  }
  else
  {
    serialized_statement.append(";");
  }
  serialized_statement.append("\n");
  return serialized_statement;
}


// Converts the TokenType enumeration to a human-readable string.
const char *NginxConfigParser::TokenTypeAsString(TokenType type)
{
  switch (type)
  {
  case TOKEN_TYPE_START:
    return "TOKEN_TYPE_START";
  case TOKEN_TYPE_NORMAL:
    return "TOKEN_TYPE_NORMAL";
  case TOKEN_TYPE_START_BLOCK:
    return "TOKEN_TYPE_START_BLOCK";
  case TOKEN_TYPE_END_BLOCK:
    return "TOKEN_TYPE_END_BLOCK";
  case TOKEN_TYPE_COMMENT:
    return "TOKEN_TYPE_COMMENT";
  case TOKEN_TYPE_STATEMENT_END:
    return "TOKEN_TYPE_STATEMENT_END";
  case TOKEN_TYPE_EOF:
    return "TOKEN_TYPE_EOF";
  case TOKEN_TYPE_ERROR:
    return "TOKEN_TYPE_ERROR";
  default:
    return "Unknown token type";
  }
}

// The actual lexical analysis (tokenization) of the configuration text occurs.
// It reads characters from the input stream and classifies them into different
//    token types based on their syntax and context (e.g., start of a block, end of a block, normal text, comments).
NginxConfigParser::TokenType NginxConfigParser::ParseToken(std::istream *input, std::string *value)
{
  TokenParserState state = TOKEN_STATE_INITIAL_WHITESPACE;
  while (input->good())
  {
    const char c = input->get();
    if (!input->good())
    {
      break;
    }
    switch (state)
    {
    case TOKEN_STATE_INITIAL_WHITESPACE:
      switch (c)
      {
      case '{':
        *value = c;
        return TOKEN_TYPE_START_BLOCK;
      case '}':
        *value = c;
        return TOKEN_TYPE_END_BLOCK;
      case '#':
        *value = c;
        state = TOKEN_STATE_TOKEN_TYPE_COMMENT;
        continue;
      case '"':
        *value = c;
        state = TOKEN_STATE_DOUBLE_QUOTE;
        continue;
      case '\'':
        *value = c;
        state = TOKEN_STATE_SINGLE_QUOTE;
        continue;
      case ';':
        *value = c;
        return TOKEN_TYPE_STATEMENT_END;
      case ' ':
      case '\t':
      case '\n':
      case '\r':
        continue;
      default:
        *value += c;
        state = TOKEN_STATE_TOKEN_TYPE_NORMAL;
        continue;
      }
    case TOKEN_STATE_SINGLE_QUOTE:
      *value += c;
      if (c == '\'')
      {
        return TOKEN_TYPE_NORMAL;
      }
      continue;
    case TOKEN_STATE_DOUBLE_QUOTE:
      *value += c;
      if (c == '"')
      {
        return TOKEN_TYPE_NORMAL;
      }
      continue;
    case TOKEN_STATE_TOKEN_TYPE_COMMENT:
      if (c == '\n' || c == '\r')
      {
        return TOKEN_TYPE_COMMENT;
      }
      *value += c;
      continue;
    case TOKEN_STATE_TOKEN_TYPE_NORMAL:
      if (c == ' ' || c == '\t' || c == '\n' || c == '\t' ||
          c == ';' || c == '{' || c == '}')
      {
        input->unget();
        return TOKEN_TYPE_NORMAL;
      }
      *value += c;
      continue;
    }
  }

  // If we get here, we reached the end of the file.
  if (state == TOKEN_STATE_SINGLE_QUOTE ||
      state == TOKEN_STATE_DOUBLE_QUOTE)
  {
    return TOKEN_TYPE_ERROR;
  }

  return TOKEN_TYPE_EOF;
}

// Orchestrates the parsing process using a stack to handle nested configurations.
bool NginxConfigParser::Parse(std::istream *config_file, NginxConfig *config)
{
  // stack used to handle the nested configuration blocks
  std::stack<NginxConfig *> config_stack;
  // config is pushed onto the stack to start the process
  config_stack.push(config);
  TokenType last_token_type = TOKEN_TYPE_START;
  TokenType token_type;
  while (true)
  {
    std::string token;
    token_type = ParseToken(config_file, &token); // Each iteration involves calling ParseToken to get the next token and its type.
    printf("%s: %s\n", TokenTypeAsString(token_type), token.c_str());
    // If we encounter an error, we stop parsing.
    if (token_type == TOKEN_TYPE_ERROR)
    {
      printf("the token_type is error");
      break;
    }
    // If we encounter a comment, we skip it.
    if (token_type == TOKEN_TYPE_COMMENT)
    {
      // Skip comments.
      continue;
    }
    // error if the token type is not expected
    if (token_type == TOKEN_TYPE_START)
    {
      printf("the token_type is start, which is not expected");
      break;
    }
    else if (token_type == TOKEN_TYPE_NORMAL)
    {
      if (last_token_type == TOKEN_TYPE_START ||
          last_token_type == TOKEN_TYPE_STATEMENT_END ||
          last_token_type == TOKEN_TYPE_START_BLOCK ||
          last_token_type == TOKEN_TYPE_END_BLOCK ||
          last_token_type == TOKEN_TYPE_NORMAL)
      {
        // If the last token was not a normal token, we create a new NginxConfigStatement.
        if (last_token_type != TOKEN_TYPE_NORMAL)
        {
          config_stack.top()->statements_.emplace_back(
              new NginxConfigStatement);
        }
        // if the last token was a normal token, we add the token to the last NginxConfigStatement.
        config_stack.top()->statements_.back().get()->tokens_.push_back(token);
      }
      else
      {
        // Error.
        printf("the token_type is normal, the last_token type is either EOF/Error/comment");
        break;
      }
    }
    else if (token_type == TOKEN_TYPE_STATEMENT_END)
    {
      if (last_token_type != TOKEN_TYPE_NORMAL)
      {
        // Error.
        printf("the token_type is statement_end, the last_token type is not normal");
        break;
      }
    }
    else if (token_type == TOKEN_TYPE_START_BLOCK)
    {
      if (last_token_type != TOKEN_TYPE_NORMAL)
      {
        // Error.
        break;
      }
      NginxConfig *const new_config = new NginxConfig;
      config_stack.top()->statements_.back().get()->child_block_.reset(new_config);
      config_stack.push(new_config);
    }
    else if (token_type == TOKEN_TYPE_END_BLOCK)
    {
      // if (last_token_type != TOKEN_TYPE_STATEMENT_END)
      // {
        // FIX:  if last token is TOKEN_TYPE_END_BLOCK, which is nested block
      //   if (last_token_type == TOKEN_TYPE_END_BLOCK)
      //   {
      //     config_stack.pop();
      //     continue;
      //   }
      //   // Error.
      //   else
      //   {
      //     std::cout << "last_token_type: " << TokenTypeAsString(last_token_type) << std::endl;
      //     printf("the token_type is end_block, the last_token type is not statement_end... ");
      //     break;
      //   }
      // }
      if (config_stack.size() == 1) {
        // Error.
        break;
      }
      config_stack.pop();
    }
    else if (token_type == TOKEN_TYPE_EOF)
    {
      if (config_stack.size() != 1) {
        // Error.
        break;
      }
      if (last_token_type != TOKEN_TYPE_STATEMENT_END &&
          last_token_type != TOKEN_TYPE_END_BLOCK)
      {
        // FIX: if its a empty file, return
        if (last_token_type == TOKEN_TYPE_START)
        {
          return true;
        }
        // Error.
        printf("the token_type is eof, the last_token type is not statement_end or end_block");
        break;
      }
      return true;
    }
    else
    {
      // Error. Unknown token.
      printf("the token_type is unknown");
      break;
    }
    last_token_type = token_type;
  }

  printf("Bad transition from %s to %s\n",
         TokenTypeAsString(last_token_type),
         TokenTypeAsString(token_type));
  return false;
}

bool NginxConfigParser::Parse(const char *file_name, NginxConfig *config)
{
  std::ifstream config_file;
  config_file.open(file_name);
  if (!config_file.good())
  {
    printf("Failed to open config file: %s\n", file_name);
    return false;
  }

  const bool return_value =
      Parse(dynamic_cast<std::istream *>(&config_file), config);
  config_file.close();
  return return_value;
}
