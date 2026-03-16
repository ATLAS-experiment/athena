/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef INDETIDENTIFIER_HGTDOUTPUTFORMATTING_H
#define INDETIDENTIFIER_HGTDOUTPUTFORMATTING_H

#include <string>
#include <string_view>

class IdDictFieldImplementation;

namespace HgtdIdentifierPkg{

  std::string
  formatOutput(std::string_view title, const IdDictFieldImplementation & idDictFieldImp);

}
 
#endif
