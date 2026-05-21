/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EFLOWREC_PFDATA_H
#define EFLOWREC_PFDATA_H

#include "EtaPhiLUT.h"

class eflowCaloObjectContainer;
class eflowRecTrack;
class eflowRecCluster;

struct PFData
  {
    eflowCaloObjectContainer *caloObjects;
    std::vector<eflowRecTrack *> tracks;
    std::vector<eflowRecCluster *> clusters;
    eflowRec::EtaPhiLUT clusterLUT;

    unsigned int nMatches = 0;
    unsigned int nOrigCaloObj = 0;
  };

#endif
