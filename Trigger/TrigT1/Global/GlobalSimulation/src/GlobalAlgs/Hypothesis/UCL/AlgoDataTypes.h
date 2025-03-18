/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_ALGODATATYPES_H
#define GLOBALSIM_ALGODATATYPES_H


#include "AlgoConstants.h"

#include <bitset>
#include <ostream>
#include <vector>
#include <memory>

namespace GlobalSim {

  template<typename BITSET>
  int bitSetToInt(BITSET bitset) {
    if (!bitset[bitset.size()-1]) { return bitset.to_ulong();}
    bitset.flip();
    return -(bitset.to_ulong() + 1);
  }
}
  
template<typename T>
std::ostream& operator << (std::ostream& os,
			   const std::vector<T>& v){

  for(const T& t : v) {
    os << t << ' ';
  }
  return os;
}

  
template<typename T>
std::ostream& operator << (std::ostream& os,
			   const std::vector<std::shared_ptr<T>>& v){

  for(const auto& t : v) {
    os << *t << ' ';
  }
  return os;
}

template<typename T>
std::ostream& operator << (std::ostream& os,
			   const std::vector<std::vector<T>>& vv){

  std::size_t iv{0};
  for(const std::vector<T>& v : vv) {
    os << "\ncontainer " << iv++ << " [" << v.size() << "]:\n\n";
    os << v;
  }
  return os;
}

#endif
