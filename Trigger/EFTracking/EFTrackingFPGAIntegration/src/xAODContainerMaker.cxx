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

StatusCode xAODContainerMaker::makePixelSpacePointContainer(
    const EFTrackingTransient::SpacePointAuxInput &psAux,
    const xAOD::PixelClusterContainer &pcluster,
    const EFTrackingTransient::Metadata *metadata,
    const EventContext &ctx) const {
  ATH_MSG_DEBUG("Making xAOD::SpacePointContainer from FTrackingDataFormats::SpacePointAuxInput");

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

    // Global position     
    Eigen::Matrix<float, 3, 1> globalPosition(
        psAux.globalPosition.at(i * 3), psAux.globalPosition.at(i * 3 + 1),
        psAux.globalPosition.at(i * 3 + 2));

    const int meas_idx = psAux.measurementIndexes.at(i);

    // measurement list    
    std::vector<const xAOD::UncalibratedMeasurement* > measurementLinks( {pcluster.at(meas_idx)});

    pxsp->setSpacePoint(psAux.elementIdList.at(i), globalPosition,
                        psAux.varianceR.at(i), psAux.varianceZ.at(i),
                        measurementLinks);
  }
  return StatusCode::SUCCESS;
}

StatusCode xAODContainerMaker::makeStripSpacePointContainer(
    const EFTrackingTransient::SpacePointAuxInput &sspAux,
    const xAOD::StripClusterContainer& scluster,
    const EFTrackingTransient::Metadata *metadata,
    const EventContext &ctx) const {
  ATH_MSG_DEBUG("Making xAOD::SpacePointContainer  from FTrackingDataFormats::SpacePointAuxInput");

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

    std::vector<const xAOD::UncalibratedMeasurement *> strip_meas;

    const int meas_idx1 = sspAux.measurementIndexes.at(i * 2 );
    const int meas_idx2 = sspAux.measurementIndexes.at(i * 2 + 1);

    //Get measurements
    strip_meas.push_back(scluster.at(meas_idx1));
    strip_meas.push_back(scluster.at(meas_idx2));

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
