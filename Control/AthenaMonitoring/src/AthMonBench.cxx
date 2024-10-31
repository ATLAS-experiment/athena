/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "AthMonBench.h"
#include <cstring>
#include <stdio.h>

AthMonBench::TMem AthMonBench::currentVMem()
{
  long result = -1;
  FILE* file = fopen("/proc/self/status", "r");
  if (!file)
    return result;
  char line[128];
  while (fgets(line, 128, file) != NULL){
    if (strncmp(line, "VmSize:", 7) == 0) {
      std::stringstream s(&(line[7]));
      s >> result;
      result *= 1024;//NB: ~1K uncertainty
      break;
    }
  }
  fclose(file);
  return result;
}

std::ostream& operator << ( std::ostream& os, const AthMonBench& br) {
  if (br.valid())
    os << "deltaMem: "<<br.deltaMem_mb()<<" mb, deltaCPU: "<<br.deltaCPU_ms()<<" ms";
  else
    os <<" [no data]";
  return os;
}
