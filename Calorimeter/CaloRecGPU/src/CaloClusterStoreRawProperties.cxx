//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#include "CaloClusterStoreRawProperties.h"


StatusCode CaloClusterStoreRawProperties::execute (const EventContext &, xAOD::CaloClusterContainer * cluster_collection) const
{
  for (xAOD::CaloCluster* cluster : *cluster_collection)
  {
    cluster->setRawE(cluster->calE());
    cluster->setRawEta(cluster->calEta());
    cluster->setRawPhi(cluster->calPhi());
    cluster->setRawM(cluster->calM());
  }

  return StatusCode::SUCCESS;

}

