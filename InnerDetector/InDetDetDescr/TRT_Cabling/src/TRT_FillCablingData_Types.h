/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


/*
 * TRT_FillCablingData Types
 * Used in _SR1 and _SR1_ECC
 */
#ifndef TRT_FILLCABLINGDATA_TYPES_H
#define TRT_FILLCABLINGDATA_TYPES_H
#include <map>
#include <string>
#include <vector>

struct GlobalCableMap_t{
   int SubDet{};
   int Phi{};
   int RODGroup{};
   std::string FEid{};
};

 typedef std::map<int, std::vector< GlobalCableMap_t *> > GlobalCableMap;

#endif // TRT_FILLCABLINGDATA_TYPES_H
