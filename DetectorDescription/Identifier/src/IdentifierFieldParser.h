/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef Identifier_IdentifierFieldParser_h
#define Identifier_IdentifierFieldParser_h
#include <iosfwd>
#include <vector>
#include "Identifier/IdentifierField.h" //for typedef

namespace Identifier{
  bool
  isDigit(const char c);
  
  IdentifierField::element_type
  parseStreamDigits(std::istream & is);

  IdentifierField::element_vector
  parseStreamList(std::istream & is);
  
  IdentifierField::element_vector
  parseTextList(std::string & text);

}
#endif
