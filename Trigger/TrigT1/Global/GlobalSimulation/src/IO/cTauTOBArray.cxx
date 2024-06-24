// Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

#include "cTauTOBArray.h"
#include "L1TopoEvent/TOBArray.h"
#include "L1TopoEvent/GenericTOB.h"

namespace GlobalSim {
  cTauTOBArray::cTauTOBArray(const std::string & name,
				  unsigned int reserve) :
    InputTOBArray(name),
    DataArrayImpl<TCS::cTauTOB>(reserve)
  {}

  void
  cTauTOBArray::print(std::ostream &o) const {
    o << name() << std::endl;
    for(const_iterator tob = begin(); tob != end(); ++tob)
      o << **tob << std::endl;
  }

}
