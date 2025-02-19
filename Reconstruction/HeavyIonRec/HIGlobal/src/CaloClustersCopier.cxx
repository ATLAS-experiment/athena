/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODCaloEvent/CaloClusterAuxContainer.h"


#include "CaloClustersCopier.h"

CaloClustersCopier::CaloClustersCopier(const std::string& name, ISvcLocator* pSvcLocator) :
  AthReentrantAlgorithm(name, pSvcLocator)
{
}

CaloClustersCopier::~CaloClustersCopier()
{
}

StatusCode CaloClustersCopier::initialize()
{
  ATH_CHECK(m_HIEventShapeKey.initialize());
  ATH_CHECK(m_inputClustersKey.initialize());
  ATH_CHECK(m_outputClustersKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode CaloClustersCopier::finalize()
{
  return StatusCode::SUCCESS;
}

StatusCode CaloClustersCopier::execute(const EventContext& context) const
{

  auto esHandle = SG::makeHandle(m_HIEventShapeKey, context);
  ATH_CHECK(esHandle.isValid());
  auto inputClustersHandle = SG::makeHandle(m_inputClustersKey, context);
  ATH_CHECK(inputClustersHandle.isValid());

  // construct and record output clusters in each event
  auto outputClusters = std::make_unique<xAOD::CaloClusterContainer>();
  auto outputClustersAux = std::make_unique<xAOD::CaloClusterAuxContainer>();
  outputClusters->setStore(outputClustersAux.get());

  // calculate energy in fcal
  double fcalEt = 0;

  for (const xAOD::HIEventShape* es : *esHandle) {
    if (es->layer() == 21 || es->layer() == 22 || es->layer() == 23) { // Example: FCal layers
      fcalEt += es->et();
    }
  }

  if ( fcalEt <  m_fcalEtCut ) {
    for ( auto inCluster:  *inputClustersHandle ) {
      xAOD::CaloCluster* outCluster = new xAOD::CaloCluster();
      outputClusters->push_back(outCluster);
      *outCluster = *inCluster;
    }
  }

  auto outputClustersHandle = SG::makeHandle(m_outputClustersKey, context);
  ATH_CHECK( outputClustersHandle.record(std::move(outputClusters), std::move(outputClustersAux)) );

  return StatusCode::SUCCESS;
}
