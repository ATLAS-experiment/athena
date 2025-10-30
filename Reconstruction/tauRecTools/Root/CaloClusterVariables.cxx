/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "tauRecTools/CaloClusterVariables.h"

#include "xAODCaloEvent/CaloVertexedTopoCluster.h"

#include <cmath>

const double CaloClusterVariables::DEFAULT = -1111.;

//****************************************
// constructor
//****************************************

CaloClusterVariables::CaloClusterVariables() :
  m_numConstit(static_cast<int>(DEFAULT)){
  }

//*******************************************
// update/fill the cluster based variables
//*******************************************

bool CaloClusterVariables::update(const xAOD::TauJet& pTau) {
    
  const auto& vertexedClusterList = pTau.vertexedClusters();

  std::vector<TLorentzVector> clusterP4Vector;
  clusterP4Vector.reserve(vertexedClusterList.size());

  for (const xAOD::CaloVertexedTopoCluster& vertexedCluster : vertexedClusterList) {
    clusterP4Vector.push_back(vertexedCluster.p4());
  }

  this->m_numConstit = std::ssize(clusterP4Vector);

  return true;
}


