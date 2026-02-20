#ifndef PFCLUSTERFILLER_H
#define PFCLUSTERFILLER_H

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