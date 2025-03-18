/*
    Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file src/xAODClusterMaker.cxx
 * @author zhaoyuan.cui@cern.ch
 * @author yuan-tang.chou@cern.ch
 * @author levi.samuel.evans@cern.ch
 * @date Mar. 11, 2025
 */

#include "xAODClusterMaker.h"

#include "Identifier/Identifier.h"
#include "StoreGate/WriteHandle.h"
#include "xAODInDetMeasurement/PixelClusterAuxContainer.h"
#include "xAODInDetMeasurement/StripClusterAuxContainer.h"

StatusCode xAODClusterMaker::initialize() {
  ATH_MSG_INFO("Initialising xAODClusterMaker tool");

  // Initialise the write handles
  ATH_CHECK(m_pixelClustersKey.initialize());
  ATH_CHECK(m_stripClustersKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode xAODClusterMaker::makeStripClusterContainer(
    const EFTrackingTransient::StripClusterAuxInput &scAux,
    const EFTrackingTransient::Metadata *metadata,
    const EventContext &ctx) const {
  ATH_MSG_DEBUG("Making xAOD::StripClusterContainer");

  SG::WriteHandle<xAOD::StripClusterContainer> stripClustersHandle{
      m_stripClustersKey, ctx};

  ATH_CHECK(stripClustersHandle.record(
      std::make_unique<xAOD::StripClusterContainer>(),
      std::make_unique<xAOD::StripClusterAuxContainer>()));

  int rdoIndex_counter = 0;

  for (unsigned int i = 0; i < metadata->numOfStripClusters; i++) {
    // Push back numClusters of StripCluster
    auto stripCl =
        stripClustersHandle->push_back(std::make_unique<xAOD::StripCluster>());

    // Build Matrix
    Eigen::Matrix<float, 1, 1> localPosition;
    Eigen::Matrix<float, 1, 1> localCovariance;

    localPosition(0, 0) = scAux.localPosition.at(i);
    localCovariance(0, 0) = scAux.localCovariance.at(i);

    Eigen::Matrix<float, 3, 1> globalPosition(
        scAux.globalPosition.at(i * 3), scAux.globalPosition.at(i * 3 + 1),
        scAux.globalPosition.at(i * 3 + 2));

    std::vector<Identifier> RDOs;
    RDOs.reserve(metadata->scRdoIndex[i]);
    // Cover RDO
    for (unsigned int j = 0; j < metadata->scRdoIndex[i]; ++j) {
      RDOs.push_back(Identifier(scAux.rdoList.at(rdoIndex_counter + j)));
    }

    rdoIndex_counter += metadata->scRdoIndex[i];

    stripCl->setMeasurement<1>(scAux.idHash.at(i), localPosition,
                               localCovariance);
    stripCl->setIdentifier(scAux.id.at(i));
    stripCl->setRDOlist(RDOs);
    stripCl->globalPosition() = globalPosition;
    stripCl->setChannelsInPhi(scAux.channelsInPhi.at(i));
  }

  return StatusCode::SUCCESS;
}

StatusCode xAODClusterMaker::makePixelClusterContainer(
    const EFTrackingTransient::PixelClusterAuxInput &pxAux,
    const EFTrackingTransient::Metadata *metadata,
    const EventContext &ctx) const {
  ATH_MSG_DEBUG("Making xAOD::PixelClusterContainer");

  SG::WriteHandle<xAOD::PixelClusterContainer> pixelClustersHandle{
      m_pixelClustersKey, ctx};

  ATH_CHECK(pixelClustersHandle.record(
      std::make_unique<xAOD::PixelClusterContainer>(),
      std::make_unique<xAOD::PixelClusterAuxContainer>()));

  ATH_CHECK(pixelClustersHandle.isValid());
  ATH_MSG_DEBUG("Container '" << m_pixelClustersKey << "' initialised");

  int rdoIndex_counter = 0;

  for (unsigned int i = 0; i < metadata->numOfPixelClusters; i++) {
    // Push back numClusters of PixelCluster
    auto pixelCl =
        pixelClustersHandle->push_back(std::make_unique<xAOD::PixelCluster>());

    Eigen::Matrix<float, 2, 1> localPosition(pxAux.localPosition.at(i * 2),
                                             pxAux.localPosition.at(i * 2 + 1));
    Eigen::Matrix<float, 2, 2> localCovariance;
    localCovariance.setZero();
    localCovariance(0, 0) = pxAux.localCovariance.at(i * 2);
    localCovariance(1, 1) = pxAux.localCovariance.at(i * 2 + 1);
    Eigen::Matrix<float, 3, 1> globalPosition(
        pxAux.globalPosition.at(i * 3), pxAux.globalPosition.at(i * 3 + 1),
        pxAux.globalPosition.at(i * 3 + 2));

    std::vector<Identifier> RDOs;
    RDOs.reserve(metadata->pcRdoIndex[i]);
    // Cover RDO
    for (unsigned int j = 0; j < metadata->pcRdoIndex[i]; ++j) {
      RDOs.push_back(Identifier(pxAux.rdoList.at(rdoIndex_counter + j)));
    }

    rdoIndex_counter += metadata->pcRdoIndex[i];

    pixelCl->setMeasurement<2>(pxAux.idHash.at(i), localPosition,
                               localCovariance);
    pixelCl->setIdentifier(pxAux.id.at(i));
    pixelCl->setRDOlist(RDOs);
    pixelCl->globalPosition() = globalPosition;
    pixelCl->setTotalToT(pxAux.totalToT.at(i));
    pixelCl->setChannelsInPhiEta(pxAux.channelsInPhi.at(i),
                                 pxAux.channelsInEta.at(i));
    pixelCl->setWidthInEta(pxAux.widthInEta.at(i));
    pixelCl->setOmegas(pxAux.omegaX.at(i), pxAux.omegaY.at(i));
  }
  return StatusCode::SUCCESS;
} 