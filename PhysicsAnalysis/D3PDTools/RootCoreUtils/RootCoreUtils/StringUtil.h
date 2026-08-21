/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


#ifndef ROOT_CORE_UTILS__STRING_UTIL_H
#define ROOT_CORE_UTILS__STRING_UTIL_H

#include <regex>
#include <string>

namespace RCU
{
  /// effects: substitute all occurences of "pattern" with "with" in
  ///   the string "str"
  /// returns: the substituted string
  /// guarantee: out of memory II
  /// requires: !pattern.empty()
  std::string substitute(std::string_view str, std::string_view pattern,
                         std::string_view with);

  /// returns: whether we can match the entire string with the regular
  ///   expression
  /// guarantee: strong
  /// failures: out of memory II
  bool match_expr(const std::regex& expr, std::string_view str);


  /// returns: a string that is the regular expression equivalent of
  ///   the given glob expression
  /// guarantee: strong
  /// failures: out of memory II
  std::string glob_to_regexp(std::string_view glob);
}

#endif
