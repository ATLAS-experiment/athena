/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EFLOWREC_PFCLUSTERFILLER_H
#define EFLOWREC_PFCLUSTERFILLER_H

#include "eflowRecCluster.h"
#include "PFData.h"

class PFClusterFiller {

public:
  PFClusterFiller(){};
  ~PFClusterFiller(){};

  static void fillClustersToRecover(PFData &data) ;
  static void fillClustersToConsider(PFData &data, eflowRecClusterContainer &recClusterContainer) ;

};
#endif
