/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "src/RangeParser.h"
#include <cstdio> //sscanf

namespace Identifier{
  bool 
  RangeParser::run(Range& range, const std::string& text) { 
    range.clear (); 
    size_type pos = 0; 
    return (parse (range, text, pos)); 
  } 

  bool 
  RangeParser::skip_spaces(const std::string& text, size_type& pos){ 
    pos = text.find_first_not_of (" \t", pos); 
    if (pos == std::string::npos) return (false); 
    return (true); 
  } 
   
  bool 
  RangeParser::test_token(const std::string& text, size_type& pos, char token){ 
    if (!skip_spaces (text, pos))return (false); 
    char c = text[pos]; 
    if (c != token) return (false); 
    pos++; 
    return (true); 
  } 
   
  bool 
  RangeParser::parse_number(const std::string& text, size_type& pos, int& value){ 
    if (pos == std::string::npos) return (false); 
    const char& cha = (text.at(pos)); 
    const char* ptr = &cha; 
    int items{}; 
    value = 0; 
    items = sscanf (ptr, "%80d", &value); 
    if (items == 0){ 
      return false; 
    } 
    pos = text.find_first_not_of ("0123456789+- \t", pos); 
    return true; 
  } 
   
  bool 
  RangeParser::parse_maximum(Range::field& field, const std::string& text, size_type& pos){ 
    bool result = true; 
    int maximum{}; 
    if (!skip_spaces (text, pos)) return (false); 
    char c = text[pos]; 
    switch (c) { 
      case '0': 
      case '1': 
      case '2': 
      case '3': 
      case '4': 
      case '5': 
      case '6': 
      case '7': 
      case '8': 
      case '9': 
      case '+': 
      case '-': 
        if (!parse_number (text, pos, maximum)) { 
          result = false; 
          break; 
        } else { 
          result = true; 
          field.set_maximum (maximum); 
        } 
        break; 
      default: 
        result = false; 
        break; 
    } 
    return (result); 
  } 
     
  bool 
  RangeParser::parse_list (Range::field& field, const std::string& text, size_type& pos){ 
    bool finished = false; 
    bool result = true; 
    int value{}; 
    while (!finished) { 
      if (!skip_spaces (text, pos)) return (true); 
      char c = text[pos]; 
      switch (c) { 
        case '0': 
        case '1': 
        case '2': 
        case '3': 
        case '4': 
        case '5': 
        case '6': 
        case '7': 
        case '8': 
        case '9': 
        case '+': 
        case '-': 
          if (!parse_number (text, pos, value)) { 
            finished = true; 
            result = false; 
            break; 
          } else { 
            field.add_value (value); 
          } 
          break; 
        case ',': 
          pos++; 
          break; 
        default: 
          finished = true; 
          result = true; 
          break; 
      } 
    } 
    return (result); 
  } 
     
  bool 
  RangeParser::parse_field(Range::field& field, const std::string& text, size_type& pos) { 
    bool result = true; 
    int minimum{}; 
    if (!skip_spaces (text, pos)) return (false); 
    char c = text[pos]; 
    switch (c) { 
      case '0': 
      case '1': 
      case '2': 
      case '3': 
      case '4': 
      case '5': 
      case '6': 
      case '7': 
      case '8': 
      case '9': 
      case '+': 
      case '-': 
        if (!parse_number (text, pos, minimum)) { 
          result = false; 
          break; 
        } else { 
          field.set_minimum (minimum); 
          result = true;
          if (test_token (text, pos, ':')) { 
            // max is optional, so we don't want to reset result here RDS 4/02
            // result = parse_maximum (field, text, pos); 

            // Optionally, look for max
            parse_maximum (field, text, pos); 
          } else if (test_token (text, pos, ',')) { 
            result = parse_list (field, text, pos); 
          } else { 
            field.set_maximum (minimum); 
            result = true; 
          } 
        }
        break; 
      case ':': 
        pos++; 
        result = parse_maximum (field, text, pos); 
        break; 
      case '*': 
        pos++; 
        result = true; 
        break; 
      default: 
        result = false; 
        break; 
    } 
    return (result); 
  } 
     
  bool 
  RangeParser::parse(Range& range, const std::string& text, size_type& pos){ 
    bool result = true; 
    bool finished = false; 
    if (!skip_spaces (text, pos)) return (true); 
    while (!finished && pos != std::string::npos && pos < text.size()) { 
      char c = text[pos]; 
      switch (c) { 
        case '0': 
        case '1': 
        case '2': 
        case '3': 
        case '4': 
        case '5': 
        case '6': 
        case '7': 
        case '8': 
        case '9': 
        case '+': 
        case '-': 
        case ':': 
        case '*': { 
          Range::field field; 
          if (!parse_field (field, text, pos)) { 
              result = false; 
              finished = true; 
            } else { 
              range.add (field); 
            } 
          } 
          break; 
        case '/': 
          pos++; 
          break; 
        default: 
          finished = true; 
          break; 
      } 
    } 
    return (result); 
  } 

}
