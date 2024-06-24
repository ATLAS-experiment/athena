/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "jJetTOBArray.h"
#include "L1TopoEvent/GenericTOB.h"

#include <ostream>

namespace GlobalSim {
  
  jJetTOBArray::jJetTOBArray(const std::string & name,
			     unsigned int reserve) :
    TCS::InputTOBArray(name),
    TCS::DataArrayImpl<TCS::jJetTOB>(reserve)
  {}
  

  void
  jJetTOBArray::print(std::ostream &o) const {
    o << name() << std::endl;
    for(const_iterator tob = begin(); tob != end(); ++tob)
      o << **tob << std::endl;
  }

}
