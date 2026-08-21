/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include <RootCoreUtils/Assert.h>
#include <RootCoreUtils/StringUtil.h>
#include <stdexcept>
#include <string>
#include <string_view>
#include <regex>

namespace RCU
{
  std::string substitute(std::string_view str, std::string_view pattern,
                         std::string_view with)
  {
    if (pattern.empty())
      throw std::runtime_error ("substitute: pattern must not be empty");

    std::string result(str);
    std::string::size_type pos = 0;
    
    while ((pos = result.find(pattern, pos)) != std::string::npos) {
      result.replace(pos, pattern.size(), with);
      pos += with.size();
    }
    return result;
  }

  bool match_expr(const std::regex& expr, std::string_view str) 
  {
    return std::regex_match(str.begin(), str.end(), expr);
  }

  std::string glob_to_regexp(std::string_view glob)
  {
    std::string result;
    result.reserve(glob.size() * 2);

    for (char c : glob)
    {
      switch (c) 
      {
        case '*': 
          result += ".*"; 
          break;
        case '?': 
          result += '.'; 
          break;
        case '^': case '$': case '+': case '.': case '\\': 
        case '(': case ')': case '[': case ']': case '{': case '}': case '|':
          result += '\\';
          result += c;
          break;
        default:
          result += c;
          break;
      }
    }
    return result;
  }
}