/*
    Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "PFClusterWidthDecorator.h"

#include "CaloEvent/CaloCluster.h"

PFClusterWidthDecorator::PFClusterWidthDecorator(const std::string& name, ISvcLocator* pSvcLocator) :
  AthReentrantAlgorithm(name, pSvcLocator)
{}

StatusCode PFClusterWidthDecorator::initialize() {
  ATH_CHECK(m_clusterContainerWidthEtaKey.initialize());
  ATH_CHECK(m_clusterContainerWidthPhiKey.initialize());
  return StatusCode::SUCCESS;
}

StatusCode PFClusterWidthDecorator::execute(const EventContext &ctx) const {

  SG::WriteDecorHandle<xAOD::CaloClusterContainer,float> clusterContainerWidthEta(m_clusterContainerWidthEtaKey, ctx);
    if (!clusterContainerWidthEta.isValid()) {
      ATH_MSG_WARNING("Invalid cluster container with name " << m_clusterContainerWidthEtaKey.key());
      return StatusCode::SUCCESS;
    }

    SG::WriteDecorHandle<xAOD::CaloClusterContainer,float> clusterContainerWidthPhi(m_clusterContainerWidthPhiKey, ctx);
    if (!clusterContainerWidthPhi.isValid()) {
      ATH_MSG_WARNING("Invalid cluster container with name " << m_clusterContainerWidthPhiKey.key());
      return StatusCode::SUCCESS;
    }

    for (const auto *const thisCluster : *clusterContainerWidthEta) {
        const CaloClusterCellLink* theCellLinks = thisCluster->getCellLinks();
        if (!theCellLinks) {
          ATH_MSG_WARNING("No cell links found for cluster");
          continue;
        }

        std::vector<double> eta,phi;
	size_t ncells = theCellLinks->size();
	eta.reserve(ncells);
	phi.reserve(ncells);
        for (CaloClusterCellLink::const_iterator it=theCellLinks->begin(); it!=theCellLinks->end(); ++it){
            const CaloCell* cell = *it;
            eta.push_back(cell->eta());
            phi.push_back(cell->phi());
        }

	PFClusterWidth width = m_clusterWidthCalculator.getPFClusterCoordinateWidth(eta, phi, thisCluster->eta(), thisCluster->phi(), ncells);
        clusterContainerWidthEta(*thisCluster) = width.etaVariance;
        clusterContainerWidthPhi(*thisCluster) = width.phiVariance;
        
  }

  return StatusCode::SUCCESS;
}
