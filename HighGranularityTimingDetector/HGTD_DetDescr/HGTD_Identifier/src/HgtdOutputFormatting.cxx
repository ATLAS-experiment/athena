/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "HgtdOutputFormatting.h"
#include "IdDict/IdDictFieldImplementation.h"
#include <sstream>
#include <ios> //for std::hex

namespace HgtdIdentifierPkg{
  
  std::string
  formatOutput(std::string_view title, const IdDictFieldImplementation & idDictFieldImp){
    std::ostringstream os;
    os<< title <<" "<< idDictFieldImp.decode_index() << " "
          << (std::string) idDictFieldImp.ored_field() << " "
          << std::hex << idDictFieldImp.mask() << " "
          << idDictFieldImp.zeroing_mask() << " "
          << std::dec << idDictFieldImp.shift()
          << " " << idDictFieldImp.bits() << " " << idDictFieldImp.bits_offset() << " ";
    return os.str();     
  }

}
