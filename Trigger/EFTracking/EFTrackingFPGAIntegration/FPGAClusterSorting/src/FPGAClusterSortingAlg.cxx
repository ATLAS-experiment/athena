//  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#include "FPGAClusterSorting/FPGAClusterSortingAlg.h"
#include "xAODInDetMeasurement/PixelClusterAuxContainer.h"
#include "xAODInDetMeasurement/StripClusterAuxContainer.h"


FPGAClusterSortingAlg::FPGAClusterSortingAlg(const std::string& name, ISvcLocator* pSvcLocator) : AthReentrantAlgorithm(name, pSvcLocator) {
}



StatusCode FPGAClusterSortingAlg::initialize() {
    
    ATH_CHECK(m_xAODPixelClusterContainerKey.initialize());
    ATH_CHECK(m_xAODStripClusterContainerKeys.initialize());
    ATH_CHECK(m_sortedxAODPixelClusterContainerKey.initialize());
    ATH_CHECK(m_sortedxAODStripClusterContainerKeys.initialize());

    return StatusCode::SUCCESS;
}

StatusCode FPGAClusterSortingAlg::execute(const EventContext& ctx) const {

    if(m_xAODPixelClusterContainerKey.key().empty()) {
        ATH_MSG_ERROR("No input xAOD Pixel Cluster container key provided");
        return StatusCode::FAILURE;
    }
    if(m_xAODStripClusterContainerKeys.key().empty()) {
        ATH_MSG_ERROR("No input xAOD Strip Cluster container key provided");
        return StatusCode::FAILURE;
    }

    SG::ReadHandle<xAOD::PixelClusterContainer> xAODPixelClusters(m_xAODPixelClusterContainerKey, ctx);
    if (!xAODPixelClusters.isValid()) {
        ATH_MSG_ERROR("Failed to retrieve xAOD Pixel Cluster container with key " << m_xAODPixelClusterContainerKey.key());
        return StatusCode::FAILURE;
    }
    SG::ReadHandle<xAOD::StripClusterContainer> xAODStripClusters(m_xAODStripClusterContainerKeys, ctx);
    if (!xAODStripClusters.isValid()) {
        ATH_MSG_ERROR("Failed to retrieve xAOD Strip Cluster container with key " << m_xAODStripClusterContainerKeys.key());
        return StatusCode::FAILURE;
    }


    std::unique_ptr<xAOD::PixelClusterContainer> sortedxAODPixelClusters = std::make_unique<xAOD::PixelClusterContainer>();
    std::unique_ptr<xAOD::PixelClusterAuxContainer> sortedxAODPixelClustersAux = std::make_unique<xAOD::PixelClusterAuxContainer>();
    sortedxAODPixelClusters->setStore (sortedxAODPixelClustersAux.get());

    SG::WriteHandle<xAOD::PixelClusterContainer> sortedxAODPixelClustersHandle(m_sortedxAODPixelClusterContainerKey, ctx);
    
    
    std::unique_ptr<xAOD::StripClusterContainer> sortedxAODStripClusters = std::make_unique<xAOD::StripClusterContainer>();
    std::unique_ptr<xAOD::StripClusterAuxContainer> sortedxAODStripClustersAux = std::make_unique<xAOD::StripClusterAuxContainer>();
    sortedxAODStripClusters->setStore (sortedxAODStripClustersAux.get());
    SG::WriteHandle<xAOD::StripClusterContainer> sortedxAODStripClustersHandle(m_sortedxAODStripClusterContainerKeys, ctx);


    // Copy pixel clusters into a vector for sorting
    std::vector<const xAOD::PixelCluster*> pixelClustersVec;
    pixelClustersVec.reserve(xAODPixelClusters->size());
    for (const xAOD::PixelCluster* cl : *xAODPixelClusters) {
        pixelClustersVec.push_back(cl);
    }

    // Sort by identifierHash
    std::sort(pixelClustersVec.begin(), pixelClustersVec.end(),
        [](const xAOD::PixelCluster* a, const xAOD::PixelCluster* b) {
            return a->identifierHash() < b->identifierHash();
        });

    // Copy sorted clusters to output container
    for (const xAOD::PixelCluster* cl : pixelClustersVec) {
        xAOD::PixelCluster* newCl = new xAOD::PixelCluster();
        sortedxAODPixelClusters->push_back(newCl);
        *newCl = *cl;
    }

    // Copy strip clusters into a vector for sorting
    std::vector<const xAOD::StripCluster*> stripClustersVec;
    stripClustersVec.reserve(xAODStripClusters->size());
    for (const xAOD::StripCluster* cl : *xAODStripClusters) {
        stripClustersVec.push_back(cl);
    }

    // Sort by identifierHash
    std::sort(stripClustersVec.begin(), stripClustersVec.end(),
        [](const xAOD::StripCluster* a, const xAOD::StripCluster* b) {
            return a->identifierHash() < b->identifierHash();
        });

    // Copy sorted clusters to output container
    for (const xAOD::StripCluster* cl : stripClustersVec) {
        xAOD::StripCluster* newCl = new xAOD::StripCluster();
        sortedxAODStripClusters->push_back(newCl);
        *newCl = *cl;
    }


    ATH_CHECK(sortedxAODPixelClustersHandle.record(std::move(sortedxAODPixelClusters), std::move(sortedxAODPixelClustersAux)).isSuccess());
    ATH_CHECK(sortedxAODStripClustersHandle.record(std::move(sortedxAODStripClusters), std::move(sortedxAODStripClustersAux)).isSuccess());




    return StatusCode::SUCCESS;
}