//  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#include "FPGAClusterSorting/FPGAClusterDataVectorSortingAlg.h"
#include "xAODInDetMeasurement/PixelClusterAuxContainer.h"
#include "xAODInDetMeasurement/StripClusterAuxContainer.h"


FPGAClusterDataVectorSortingAlg::FPGAClusterDataVectorSortingAlg(const std::string& name, ISvcLocator* pSvcLocator) : AthReentrantAlgorithm(name, pSvcLocator) {
}



StatusCode FPGAClusterDataVectorSortingAlg::initialize() {
    
    ATH_CHECK(m_xAODPixelClusterContainerKey.initialize());
    ATH_CHECK(m_xAODStripClusterContainerKeys.initialize());
    ATH_CHECK(m_sortedxAODPixelClusterContainerKey.initialize());
    ATH_CHECK(m_sortedxAODStripClusterContainerKeys.initialize());

    return StatusCode::SUCCESS;
}

StatusCode FPGAClusterDataVectorSortingAlg::execute(const EventContext& ctx) const {

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


    auto sortedxAODPixelClusters = std::make_unique<ConstDataVector<xAOD::PixelClusterContainer>>(SG::VIEW_ELEMENTS);
    SG::WriteHandle<ConstDataVector<xAOD::PixelClusterContainer>> sortedxAODPixelClustersHandle(m_sortedxAODPixelClusterContainerKey, ctx);
    
    
    auto sortedxAODStripClusters = std::make_unique<ConstDataVector<xAOD::StripClusterContainer>>(SG::VIEW_ELEMENTS);
    SG::WriteHandle<ConstDataVector<xAOD::StripClusterContainer>> sortedxAODStripClustersHandle(m_sortedxAODStripClusterContainerKeys, ctx);


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
        sortedxAODPixelClusters->push_back(cl);
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
        sortedxAODStripClusters->push_back(cl);
    }

    ATH_CHECK(sortedxAODPixelClustersHandle.record(std::move(sortedxAODPixelClusters)));
    ATH_CHECK(sortedxAODStripClustersHandle.record(std::move(sortedxAODStripClusters)));




    return StatusCode::SUCCESS;
}