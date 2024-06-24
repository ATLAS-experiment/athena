// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#include "jXETOBArray.h"

namespace GlobalSim {
  jXETOBArray::jXETOBArray(const std::string & name, unsigned int reserve) :
    TCS::InputTOBArray(name),
    DataArrayImpl<TCS::jXETOB>(reserve)
  {}

  void
  jXETOBArray::print(std::ostream &o) const {
    o << name() << std::endl;
    for(const_iterator tob = begin(); tob != end(); ++tob)
      o << **tob << std::endl;
  }
}
