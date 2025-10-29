//  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#ifndef GLOBSIM_DUMP_H
#define GLOBSIM_DUMP_H

// utilitiy to dump objects which support operator<<() to a file
#include <string>
namespace GlobalSim {
  template<typename T>
  void dump(const std::string& fn, const T& t);
}

#endif
