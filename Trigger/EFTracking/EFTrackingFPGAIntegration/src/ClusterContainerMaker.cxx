/*
    Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file src/ClusterContainerMaker.cxx
 * @author zhaoyuan.cui@cern.ch
 * @author yuan-tang.chou@cern.ch
 * @date Apr. 15, 2024
 */

#include "ClusterContainerMaker.h"

#include "Identifier/Identifier.h"
#include "StoreGate/WriteHandle.h"
#include "xAODInDetMeasurement/PixelClusterAuxContainer.h"
#include "xAODInDetMeasurement/StripClusterAuxContainer.h"

StatusCode ClusterContainerMaker::initialize() {
  ATH_MSG_INFO("Initializing ClusterContainerMaker tool");

  ATH_CHECK(m_pixelClustersKey.initialize());
  ATH_CHECK(m_stripClustersKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode ClusterContainerMaker::makeStripClusterContainer(
    const int numClusters,
    const EFTrackingDataFormats::StripClusterAuxInput &scAux,
    const EFTrackingDataFormats::Metadata *meta,
    const EventContext &ctx) const {
  ATH_MSG_DEBUG("Making xAOD::StripClusterContainer");

  SG::WriteHandle<xAOD::StripClusterContainer> stripClustersHandle{
      m_stripClustersKey, ctx};

  ATH_CHECK(stripClustersHandle.record(
      std::make_unique<xAOD::StripClusterContainer>(),
      std::make_unique<xAOD::StripClusterAuxContainer>()));

  int rdoIndex_counter = 0;

  for (int i = 0; i < numClusters; i++) {
    // Puch back numClusters of StripCluster
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
    RDOs.reserve(meta->scRdoIndex[i]);
    // Cover RDO
    for (int j = 0; j < meta->scRdoIndex[i]; ++j) {
      RDOs.push_back(Identifier(scAux.rdoList.at(rdoIndex_counter + j)));
    }

    rdoIndex_counter += meta->scRdoIndex[i];

    stripCl->setMeasurement<1>(scAux.idHash.at(i), localPosition,
                               localCovariance);
    stripCl->setIdentifier(scAux.id.at(i));
    stripCl->setRDOlist(RDOs);
    stripCl->globalPosition() = globalPosition;
    stripCl->setChannelsInPhi(scAux.channelsInPhi.at(i));
  }

  return StatusCode::SUCCESS;
}

StatusCode ClusterContainerMaker::makePixelClusterContainer(
    const int numClusters,
    const EFTrackingDataFormats::PixelClusterAuxInput &pxAux,
    const EFTrackingDataFormats::Metadata *meta,
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
  int totListIndex_counter = 0;
  int chargeListIndex_counter = 0;

  for (int i = 0; i < numClusters; i++) {
    // Puch back numClusters of PixelCluster
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
    RDOs.reserve(meta->pcRdoIndex[i]);
    // Cover RDO
    for (int j = 0; j < meta->pcRdoIndex[i]; ++j) {
      RDOs.push_back(Identifier(pxAux.rdoList.at(rdoIndex_counter + j)));
    }

    std::vector<int> vec_totList;
    vec_totList.reserve(meta->pcTotIndex[i]);
    for (int j = 0; j < meta->pcTotIndex[i]; ++j) {
      vec_totList.push_back(pxAux.totList.at(totListIndex_counter + j));
    }

    std::vector<float> vec_chargeList;
    vec_chargeList.reserve(meta->pcChargeIndex[i]);
    for (int k = 0; k < meta->pcChargeIndex[i]; ++k) {
      vec_chargeList.push_back(pxAux.totList.at(chargeListIndex_counter + k));
    }

    rdoIndex_counter += meta->pcRdoIndex[i];
    totListIndex_counter += meta->pcTotIndex[i];
    chargeListIndex_counter += meta->pcChargeIndex[i];

    pixelCl->setMeasurement<2>(pxAux.idHash.at(i), localPosition,
                               localCovariance);
    pixelCl->setIdentifier(pxAux.id.at(i));
    pixelCl->setRDOlist(RDOs);
    pixelCl->globalPosition() = globalPosition;
    pixelCl->setToTlist(vec_totList);
    pixelCl->setTotalToT(pxAux.totalToT.at(i));
    pixelCl->setChargelist(vec_chargeList);
    pixelCl->setTotalCharge(pxAux.totalCharge.at(i));
    pixelCl->setLVL1A(pxAux.lvl1a.at(i));
    pixelCl->setChannelsInPhiEta(pxAux.channelsInPhi.at(i),
                                 pxAux.channelsInEta.at(i));
    pixelCl->setWidthInEta(pxAux.widthInEta.at(i));
    pixelCl->setOmegas(pxAux.omegaX.at(i), pxAux.omegaY.at(i));
    pixelCl->setIsSplit(pxAux.isSplit.at(i));
    pixelCl->setSplitProbabilities(pxAux.splitProbability1.at(i),
                                   pxAux.splitProbability2.at(i));
  }
  return StatusCode::SUCCESS;
}