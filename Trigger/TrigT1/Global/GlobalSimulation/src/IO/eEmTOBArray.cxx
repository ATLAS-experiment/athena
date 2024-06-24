// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#include "eEmTOBArray.h"
#include "L1TopoEvent/GenericTOB.h"

#include "L1TopoEvent/eEmTOB.h"

#include <ostream>

namespace GlobalSim {
  eEmTOBArray::eEmTOBArray(const std::string & name, unsigned int reserve) :
    InputTOBArray(name),
    DataArrayImpl<TCS::eEmTOB>(reserve)
  {}

  void
  eEmTOBArray::print(std::ostream &o) const {
    o << name() << '\n';
    for(const_iterator tob = begin(); tob != end(); ++tob)
      o << **tob << '\n';
  }
}

