/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "AthMonBench.h"
#include <iostream>
#include <limits>
#include <string>
#include <fstream>

namespace{
  template <long f>
  long
  multiply(long result){
    static constexpr long maxval = std::numeric_limits<long>::max()/f;
    if ((result>maxval) or (result<0)) return -1;
    return result * f;
  }
}

AthMonBench::TMem 
AthMonBench::currentVMem(){
  long result = -1;
  std::ifstream file("/proc/self/status");
  const std::string search{"VmSize:"};
  std::string line;
  while(getline(file, line)) { 
    if (line.starts_with(search)) {
      result = std::stol(line.substr(search.size()));
      result = multiply<1024L>(result);
      break;
    }
  }
  return result;
}

std::ostream& operator << ( std::ostream& os, const AthMonBench& br) {
  if (br.valid())
    os << "deltaMem: "<<br.deltaMem_mb()<<" mb, deltaCPU: "<<br.deltaCPU_ms()<<" ms";
  else
    os <<" [no data]";
  return os;
}
