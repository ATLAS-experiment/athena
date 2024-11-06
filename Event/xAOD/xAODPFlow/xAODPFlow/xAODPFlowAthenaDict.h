/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODPFLOW_ATHENADICT_H
#define XAODPFLOW_ATHENADICT_H

#include <vector>
#include "xAODPFlow/xAODPFlowDict.h"

namespace {
   struct GCCXML_DUMMY_INSTANTIATION_XAODFLOWATHENA {
      SG::AuxTypeVectorFactory<std::vector< std::pair< ElementLink< CaloCellContainer >, double> > > FlowElemdummy6;
      std::vector< std::vector< std::pair< ElementLink< CaloCellContainer >, double > > > FlowElemdummy7;
   };
}

#endif