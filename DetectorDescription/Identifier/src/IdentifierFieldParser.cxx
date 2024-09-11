/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "src/IdentifierFieldParser.h"

#include <string>
#include <iostream>
#include <charconv>
#include <cctype>



namespace Identifier{
  bool
  isDigit(const char c){
    return std::isdigit(c) or c=='-' or c=='+';
  }
  
  IdentifierField::element_type
  parseStreamDigits(std::istream & is){
    static constexpr std::errc ok{};
    char buffer[20]{};
    int i{};
    IdentifierField::element_type v{};
    for(int c{};isDigit(is.peek()) and i<20;++i ){
      if (c == '+') is.ignore();
      c=is.get();
      buffer[i] = c;
    }
    const auto [ptr,ec] = std::from_chars(buffer, buffer+i, v);
    if (ec != ok) throw std::invalid_argument("Invalid argument in parseStreamDigits");
    return v;
  }

  IdentifierField::element_vector
  parseStreamList(std::istream & is){
    IdentifierField::element_vector result;
    for (int c{};(not is.eof());c=is.peek()){
      while (std::isspace(is.peek())){is.ignore();}
      if (isDigit(c)){
        if (c =='+') is.ignore();
        int v = parseStreamDigits(is);
        result.push_back(v);
      } else if (c == ','){
        is.ignore();
      } else if (c == '/' ){ 
        break;
      }
    }
    return result;
  }

}

