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

#include "EFTrackingFPGAUtility/xAODClusterMaker.h"

#include "AthContainers/Accessor.h"
#include "AthenaKernel/Chrono.h"
#include "CxxUtils/copy_bounded.h"
#include "CxxUtils/span.h"
#include "Identifier/Identifier.h"
#include "StoreGate/WriteHandle.h"
#include "xAODInDetMeasurement/PixelClusterAuxContainer.h"
#include "xAODInDetMeasurement/StripClusterAuxContainer.h"

StatusCode xAODClusterMaker::initialize() {
  ATH_MSG_INFO("Initialising xAODClusterMaker tool");

  // Initialise the write handles
  ATH_CHECK(m_pixelClustersKey.initialize());
  ATH_CHECK(m_stripClustersKey.initialize());

  // Initialise the chrono service
  ATH_CHECK(m_chronoSvc.retrieve());

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

  if (!m_doBulkCopy) {
    // --------------------------------------------------------------------
    // proceed with the element-wise method
    // --------------------------------------------------------------------
    ATH_MSG_DEBUG("You are running the element-wise container creation method.");
    Athena::Chrono chrono("ElementWiseMethod", m_chronoSvc.get());

    ATH_CHECK(pixelClustersHandle.record(
        std::make_unique<xAOD::PixelClusterContainer>(),
        std::make_unique<xAOD::PixelClusterAuxContainer>()));

    ATH_CHECK(pixelClustersHandle.isValid());
    ATH_MSG_DEBUG("Container '" << m_pixelClustersKey << "' initialised");

    int rdoIndex_counter = 0;

    for (unsigned int i = 0; i < metadata->numOfPixelClusters; i++) {
      // Push back numClusters of PixelCluster
      auto pixelCl = pixelClustersHandle->push_back(
          std::make_unique<xAOD::PixelCluster>());

      Eigen::Matrix<float, 2, 1> localPosition(
          pxAux.localPosition[i * 2], pxAux.localPosition[i * 2 + 1]);
      Eigen::Matrix<float, 2, 2> localCovariance;
      localCovariance.setZero();
      localCovariance(0, 0) = pxAux.localCovariance[i * 2];
      localCovariance(1, 1) = pxAux.localCovariance[i * 2 + 1];
      Eigen::Matrix<float, 3, 1> globalPosition(
          pxAux.globalPosition[i * 3], pxAux.globalPosition[i * 3 + 1],
          pxAux.globalPosition[i * 3 + 2]);

      std::vector<Identifier> RDOs;
      RDOs.reserve(metadata->pcRdoIndex[i]);
      // Cover RDO
      for (unsigned int j = 0; j < metadata->pcRdoIndex[i]; ++j) {
        RDOs.push_back(Identifier(pxAux.rdoList[rdoIndex_counter + j]));
      }

      rdoIndex_counter += metadata->pcRdoIndex[i];

      pixelCl->setMeasurement<2>(pxAux.idHash[i], localPosition,
                                 localCovariance);
      pixelCl->setIdentifier(pxAux.id[i]);
      pixelCl->setRDOlist(RDOs);
      pixelCl->globalPosition() = globalPosition;
      pixelCl->setTotalToT(pxAux.totalToT[i]);
      pixelCl->setChannelsInPhiEta(pxAux.channelsInPhi[i],
                                   pxAux.channelsInEta[i]);
      pixelCl->setWidthInEta(pxAux.widthInEta[i]);
      pixelCl->setOmegas(pxAux.omegaX[i], pxAux.omegaY[i]);
    }
    return StatusCode::SUCCESS;
  }

  // --------------------------------------------------------------------
  // proceed with the bulk copy method
  // --------------------------------------------------------------------

  ATH_MSG_DEBUG("You are running the bulk copy container creation method.");
  Athena::Chrono chrono("BulkCopyMethod", m_chronoSvc.get());

  // --------------------------
  // Create the container and aux. container
  // --------------------------
  auto pixelCl = std::make_unique<xAOD::PixelClusterContainer>();
  auto pixelClAux = std::make_unique<xAOD::PixelClusterAuxContainer>();
  pixelCl->setStore(pixelClAux.get());

  // Pre-allocate memory for all clusters in the AuxContainer
  // and reserve the same space in the container.
  const size_t nClusters = metadata->numOfPixelClusters;
  pixelClAux->resize(nClusters);
  pixelCl->reserve(nClusters);

  // Now, push back the PixelCluster objects
  // and create them all at once.
  // Note, that no data is set yet,
  // we will do that in the next step.
  for (size_t i = 0; i < nClusters; ++i) {
    pixelCl->push_back(std::make_unique<xAOD::PixelCluster>());
  }

  // --------------------------
  // Prepare local buffers for each of the
  // fixed-size attributes.
  // --------------------------
  std::vector<long unsigned int> identifierBuffer(nClusters);
  std::vector<unsigned int> idHashBuffer(nClusters);
  std::vector<std::array<float, 3>> gpBuffer(nClusters);
  std::vector<float> localPosX(nClusters);
  std::vector<float> localPosY(nClusters);
  std::vector<float> localCovXX(nClusters);
  std::vector<float> localCovYY(nClusters);
  std::vector<int> totalToTBuffer(nClusters);
  std::vector<float> widthInEtaBuffer(nClusters);
  std::vector<float> omegaXBuffer(nClusters);
  std::vector<float> omegaYBuffer(nClusters);
  std::vector<int> channelsInPhiBuffer(nClusters);
  std::vector<int> channelsInEtaBuffer(nClusters);

  for (size_t i = 0; i < nClusters; ++i) {

    // Fill the identifierBuffer
    identifierBuffer[i] = pxAux.id[i];

    // Fill the idHashBuffer
    idHashBuffer[i] = pxAux.idHash[i];

    // Fill the globalPositionBuffers
    gpBuffer[i] = {
        pxAux.globalPosition[3 * i],
        pxAux.globalPosition[3 * i + 1],
        pxAux.globalPosition[3 * i + 2],
    };

    // Fill the localPositionBuffers
    localPosX[i] = pxAux.localPosition[2 * i];
    localPosY[i] = pxAux.localPosition[2 * i + 1];

    // Fill the localCovBuffer[i]
    localCovXX[i] = pxAux.localCovariance[2 * i];
    localCovYY[i] = pxAux.localCovariance[2 * i + 1];

    // Fill the totalToTBuffer
    totalToTBuffer[i] = pxAux.totalToT[i];

    // Fill the widthInEtaBuffer
    widthInEtaBuffer[i] = pxAux.widthInEta[i];

    // Fill the omegaBuffer
    omegaXBuffer[i] = pxAux.omegaX[i];
    omegaYBuffer[i] = pxAux.omegaY[i];

    // Fill the channelsInPhiEtaBuffer
    channelsInPhiBuffer[i] = pxAux.channelsInPhi[i];
    channelsInEtaBuffer[i] = pxAux.channelsInEta[i];
  }

  // --------------------------
  // Now, use SG::Accessors to do a bulk copy into
  // the container memory.
  // --------------------------

  // FIXED-SIZE ATTRIBUTES
  static const SG::Accessor<long unsigned int> idAcc("identifier");
  static const SG::Accessor<unsigned int> idHashAcc("idHash");
  static const SG::Accessor<float> localPosXAcc("localPositionX");
  static const SG::Accessor<float> localPosYAcc("localPositionY");
  static const SG::Accessor<float> localCovXXAcc("localCovarianceXX");
  static const SG::Accessor<float> localCovYYAcc("localCovarianceYY");
  static const SG::Accessor<int> totAcc("totalToT");
  static const SG::Accessor<float> widthEtaAcc("widthInEta");
  static const SG::Accessor<float> omegaXAcc("omegasX");
  static const SG::Accessor<float> omegaYAcc("omegasY");
  static const SG::Accessor<int> channelsInPhiAcc("channelsInPhi");
  static const SG::Accessor<int> channelsInEtaAcc("channelsInEta");

  // VARIABLE-LENGTH ATTRIBUTES
  static const SG::Accessor<std::array<float, 3>> globalPosAcc(
      "globalPosition");

  // Get spans into the container
  auto idSpan = idAcc.getDataSpan(*pixelCl);
  auto idHashSpan = idHashAcc.getDataSpan(*pixelCl);
  auto locPosXSpan = localPosXAcc.getDataSpan(*pixelCl);
  auto locPosYSpan = localPosYAcc.getDataSpan(*pixelCl);
  auto locCovXXSpan = localCovXXAcc.getDataSpan(*pixelCl);
  auto locCovYYSpan = localCovYYAcc.getDataSpan(*pixelCl);
  auto totSpan = totAcc.getDataSpan(*pixelCl);
  auto wEtaSpan = widthEtaAcc.getDataSpan(*pixelCl);
  auto omegaXSpan = omegaXAcc.getDataSpan(*pixelCl);
  auto omegaYSpan = omegaYAcc.getDataSpan(*pixelCl);
  auto gpSpan = globalPosAcc.getDataSpan(*pixelCl);
  auto channelsInPhiSpan = channelsInPhiAcc.getDataSpan(*pixelCl);
  auto channelsInEtaSpan = channelsInEtaAcc.getDataSpan(*pixelCl);

  // Bulk copy the fixed-size attributes
  CxxUtils::copy_bounded(identifierBuffer, idSpan);
  CxxUtils::copy_bounded(idHashBuffer, idHashSpan);
  CxxUtils::copy_bounded(localPosX, locPosXSpan);
  CxxUtils::copy_bounded(localPosY, locPosYSpan);
  CxxUtils::copy_bounded(localCovXX, locCovXXSpan);
  CxxUtils::copy_bounded(localCovYY, locCovYYSpan);
  CxxUtils::copy_bounded(totalToTBuffer, totSpan);
  CxxUtils::copy_bounded(widthInEtaBuffer, wEtaSpan);
  CxxUtils::copy_bounded(omegaXBuffer, omegaXSpan);
  CxxUtils::copy_bounded(omegaYBuffer, omegaYSpan);
  CxxUtils::copy_bounded(gpBuffer, gpSpan);
  CxxUtils::copy_bounded(channelsInPhiBuffer, channelsInPhiSpan);
  CxxUtils::copy_bounded(channelsInEtaBuffer, channelsInEtaSpan);

  // --------------------------
  // Copy the variable-length data.
  // We need to loop over the number of
  // clusters to get access to each
  // element.
  // --------------------------
  static const SG::Accessor<std::vector<unsigned long long>> rdoListAcc(
      "rdoList");
  auto rdoSpan = rdoListAcc.getDataSpan(*pixelCl);

  int rdoIndex_counter = 0;
  // loop over the clusters
  for (size_t i = 0; i < nClusters; ++i) {
    size_t nRDOs = metadata->pcRdoIndex[i];

    // direct ref to destination vector
    std::vector<unsigned long long> &rdosForThisCluster = rdoSpan[i];

    // pre-size the vector
    rdosForThisCluster.resize(nRDOs);

    // direct element access
    for (size_t j = 0; j < nRDOs; ++j) {
      rdosForThisCluster[j] = pxAux.rdoList[rdoIndex_counter + j];
    }
    rdoIndex_counter += nRDOs;
  }
  // --------------------------
  // Record the container + aux container into StoreGate,
  // but now we do it AFTER filling the container!
  // --------------------------

  ATH_CHECK(
      pixelClustersHandle.record(std::move(pixelCl), std::move(pixelClAux)));

  ATH_MSG_DEBUG("Bulk copy for fixed-size variables done.");
  return StatusCode::SUCCESS;
}
