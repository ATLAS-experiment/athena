/*
    Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file src/xAODContainerMaker.cxx
 * @author zhaoyuan.cui@cern.ch
 * @author yuan-tang.chou@cern.ch
 * @date Apr. 15, 2024
 */

#include "xAODContainerMaker.h"

#include "Identifier/Identifier.h"
#include "StoreGate/WriteHandle.h"
#include "xAODInDetMeasurement/PixelClusterAuxContainer.h"
#include "xAODInDetMeasurement/SpacePointAuxContainer.h"
#include "xAODInDetMeasurement/StripClusterAuxContainer.h"

StatusCode xAODContainerMaker::initialize() {
  ATH_MSG_INFO("Initializing xAODContainerMaker tool");

  ATH_CHECK(m_pixelClustersKey.initialize());
  ATH_CHECK(m_stripClustersKey.initialize());
  ATH_CHECK(m_stripSpacePointsKey.initialize());
  ATH_CHECK(m_pixelSpacePointsKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode xAODContainerMaker::makeStripClusterContainer(
    const EFTrackingDataFormats::StripClusterAuxInput &scAux,
    const EFTrackingDataFormats::Metadata *metadata,
    const EventContext &ctx) const {
  ATH_MSG_DEBUG("Making xAOD::StripClusterContainer");

  SG::WriteHandle<xAOD::StripClusterContainer> stripClustersHandle{
      m_stripClustersKey, ctx};

  ATH_CHECK(stripClustersHandle.record(
      std::make_unique<xAOD::StripClusterContainer>(),
      std::make_unique<xAOD::StripClusterAuxContainer>()));

  int rdoIndex_counter = 0;

  for (unsigned int i = 0; i < metadata->numOfStripClusters; i++) {
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

StatusCode xAODContainerMaker::makePixelClusterContainer(
    const EFTrackingDataFormats::PixelClusterAuxInput &pxAux,
    const EFTrackingDataFormats::Metadata *metadata,
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

  for (unsigned int i = 0; i < metadata->numOfPixelClusters; i++) {
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
    RDOs.reserve(metadata->pcRdoIndex[i]);
    // Cover RDO
    for (unsigned int j = 0; j < metadata->pcRdoIndex[i]; ++j) {
      RDOs.push_back(Identifier(pxAux.rdoList.at(rdoIndex_counter + j)));
    }

    std::vector<int> vec_totList;
    vec_totList.reserve(metadata->pcTotIndex[i]);
    for (unsigned int j = 0; j < metadata->pcTotIndex[i]; ++j) {
      vec_totList.push_back(pxAux.totList.at(totListIndex_counter + j));
    }

    std::vector<float> vec_chargeList;
    vec_chargeList.reserve(metadata->pcChargeIndex[i]);
    for (unsigned int k = 0; k < metadata->pcChargeIndex[i]; ++k) {
      vec_chargeList.push_back(pxAux.totList.at(chargeListIndex_counter + k));
    }

    rdoIndex_counter += metadata->pcRdoIndex[i];
    totListIndex_counter += metadata->pcTotIndex[i];
    chargeListIndex_counter += metadata->pcChargeIndex[i];

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

StatusCode xAODContainerMaker::makePixelSpacePointContainer(
    const EFTrackingDataFormats::SpacePointAuxInput &psAux,
    const std::vector<std::vector<const xAOD::UncalibratedMeasurement *>>&
        pixelsp_meas,
    const EFTrackingDataFormats::Metadata *metadata,
    const EventContext &ctx) const {
  ATH_MSG_DEBUG("Making xAOD::SpacePointContainer");

  SG::WriteHandle<xAOD::SpacePointContainer> pixelSpacePointsHandle{
      m_pixelSpacePointsKey, ctx};

  ATH_CHECK(pixelSpacePointsHandle.record(
      std::make_unique<xAOD::SpacePointContainer>(),
      std::make_unique<xAOD::SpacePointAuxContainer>()));

  ATH_CHECK(pixelSpacePointsHandle.isValid());
  ATH_MSG_DEBUG("Container '" << m_pixelSpacePointsKey << "' initialised");

  for (unsigned int i = 0; i < metadata->numOfPixelSpacePoints; i++) {
    // Puch back numPixelSpacePoints of SpacePoint
    auto pxsp =
        pixelSpacePointsHandle->push_back(std::make_unique<xAOD::SpacePoint>());
    Eigen::Matrix<float, 3, 1> globalPosition(
        psAux.globalPosition.at(i * 3), psAux.globalPosition.at(i * 3 + 1),
        psAux.globalPosition.at(i * 3 + 2));

    std::vector<const xAOD::UncalibratedMeasurement *> pixel_meas(
        pixelsp_meas.at(i).size());
    std::copy(pixelsp_meas.at(i).begin(), pixelsp_meas.at(i).end(),
              pixel_meas.begin());

    pxsp->setSpacePoint(psAux.elementIdList.at(i), globalPosition,
                        psAux.varianceR.at(i), psAux.varianceZ.at(i),
                        pixel_meas);
  }
  return StatusCode::SUCCESS;
}

StatusCode xAODContainerMaker::makeStripSpacePointContainer(
    const EFTrackingDataFormats::SpacePointAuxInput &sspAux,
    const std::vector<std::vector<const xAOD::UncalibratedMeasurement *>>&
        stripsp_meas,
    const EFTrackingDataFormats::Metadata *metadata,
    const EventContext &ctx) const {
  ATH_MSG_DEBUG("Making xAOD::SpacePointContainer");

  SG::WriteHandle<xAOD::SpacePointContainer> stripSpacePointsHandle{
      m_stripSpacePointsKey, ctx};

  ATH_CHECK(stripSpacePointsHandle.record(
      std::make_unique<xAOD::SpacePointContainer>(),
      std::make_unique<xAOD::SpacePointAuxContainer>()));

  ATH_CHECK(stripSpacePointsHandle.isValid());
  ATH_MSG_DEBUG("Container '" << m_stripSpacePointsKey << "' initialised");

  for (unsigned int i = 0; i < metadata->numOfStripSpacePoints; i++) {
    // Puch back numStripSpacePoints of SpacePoint
    auto ssp =
        stripSpacePointsHandle->push_back(std::make_unique<xAOD::SpacePoint>());

    Eigen::Matrix<float, 3, 1> globalPosition(
        sspAux.globalPosition.at(i * 3), sspAux.globalPosition.at(i * 3 + 1),
        sspAux.globalPosition.at(i * 3 + 2));

    std::vector<const xAOD::UncalibratedMeasurement *> strip_meas(
        stripsp_meas.at(i).size());
    std::copy(stripsp_meas.at(i).begin(), stripsp_meas.at(i).end(),
              strip_meas.begin());

    float topHalfStripLength = sspAux.topHalfStripLength.at(i);
    float bottomHalfStripLength = sspAux.bottomHalfStripLength.at(i);
    Eigen::Matrix<float, 3, 1> topStripDirection(
        sspAux.topStripDirection.at(i * 3),
        sspAux.topStripDirection.at(i * 3 + 1),
        sspAux.topStripDirection.at(i * 3 + 2));
    Eigen::Matrix<float, 3, 1> bottomStripDirection(
        sspAux.bottomStripDirection.at(i * 3),
        sspAux.bottomStripDirection.at(i * 3 + 1),
        sspAux.bottomStripDirection.at(i * 3 + 2));
    Eigen::Matrix<float, 3, 1> stripCenterDistance(
        sspAux.topStripCenter.at(i * 3), sspAux.topStripCenter.at(i * 3 + 1),
        sspAux.topStripCenter.at(i * 3 + 2));
    Eigen::Matrix<float, 3, 1> topStripCenter(
        sspAux.topStripCenter.at(i * 3), sspAux.topStripCenter.at(i * 3 + 1),
        sspAux.topStripCenter.at(i * 3 + 2));
    ssp->setSpacePoint(
        {sspAux.elementIdList.at(i), sspAux.elementIdList.at(i + 1)},
        globalPosition, sspAux.varianceR.at(i), sspAux.varianceZ.at(i),
        strip_meas, topHalfStripLength, bottomHalfStripLength,
        topStripDirection, bottomStripDirection, stripCenterDistance,
        topStripCenter);
  }
  return StatusCode::SUCCESS;
}
