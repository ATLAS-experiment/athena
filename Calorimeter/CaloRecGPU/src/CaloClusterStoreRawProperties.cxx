//
// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#include "CaloClusterStoreRawProperties.h"

CaloClusterStoreRawProperties::CaloClusterStoreRawProperties(const std::string & type, const std::string & name, const IInterface * parent):
  AthAlgTool(type, name, parent)
{
  declareInterface<CaloClusterCollectionProcessor> (this);
}

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

