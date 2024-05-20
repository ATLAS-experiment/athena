/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GENERATORUTILS_STRINGPARSE_H
#define GENERATORUTILS_STRINGPARSE_H
#include <string>
#include <vector>
#include <sstream>
#include <charconv>
#include <iostream>
#include <iomanip>

/// @brief Utility object for parsing a string into tokens and returning them as a variety of types
///
/// This code is used to parse a string, tokenising space-separated parts into component 'pieces'.
///
/// Methods can then be called to
///   1. Return the nth component as a string (piece method)
///   2. Return the nth component as an integer (intpiece and longpiece methods) if conversion is possible
///      (returns -1 if it is off the end and 0 if it cannot be converted)
///   3. Return the nth component as a double (numpiece method) if conversion is possible
///      (returns -1.1 if it is off the end and 0 if it cannot be converted)
///
/// Note that the 'num' index used to retrieve the pieces starts at 1, not the C-conventional 0.
///
/// @author Ian Hinchliffe, April 2000
/// @author Yun-Ha.Shin, June 2006
/// @author Andy Buckley, September 2012
/// @author Andrii Verbytskyi, Jul 2023
/// @author Andrii Verbytskyi, May 2024
///
class StringParse: public std::vector<std::string> {
public:

  /// Constructor, taking a string of whitespace-separated tokens
  StringParse(const std::string& input) {
    std::istringstream instring(input);
    std::string token;
    while (instring >> token) this->push_back(token);
  }

  /// Templated function to get the num'th token as any numeric type
  template <typename T> T piece(size_t num) const {
    if (num > this->size()) return {-1};
    std::string token = this->at(num-1);
    T result{-1};
    /// https://en.cppreference.com/w/cpp/utility/from_chars
    auto [ptr, ec] = std::from_chars(token.data(), token.data() + token.size(), result);
    if (ec == std::errc() && ptr == token.data() + token.size()) return result;
    if (ec == std::errc::invalid_argument || ptr != token.data() + token.size() ) return {-1}; //std::cout << "This number cannot be parsed.\n"; 
    if (ec == std::errc::result_out_of_range) return {-1}; //std::cout << "This number is out of range of the return type.\n";
    return {-1};
  }
};
  /// Function to get the num'th token as a string
template <> std::string StringParse::piece(size_t num) const {
    return num > this->size() ? std::string{} : this->at(num-1);
}
#endif
