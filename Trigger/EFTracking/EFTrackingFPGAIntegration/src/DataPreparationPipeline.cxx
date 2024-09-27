/*
    Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file src/DataPreparationPipeline.h
 * @author zhaoyuan.cui@cern.ch
 * @author yuan-tang.chou@cern.ch
 * @date Feb. 18, 2024
 */

#include "DataPreparationPipeline.h"

#include <algorithm>

#include "AthContainers/debug.h"
#include "xAODInDetMeasurement/StripClusterAuxContainer.h"

#define MAX_CLUSTER_NUM \
  500000  // A large enough number for the current development, should be
          // further discussed

#define MAX_SPACEPOINT_NUM \
  500000  // A large enough number for the current development, should be
          // further discussed

StatusCode DataPreparationPipeline::initialize() {
  // Check if we are running the software mode, this doesn't require an FPGA
  // presents in the machine
  if (m_runSW) {
    ATH_MSG_INFO(
        "Running the software mode. The FPGA accelerator will not be used. "
        "Software version of the kernel is used instead.");
  } else {
    ATH_MSG_INFO("Running on the FPGA accelerator");
    ATH_CHECK(IntegrationBase::precheck({m_xclbin, m_kernelName}));
    ATH_CHECK(IntegrationBase::initialize());
    ATH_CHECK(IntegrationBase::loadProgram(m_xclbin));
  }

  ATH_CHECK(m_stripClustersKey.initialize());
  ATH_CHECK(m_pixelClustersKey.initialize());
  ATH_CHECK(m_stripSpacePointsKey.initialize());
  ATH_CHECK(m_pixelSpacePointsKey.initialize());
  ATH_CHECK(m_xAODContainerMaker.retrieve());

  return StatusCode::SUCCESS;
}

StatusCode DataPreparationPipeline::execute(const EventContext &ctx) const {

  // Retrieve the strip and pixel cluster container from the event store
  SG::ReadHandle<xAOD::StripClusterContainer> inputStripClusters(
      m_stripClustersKey, ctx);
  SG::ReadHandle<xAOD::PixelClusterContainer> inputPixelClusters(
      m_pixelClustersKey, ctx);
  SG::ReadHandle<xAOD::SpacePointContainer> inputStripSpacePoints(
      m_stripSpacePointsKey, ctx);
  SG::ReadHandle<xAOD::SpacePointContainer> inputPixelSpacePoints(
      m_pixelSpacePointsKey, ctx);

  // Check if the strip cluster container is valid
  if (!inputStripClusters.isValid()) {
    ATH_MSG_ERROR("Failed to retrieve: " << m_stripClustersKey);
    return StatusCode::FAILURE;
  }

  // Check if the pixel cluster container is valid
  if (!inputPixelClusters.isValid()) {
    ATH_MSG_ERROR("Failed to retrieve: " << m_pixelClustersKey);
    return StatusCode::FAILURE;
  }

  // Check if the Strip space point container is valid
  if (!inputStripSpacePoints.isValid()) {
    ATH_MSG_ERROR("Failed to retrieve: " << m_stripSpacePointsKey);
    return StatusCode::FAILURE;
  }
  // Check if the Pixel space point container is valid
  if (!inputPixelSpacePoints.isValid()) {
    ATH_MSG_ERROR("Failed to retrieve: " << m_pixelSpacePointsKey);
    return StatusCode::FAILURE;
  }
  if (msgLvl(MSG::DEBUG)) {

    ATH_MSG_DEBUG("There are " << inputStripClusters->size()
                               << " strip cluster in this event");
    ATH_MSG_DEBUG("There are " << inputPixelClusters->size()
                               << " pixel cluster in this event");
    ATH_MSG_DEBUG("There are " << inputStripSpacePoints->size()
                               << " Strip space point in this event");
    ATH_MSG_DEBUG("There are " << inputPixelSpacePoints->size()
                               << " Pixel space point in this event");

    // Print auxid for data members for later assignments
    // This is only temporary for development purpose and will be removed in the
    // future
    const SG::auxid_set_t &auxid_set = inputStripClusters->getAuxIDs();
    for (auto id : auxid_set) {
      ATH_MSG_DEBUG("StripClusterContainer" << SGdebug::aux_var_name(id)
                                            << " has AuxId: " << id);
    }

    const SG::auxid_set_t &auxid_set_pixel = inputPixelClusters->getAuxIDs();
    for (auto id : auxid_set_pixel) {
      ATH_MSG_DEBUG("PixelClusterContainer" << SGdebug::aux_var_name(id)
                                            << " has AuxId: " << id);
    }

    const SG::auxid_set_t &auxid_set_ssp = inputStripSpacePoints->getAuxIDs();
    for (auto id : auxid_set_ssp) {
      ATH_MSG_DEBUG("StripSpacePointContainer" << SGdebug::aux_var_name(id)
                                               << " has AuxId: " << id);
    }

    const SG::auxid_set_t &auxid_set_psp = inputPixelSpacePoints->getAuxIDs();
    for (auto id : auxid_set_psp) {
      ATH_MSG_DEBUG("PixelSpacePointContainer" << SGdebug::aux_var_name(id)
                                               << " has AuxId: " << id);
    }
  }

  // Prepare the input data for the kernel
  // This is to "remake" the cluster but in a kernel compatible format using
  // the struct defined in EFTrackingDataFormats.h
  std::vector<EFTrackingDataFormats::StripCluster>
      ef_stripClusters;  // Strip clusters as kernel input argument
  std::vector<EFTrackingDataFormats::PixelCluster>
      ef_pixelClusters;  // Pixel clusters as kernel input argument
  std::vector<EFTrackingDataFormats::SpacePoint>
      ef_stripSpacePoints;  // Strip Space points as kernel input argument
  std::vector<EFTrackingDataFormats::SpacePoint>
      ef_pixelSpacePoints;  // Pixel Space points as kernel input argument

  // vector of vector of to send transit xAOD::UncalibratedMeasurement for the
  // spacepoint
  std::vector<std::vector<const xAOD::UncalibratedMeasurement *>> pixelsp_meas;
  std::vector<std::vector<const xAOD::UncalibratedMeasurement *>> stripsp_meas;

  // Currently we only take the first 10 clusters for testing, this will be
  // removed in the future
  ATH_CHECK(
      getInputClusterData(inputStripClusters.ptr(), ef_stripClusters, 10));
  ATH_CHECK(
      getInputClusterData(inputPixelClusters.ptr(), ef_pixelClusters, 10));
  bool isStrip = true;
  ATH_CHECK(getInputSpacePointData(inputStripSpacePoints.ptr(),
                                   ef_stripSpacePoints, stripsp_meas, 10,
                                   isStrip));
  ATH_CHECK(getInputSpacePointData(inputPixelSpacePoints.ptr(),
                                   ef_pixelSpacePoints, pixelsp_meas, 10,
                                   !isStrip));

  if (msgLvl(MSG::DEBUG)) {
    // print the first 3 elements for all the arrays
    // This is to debug and make sure we are doing the right thing
    for (int i = 0; i < 10; i++) {
      ATH_MSG_DEBUG("StripCluster["
                    << i << "]: " << ef_stripClusters.at(i).localPosition
                    << ", " << ef_stripClusters.at(i).localCovariance << ", "
                    << ef_stripClusters.at(i).idHash << ", "
                    << ef_stripClusters.at(i).id << ", "
                    << ef_stripClusters.at(i).globalPosition[0] << ", "
                    << ef_stripClusters.at(i).globalPosition[1] << ", "
                    << ef_stripClusters.at(i).globalPosition[2] << ", "
                    << ef_stripClusters.at(i).rdoList[0] << ", "
                    << ef_stripClusters.at(i).channelsInPhi);
    }
  }

  if (m_runSW) {
    ATH_CHECK(runSW(ef_stripClusters, ef_pixelClusters, ef_stripSpacePoints,
                    stripsp_meas, ef_pixelSpacePoints, pixelsp_meas, ctx));
  } else {
    ATH_CHECK(runHW(ef_stripClusters, ef_pixelClusters, ef_stripSpacePoints,
                    stripsp_meas, ef_pixelSpacePoints, pixelsp_meas));
  }

  return StatusCode::SUCCESS;
}

StatusCode DataPreparationPipeline::getInputClusterData(
    const xAOD::StripClusterContainer *sc,
    std::vector<EFTrackingDataFormats::StripCluster> &ef_sc,
    long unsigned int N) const {
  if (N > sc->size()) {
    ATH_MSG_ERROR("You want to get the "
                  << N << "th strip cluster, but there are only " << sc->size()
                  << " strip clusters in the container.");
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Making vector of strip clusters...");
  for (long unsigned int i = 0; i < N; i++) {
    EFTrackingDataFormats::StripCluster cache;
    // Get the data from the input xAOD::StripClusterContainer and set it to the
    // cache
    cache.localPosition = sc->at(i)->localPosition<1>()(0, 0);
    cache.localCovariance = sc->at(i)->localCovariance<1>()(0, 0);
    cache.idHash = sc->at(i)->identifierHash();
    cache.id = sc->at(i)->identifier();
    cache.globalPosition[0] = sc->at(i)->globalPosition()[0];
    cache.globalPosition[1] = sc->at(i)->globalPosition()[1];
    cache.globalPosition[2] = sc->at(i)->globalPosition()[2];

    for (long unsigned int j = 0; j < sc->at(i)->rdoList().size(); j++) {
      cache.rdoList[j] = sc->at(i)->rdoList().at(j).get_compact();
    }

    cache.channelsInPhi = sc->at(i)->channelsInPhi();
    cache.sizeOfRDOList = sc->at(i)->rdoList().size();

    ef_sc.push_back(cache);
  }

  ATH_MSG_DEBUG("Made " << ef_sc.size() << " strip clusters in the vector");
  return StatusCode::SUCCESS;
}

StatusCode DataPreparationPipeline::getInputClusterData(
    const xAOD::PixelClusterContainer *pc,
    std::vector<EFTrackingDataFormats::PixelCluster> &ef_pc,
    long unsigned int N) const {
  if (N > pc->size()) {
    ATH_MSG_ERROR("You want to get the "
                  << N << "th pixel cluster, but there are only " << pc->size()
                  << " pixel clusters in the container.");
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Making vector of pixel clusters...");
  for (long unsigned int i = 0; i < N; i++) {
    EFTrackingDataFormats::PixelCluster cache;
    // Get the data from the input xAOD::PixelClusterContainer and set it to the
    // cache
    cache.id = pc->at(i)->identifier();
    cache.idHash = pc->at(i)->identifierHash();
    cache.localPosition[0] = pc->at(i)->localPosition<2>()(0, 0);
    cache.localPosition[1] = pc->at(i)->localPosition<2>()(1, 0);
    cache.localCovariance[0] = pc->at(i)->localCovariance<2>()(0, 0);
    cache.localCovariance[1] = pc->at(i)->localCovariance<2>()(1, 0);
    cache.globalPosition[0] = pc->at(i)->globalPosition()[0];
    cache.globalPosition[1] = pc->at(i)->globalPosition()[1];
    cache.globalPosition[2] = pc->at(i)->globalPosition()[2];

    for (long unsigned int j = 0; j < pc->at(i)->rdoList().size(); j++) {
      cache.rdoList[j] = pc->at(i)->rdoList().at(j).get_compact();
    }

    cache.channelsInPhi = pc->at(i)->channelsInPhi();
    cache.channelsInEta = pc->at(i)->channelsInEta();
    cache.widthInEta = pc->at(i)->widthInEta();
    cache.omegaX = pc->at(i)->omegaX();
    cache.omegaY = pc->at(i)->omegaY();

    for (long unsigned int j = 0; j < pc->at(i)->totList().size(); j++) {
      cache.totList[j] = pc->at(i)->totList().at(j);
    }

    cache.totalToT = pc->at(i)->totalToT();

    for (long unsigned int j = 0; j < pc->at(i)->chargeList().size(); j++) {
      cache.chargeList[j] = pc->at(i)->chargeList().at(j);
    }

    cache.totalCharge = pc->at(i)->totalCharge();
    cache.energyLoss = pc->at(i)->energyLoss();
    cache.isSplit = pc->at(i)->isSplit();
    cache.splitProbability1 = pc->at(i)->splitProbability1();
    cache.splitProbability2 = pc->at(i)->splitProbability2();
    cache.lvl1a = pc->at(i)->lvl1a();
    cache.sizeOfRDOList = pc->at(i)->rdoList().size();
    cache.sizeOfTotList = pc->at(i)->totList().size();
    cache.sizeOfChargeList = pc->at(i)->chargeList().size();

    ef_pc.push_back(cache);
  }

  ATH_MSG_DEBUG("Made " << ef_pc.size() << " pixel clusters in the vector");
  return StatusCode::SUCCESS;
}

StatusCode DataPreparationPipeline::getInputSpacePointData(
    const xAOD::SpacePointContainer *sp,
    std::vector<EFTrackingDataFormats::SpacePoint> &ef_sp,
    std::vector<std::vector<const xAOD::UncalibratedMeasurement *>> &sp_meas,
    long unsigned int N, bool isStrip) const {
  if (N > sp->size()) {
    ATH_MSG_ERROR("You want to get the "
                  << N << "th space point, but there are only " << sp->size()
                  << " SpacePoint in the container.");
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Making vector of space point...");
  for (long unsigned int i = 0; i < N; i++) {
    EFTrackingDataFormats::SpacePoint cache;
    // Get the data from the input xAOD::SpacePointContainer and set it to the
    // cache
    cache.idHash[0] = sp->at(i)->elementIdList()[0];
    cache.globalPosition[0] = sp->at(i)->x();
    cache.globalPosition[1] = sp->at(i)->y();
    cache.globalPosition[2] = sp->at(i)->z();
    cache.radius = sp->at(i)->radius();
    cache.cov_r = sp->at(i)->varianceR();
    cache.cov_z = sp->at(i)->varianceZ();

    // Pass the uncalibrated measurement for later stage of xAOD container
    // creation for spacepoint
    std::vector<const xAOD::UncalibratedMeasurement *> temp_vec(
        sp->at(i)->measurements().size());
    std::copy(sp->at(i)->measurements().begin(),
              sp->at(i)->measurements().end(), temp_vec.begin());

    sp_meas.push_back(temp_vec);

    if (isStrip) {
      cache.idHash[1] = sp->at(i)->elementIdList()[1];
      cache.topHalfStripLength = sp->at(i)->topHalfStripLength();
      cache.bottomHalfStripLength = sp->at(i)->bottomHalfStripLength();
      std::copy(sp->at(i)->topStripDirection().data(),
                sp->at(i)->topStripDirection().data() +
                    sp->at(i)->topStripDirection().size(),
                cache.topStripDirection);
      std::copy(sp->at(i)->bottomStripDirection().data(),
                sp->at(i)->bottomStripDirection().data() +
                    sp->at(i)->bottomStripDirection().size(),
                cache.bottomStripDirection);
      std::copy(sp->at(i)->stripCenterDistance().data(),
                sp->at(i)->stripCenterDistance().data() +
                    sp->at(i)->stripCenterDistance().size(),
                cache.stripCenterDistance);
      std::copy(sp->at(i)->topStripCenter().data(),
                sp->at(i)->topStripCenter().data() +
                    sp->at(i)->topStripCenter().size(),
                cache.topStripCenter);
    }
    ef_sp.push_back(cache);
  }

  ATH_MSG_DEBUG("Made " << ef_sp.size() << " space points in the vector");
  return StatusCode::SUCCESS;
}

StatusCode DataPreparationPipeline::runSW(
    const std::vector<EFTrackingDataFormats::StripCluster> &ef_sc,
    const std::vector<EFTrackingDataFormats::PixelCluster> &ef_pc,
    const std::vector<EFTrackingDataFormats::SpacePoint> &ef_ssp,
    const std::vector<std::vector<const xAOD::UncalibratedMeasurement *>>
        &ssp_mes,
    const std::vector<EFTrackingDataFormats::SpacePoint> &ef_psp,
    const std::vector<std::vector<const xAOD::UncalibratedMeasurement *>>
        &psp_mes,
    const EventContext &ctx) const {
  ATH_MSG_DEBUG("Running the software version of the kernel");

  std::unique_ptr<EFTrackingDataFormats::Metadata> metadata =
      std::make_unique<EFTrackingDataFormats::Metadata>();
  // Strip cluster
  std::vector<float> scLocalPosition(MAX_CLUSTER_NUM);
  std::vector<float> scLocalCovariance(MAX_CLUSTER_NUM);
  std::vector<unsigned int> scIdHash(MAX_CLUSTER_NUM);
  std::vector<long unsigned int> scId(MAX_CLUSTER_NUM);
  std::vector<float> scGlobalPosition(MAX_CLUSTER_NUM * 3);
  std::vector<unsigned long long> scRdoList(MAX_CLUSTER_NUM * 1000);
  std::vector<int> scChannelsInPhi(MAX_CLUSTER_NUM);
  EFTrackingDataFormats::StripClusterOutput ef_scOutput;
  ef_scOutput.scLocalPosition = scLocalPosition.data();
  ef_scOutput.scLocalCovariance = scLocalCovariance.data();
  ef_scOutput.scIdHash = scIdHash.data();
  ef_scOutput.scId = scId.data();
  ef_scOutput.scGlobalPosition = scGlobalPosition.data();
  ef_scOutput.scRdoList = scRdoList.data();
  ef_scOutput.scChannelsInPhi = scChannelsInPhi.data();

  // Pixel cluster
  std::vector<float> pcLocalPosition(MAX_CLUSTER_NUM * 2);
  std::vector<float> pcLocalCovariance(MAX_CLUSTER_NUM * 2);
  std::vector<unsigned int> pcIdHash(MAX_CLUSTER_NUM);
  std::vector<long unsigned int> pcId(MAX_CLUSTER_NUM);
  std::vector<float> pcGlobalPosition(MAX_CLUSTER_NUM * 3);
  std::vector<unsigned long long> pcRdoList(MAX_CLUSTER_NUM);
  std::vector<int> pcChannelsInPhi(MAX_CLUSTER_NUM);
  std::vector<int> pcChannelsInEta(MAX_CLUSTER_NUM);
  std::vector<float> pcWidthInEta(MAX_CLUSTER_NUM);
  std::vector<float> pcOmegaX(MAX_CLUSTER_NUM);
  std::vector<float> pcOmegaY(MAX_CLUSTER_NUM);
  std::vector<int> pcTotList(MAX_CLUSTER_NUM);
  std::vector<int> pcTotalToT(MAX_CLUSTER_NUM);
  std::vector<float> pcChargeList(MAX_CLUSTER_NUM);
  std::vector<float> pcTotalCharge(MAX_CLUSTER_NUM);
  std::vector<float> pcEnergyLoss(MAX_CLUSTER_NUM);
  std::vector<char> pcIsSplit(MAX_CLUSTER_NUM);
  std::vector<float> pcSplitProbability1(MAX_CLUSTER_NUM);
  std::vector<float> pcSplitProbability2(MAX_CLUSTER_NUM);
  std::vector<int> pcLvl1a(MAX_CLUSTER_NUM);
  EFTrackingDataFormats::PixelClusterOutput ef_pcOutput;
  ef_pcOutput.pcLocalPosition = pcLocalPosition.data();
  ef_pcOutput.pcLocalCovariance = pcLocalCovariance.data();
  ef_pcOutput.pcIdHash = pcIdHash.data();
  ef_pcOutput.pcId = pcId.data();
  ef_pcOutput.pcGlobalPosition = pcGlobalPosition.data();
  ef_pcOutput.pcRdoList = pcRdoList.data();
  ef_pcOutput.pcChannelsInPhi = pcChannelsInPhi.data();
  ef_pcOutput.pcChannelsInEta = pcChannelsInEta.data();
  ef_pcOutput.pcWidthInEta = pcWidthInEta.data();
  ef_pcOutput.pcOmegaX = pcOmegaX.data();
  ef_pcOutput.pcOmegaY = pcOmegaY.data();
  ef_pcOutput.pcTotList = pcTotList.data();
  ef_pcOutput.pcTotalToT = pcTotalToT.data();
  ef_pcOutput.pcChargeList = pcChargeList.data();
  ef_pcOutput.pcTotalCharge = pcTotalCharge.data();
  ef_pcOutput.pcEnergyLoss = pcEnergyLoss.data();
  ef_pcOutput.pcIsSplit = pcIsSplit.data();
  ef_pcOutput.pcSplitProbability1 = pcSplitProbability1.data();
  ef_pcOutput.pcSplitProbability2 = pcSplitProbability2.data();
  ef_pcOutput.pcLvl1a = pcLvl1a.data();

  // Strip SpacePoint
  std::vector<unsigned int> sspIdHash(MAX_SPACEPOINT_NUM * 2);
  std::vector<float> sspGlobalPosition(MAX_SPACEPOINT_NUM * 3);
  std::vector<float> sspRadius(MAX_SPACEPOINT_NUM);
  std::vector<float> sspVarianceR(MAX_SPACEPOINT_NUM);
  std::vector<float> sspVarianceZ(MAX_SPACEPOINT_NUM);
  std::vector<int> sspMeasurementIndexes(MAX_SPACEPOINT_NUM * 100);
  std::vector<float> sspTopHalfStripLength(MAX_SPACEPOINT_NUM);
  std::vector<float> sspBottomHalfStripLength(MAX_SPACEPOINT_NUM);
  std::vector<float> sspTopStripDirection(MAX_SPACEPOINT_NUM * 3);
  std::vector<float> sspBottomStripDirection(MAX_SPACEPOINT_NUM * 3);
  std::vector<float> sspStripCenterDistance(MAX_SPACEPOINT_NUM * 3);
  std::vector<float> sspTopStripCenter(MAX_SPACEPOINT_NUM * 3);
  EFTrackingDataFormats::SpacePointOutput ef_sspOutput;
  ef_sspOutput.spIdHash = sspIdHash.data();
  ef_sspOutput.spGlobalPosition = sspGlobalPosition.data();
  ef_sspOutput.spRadius = sspRadius.data();
  ef_sspOutput.spVarianceR = sspVarianceR.data();
  ef_sspOutput.spVarianceZ = sspVarianceZ.data();
  ef_sspOutput.spMeasurementIndexes = sspMeasurementIndexes.data();
  ef_sspOutput.spTopHalfStripLength = sspTopHalfStripLength.data();
  ef_sspOutput.spBottomHalfStripLength = sspBottomHalfStripLength.data();
  ef_sspOutput.spTopStripDirection = sspTopStripDirection.data();
  ef_sspOutput.spBottomStripDirection = sspBottomStripDirection.data();
  ef_sspOutput.spStripCenterDistance = sspStripCenterDistance.data();
  ef_sspOutput.spTopStripCenter = sspTopStripCenter.data();

  // Pixel SpacePoint
  std::vector<unsigned int> pspIdHash(MAX_SPACEPOINT_NUM);
  std::vector<float> pspGlobalPosition(MAX_SPACEPOINT_NUM * 3);
  std::vector<float> pspRadius(MAX_SPACEPOINT_NUM);
  std::vector<float> pspVarianceR(MAX_SPACEPOINT_NUM);
  std::vector<float> pspVarianceZ(MAX_SPACEPOINT_NUM);
  std::vector<int> pspMeasurementIndexes(MAX_SPACEPOINT_NUM * 100);
  EFTrackingDataFormats::SpacePointOutput ef_pspOutput;
  ef_pspOutput.spIdHash = pspIdHash.data();
  ef_pspOutput.spGlobalPosition = pspGlobalPosition.data();
  ef_pspOutput.spRadius = pspRadius.data();
  ef_pspOutput.spVarianceR = pspVarianceR.data();
  ef_pspOutput.spVarianceZ = pspVarianceZ.data();
  ef_pspOutput.spMeasurementIndexes = pspMeasurementIndexes.data();

  ATH_CHECK(transferSW(ef_sc, ef_scOutput, ef_pc, ef_pcOutput, ef_ssp,
                       ef_sspOutput, ef_psp, ef_pspOutput, metadata.get()));

  // resize the vector to be the length of the cluster
  scLocalPosition.resize(metadata->numOfStripClusters);
  scLocalCovariance.resize(metadata->numOfStripClusters);
  scIdHash.resize(metadata->numOfStripClusters);
  scId.resize(metadata->numOfStripClusters);
  scGlobalPosition.resize(metadata->numOfStripClusters * 3);
  scRdoList.resize(metadata->scRdoIndexSize);
  scChannelsInPhi.resize(metadata->numOfStripClusters);

  pcLocalPosition.resize(metadata->numOfPixelClusters * 2);
  pcLocalCovariance.resize(metadata->numOfPixelClusters * 2);
  pcIdHash.resize(metadata->numOfPixelClusters);
  pcId.resize(metadata->numOfPixelClusters);
  pcGlobalPosition.resize(metadata->numOfPixelClusters * 3);
  pcRdoList.resize(metadata->pcRdoIndexSize);
  pcChannelsInPhi.resize(metadata->numOfPixelClusters);
  pcChannelsInEta.resize(metadata->numOfPixelClusters);
  pcWidthInEta.resize(metadata->numOfPixelClusters);
  pcOmegaX.resize(metadata->numOfPixelClusters);
  pcOmegaY.resize(metadata->numOfPixelClusters);
  pcTotList.resize(metadata->pcTotIndexSize);
  pcTotalToT.resize(metadata->numOfPixelClusters);
  pcChargeList.resize(metadata->pcChargeIndexSize);
  pcTotalCharge.resize(metadata->numOfPixelClusters);
  pcEnergyLoss.resize(metadata->numOfPixelClusters);
  pcIsSplit.resize(metadata->numOfPixelClusters);
  pcSplitProbability1.resize(metadata->numOfPixelClusters);
  pcSplitProbability2.resize(metadata->numOfPixelClusters);
  pcLvl1a.resize(metadata->numOfPixelClusters);

  // resize the vector to be the length of the space point
  sspIdHash.resize(metadata->numOfStripSpacePoints * 2);
  sspGlobalPosition.resize(metadata->numOfStripSpacePoints * 3);
  sspRadius.resize(metadata->numOfStripSpacePoints);
  sspVarianceR.resize(metadata->numOfStripSpacePoints);
  sspVarianceZ.resize(metadata->numOfStripSpacePoints);
  sspTopHalfStripLength.resize(metadata->numOfStripSpacePoints);
  sspBottomHalfStripLength.resize(metadata->numOfStripSpacePoints);
  sspTopStripDirection.resize(metadata->numOfStripSpacePoints * 3);
  sspBottomStripDirection.resize(metadata->numOfStripSpacePoints * 3);
  sspStripCenterDistance.resize(metadata->numOfStripSpacePoints * 3);
  sspTopStripCenter.resize(metadata->numOfStripSpacePoints * 3);

  pspIdHash.resize(metadata->numOfPixelSpacePoints);
  pspGlobalPosition.resize(metadata->numOfPixelSpacePoints * 3);
  pspRadius.resize(metadata->numOfPixelSpacePoints);
  pspVarianceR.resize(metadata->numOfPixelSpacePoints);
  pspVarianceZ.resize(metadata->numOfPixelSpacePoints);

  // print all strip clusters
  for (unsigned i = 0; i < metadata->numOfStripClusters; i++) {
    ATH_MSG_DEBUG("scLocalPosition["
                  << i << "] = " << ef_scOutput.scLocalPosition[i]);
    ATH_MSG_DEBUG("scLocalCovariance["
                  << i << "] = " << ef_scOutput.scLocalCovariance[i]);
    ATH_MSG_DEBUG("scIdHash[" << i << "] = " << ef_scOutput.scIdHash[i]);
    ATH_MSG_DEBUG("scId[" << i << "] = " << ef_scOutput.scId[i]);
    ATH_MSG_DEBUG("scGlobalPosition["
                  << i << "] = " << ef_scOutput.scGlobalPosition[i * 3] << ", "
                  << ef_scOutput.scGlobalPosition[i * 3 + 1] << ", "
                  << ef_scOutput.scGlobalPosition[i * 3 + 2]);
    ATH_MSG_DEBUG("scRdoList[" << i << "] = " << ef_scOutput.scRdoList[i]);
    ATH_MSG_DEBUG("scChannelsInPhi["
                  << i << "] = " << ef_scOutput.scChannelsInPhi[i]);
  }

  // print all pixel clusters
  for (unsigned i = 0; i < metadata->numOfPixelClusters; i++) {

    ATH_MSG_DEBUG("pcLocalPosition["
                  << i << "] = " << ef_pcOutput.pcLocalPosition[i * 2] << ", "
                  << ef_pcOutput.pcLocalPosition[i * 2 + 1]);
    ATH_MSG_DEBUG("pcLocalCovariance["
                  << i << "] = " << ef_pcOutput.pcLocalCovariance[i * 2] << ", "
                  << ef_pcOutput.pcLocalCovariance[i * 2 + 1]);
    ATH_MSG_DEBUG("pcIdHash[" << i << "] = " << ef_pcOutput.pcIdHash[i]);
    ATH_MSG_DEBUG("pcId[" << i << "] = " << pcId[i]);
    ATH_MSG_DEBUG("pcGlobalPosition["
                  << i << "] = " << ef_pcOutput.pcGlobalPosition[i * 3] << ", "
                  << ef_pcOutput.pcGlobalPosition[i * 3 + 1] << ", "
                  << ef_pcOutput.pcGlobalPosition[i * 3 + 2]);
    // ATH_MSG_DEBUG("pcRdoList[" << i << "] = " << pcRdoList[i]);
    ATH_MSG_DEBUG("pcChannelsInPhi["
                  << i << "] = " << ef_pcOutput.pcChannelsInPhi[i]);
    ATH_MSG_DEBUG("pcChannelsInEta["
                  << i << "] = " << ef_pcOutput.pcChannelsInEta[i]);
    ATH_MSG_DEBUG("pcWidthInEta[" << i
                                  << "] = " << ef_pcOutput.pcWidthInEta[i]);
    ATH_MSG_DEBUG("pcOmegaX[" << i << "] = " << ef_pcOutput.pcOmegaX[i]);
    ATH_MSG_DEBUG("pcOmegaY[" << i << "] = " << ef_pcOutput.pcOmegaY[i]);
    // ATH_MSG_DEBUG("pcTotList[" << i << "] = " << pcTotList[i]);
    ATH_MSG_DEBUG("pcTotalToT[" << i << "] = " << ef_pcOutput.pcTotalToT[i]);
    // ATH_MSG_DEBUG("pcChargeList[" << i << "] = " << pcChargeList[i]);
    ATH_MSG_DEBUG("pcTotalCharge[" << i
                                   << "] = " << ef_pcOutput.pcTotalCharge[i]);
    ATH_MSG_DEBUG("pcEnergyLoss[" << i
                                  << "] = " << ef_pcOutput.pcEnergyLoss[i]);
    ATH_MSG_DEBUG("pcIsSplit[" << i << "] = " << ef_pcOutput.pcIsSplit[i]);
    ATH_MSG_DEBUG("pcSplitProbability1["
                  << i << "] = " << ef_pcOutput.pcSplitProbability1[i]);
    ATH_MSG_DEBUG("pcSplitProbability2["
                  << i << "] = " << ef_pcOutput.pcSplitProbability2[i]);
    ATH_MSG_DEBUG("pcLvl1a[" << i << "] = " << ef_pcOutput.pcLvl1a[i]);
  }

  // print all Strip space points
  for (unsigned i = 0; i < metadata->numOfStripSpacePoints; i++) {
    ATH_MSG_DEBUG("sspIdHash[" << i << "] = " << ef_sspOutput.spIdHash[i * 2]
                               << ", " << ef_sspOutput.spIdHash[i * 2 + 1]);
    ATH_MSG_DEBUG("sspGlobalPosition["
                  << i << "] = " << ef_sspOutput.spGlobalPosition[i * 3] << ", "
                  << ef_sspOutput.spGlobalPosition[i * 3 + 1] << ", "
                  << ef_sspOutput.spGlobalPosition[i * 3 + 2]);
    ATH_MSG_DEBUG("sspRadius[" << i << "] = " << ef_sspOutput.spRadius[i]);
    ATH_MSG_DEBUG("sspVarianceR[" << i
                                  << "] = " << ef_sspOutput.spVarianceR[i]);
    ATH_MSG_DEBUG("sspVarianceZ[" << i
                                  << "] = " << ef_sspOutput.spVarianceZ[i]);
    ATH_MSG_DEBUG("sspTopHalfStripLength["
                  << i << "] = " << ef_sspOutput.spTopHalfStripLength[i]);
    ATH_MSG_DEBUG("sspBottomHalfStripLength["
                  << i << "] = " << ef_sspOutput.spBottomHalfStripLength[i]);
    ATH_MSG_DEBUG("sspTopStripDirection["
                  << i << "] = " << ef_sspOutput.spTopStripDirection[i * 3]
                  << ", " << ef_sspOutput.spTopStripDirection[i * 3 + 1] << ", "
                  << ef_sspOutput.spTopStripDirection[i * 3 + 2]);
    ATH_MSG_DEBUG("sspBottomStripDirection["
                  << i << "] = " << ef_sspOutput.spBottomStripDirection[i * 3]
                  << ", " << ef_sspOutput.spBottomStripDirection[i * 3 + 1]
                  << ", " << ef_sspOutput.spBottomStripDirection[i * 3 + 2]);
    ATH_MSG_DEBUG("sspStripCenterDistance["
                  << i << "] = " << ef_sspOutput.spStripCenterDistance[i * 3]
                  << ", " << ef_sspOutput.spStripCenterDistance[i * 3 + 1]
                  << ", " << ef_sspOutput.spStripCenterDistance[i * 3 + 2]);
    ATH_MSG_DEBUG("sspTopStripCenter["
                  << i << "] = " << ef_sspOutput.spTopStripCenter[i * 3] << ", "
                  << ef_sspOutput.spTopStripCenter[i * 3 + 1] << ", "
                  << ef_sspOutput.spTopStripCenter[i * 3 + 2]);
  }

  // print all Pixel space points
  for (unsigned i = 0; i < metadata->numOfPixelSpacePoints; i++) {
    ATH_MSG_DEBUG("pspIdHash[" << i << "] = " << ef_pspOutput.spIdHash[i]);
    ATH_MSG_DEBUG("pspGlobalPosition["
                  << i << "] = " << ef_pspOutput.spGlobalPosition[i * 3] << ", "
                  << ef_pspOutput.spGlobalPosition[i * 3 + 1] << ", "
                  << ef_pspOutput.spGlobalPosition[i * 3 + 2]);
    ATH_MSG_DEBUG("pspRadius[" << i << "] = " << ef_pspOutput.spRadius[i]);
    ATH_MSG_DEBUG("pspVarianceR[" << i
                                  << "] = " << ef_pspOutput.spVarianceR[i]);
    ATH_MSG_DEBUG("pspVarianceZ[" << i
                                  << "] = " << ef_pspOutput.spVarianceZ[i]);
  }

  // Group data to make the strip cluster container
  EFTrackingDataFormats::StripClusterAuxInput scAux;
  scAux.localPosition = scLocalPosition;
  scAux.localCovariance = scLocalCovariance;
  scAux.idHash = scIdHash;
  scAux.id = scId;
  scAux.globalPosition = scGlobalPosition;
  scAux.rdoList = scRdoList;
  scAux.channelsInPhi = scChannelsInPhi;

  ATH_CHECK(m_xAODContainerMaker->makeStripClusterContainer(
      ef_sc.size(), scAux, metadata.get(), ctx));

  // Group data to make the pixel cluster container
  EFTrackingDataFormats::PixelClusterAuxInput pxAux;
  pxAux.id = pcId;
  pxAux.idHash = pcIdHash;
  pxAux.localPosition = pcLocalPosition;
  pxAux.localCovariance = pcLocalCovariance;
  pxAux.globalPosition = pcGlobalPosition;
  pxAux.rdoList = pcRdoList;
  pxAux.channelsInPhi = pcChannelsInPhi;
  pxAux.channelsInEta = pcChannelsInEta;
  pxAux.widthInEta = pcWidthInEta;
  pxAux.omegaX = pcOmegaX;
  pxAux.omegaY = pcOmegaY;
  pxAux.totList = pcTotList;
  pxAux.totalToT = pcTotalToT;
  pxAux.chargeList = pcChargeList;
  pxAux.totalCharge = pcTotalCharge;
  pxAux.energyLoss = pcEnergyLoss;
  pxAux.isSplit = pcIsSplit;
  pxAux.splitProbability1 = pcSplitProbability1;
  pxAux.splitProbability2 = pcSplitProbability2;
  pxAux.lvl1a = pcLvl1a;

  ATH_CHECK(m_xAODContainerMaker->makePixelClusterContainer(
      ef_pc.size(), pxAux, metadata.get(), ctx));

  // Group data to make the strip space point container
  EFTrackingDataFormats::SpacePointAuxInput sspAux;
  sspAux.elementIdList = sspIdHash;
  sspAux.globalPosition = sspGlobalPosition;
  sspAux.radius = sspRadius;
  sspAux.varianceR = sspVarianceR;
  sspAux.varianceZ = sspVarianceZ;
  sspAux.measurementIndexes = sspMeasurementIndexes;
  sspAux.topHalfStripLength = sspTopHalfStripLength;
  sspAux.bottomHalfStripLength = sspBottomHalfStripLength;
  sspAux.topStripDirection = sspTopStripDirection;
  sspAux.bottomStripDirection = sspBottomStripDirection;
  sspAux.stripCenterDistance = sspStripCenterDistance;
  sspAux.topStripCenter = sspTopStripCenter;

  ATH_CHECK(m_xAODContainerMaker->makeStripSpacePointContainer(
      ef_ssp.size(), sspAux, ssp_mes, ctx));

  // Group data to make the pixel space point container
  EFTrackingDataFormats::SpacePointAuxInput pspAux;
  pspAux.elementIdList = pspIdHash;
  pspAux.globalPosition = pspGlobalPosition;
  pspAux.radius = pspRadius;
  pspAux.varianceR = pspVarianceR;
  pspAux.varianceZ = pspVarianceZ;
  pspAux.measurementIndexes = pspMeasurementIndexes;

  ATH_CHECK(m_xAODContainerMaker->makePixelSpacePointContainer(
      ef_psp.size(), pspAux, psp_mes, ctx));

  // validate output container
  SG::ReadHandle<xAOD::StripClusterContainer> outputStripClusters(
      "ITkStripClusters");
  // print the local position
  for (unsigned i = 0; i < metadata->numOfStripClusters; i++) {
    ATH_MSG_DEBUG("outputStripClusters["
                  << i << "]->localPosition<1>()(0, 0) = "
                  << outputStripClusters->at(i)->localPosition<1>());
  }
  // validate output container
  SG::ReadHandle<xAOD::PixelClusterContainer> outputPixelClusters(
      "ITkPixelClusters");
  // print the local position
  for (unsigned i = 0; i < metadata->numOfPixelClusters; i++) {
    ATH_MSG_DEBUG("outputPixelClusters["
                  << i << "]->localPosition<2>()(0, 0) = "
                  << outputPixelClusters->at(i)->localPosition<2>()(0, 0));
  }

  return StatusCode::SUCCESS;
}

// This function is still in protoype and should be further discussed
StatusCode DataPreparationPipeline::runHW(
    const std::vector<EFTrackingDataFormats::StripCluster> &ef_sc,
    const std::vector<EFTrackingDataFormats::PixelCluster> &ef_pc,
    const std::vector<EFTrackingDataFormats::SpacePoint> &ef_ssp,
    const std::vector<std::vector<const xAOD::UncalibratedMeasurement *>>
        &ssp_mes,
    const std::vector<EFTrackingDataFormats::SpacePoint> &ef_psp,
    const std::vector<std::vector<const xAOD::UncalibratedMeasurement *>>
        &psp_mes) const {
  ATH_MSG_DEBUG("Running on the hardware");

  // Prepare host pointers for kernel output
  float *scLocalPosition;
  float *scLocalCovariance;
  unsigned int *scIdHash;
  long unsigned int *scId;
  float *scGlobalPosition;
  unsigned long long *scRdoList;
  int *scChannelsInPhi;

  float *pcLocalPosition;
  float *pcLocalCovariance;
  unsigned int *pcIdHash;
  long unsigned int *pcId;
  float *pcGlobalPosition;
  unsigned long long *pcRdoList;
  int *pcChannelsInPhi;
  int *pcChannelsInEta;
  float *pcWidthInEta;
  float *pcOmegaX;
  float *pcOmegaY;
  int *pcTotList;
  int *pcTotalToT;
  float *pcChargeList;
  float *pcTotalCharge;
  float *pcEnergyLoss;
  char *pcIsSplit;
  float *pcSplitProbability1;
  float *pcSplitProbability2;
  int *pcLvl1a;

  EFTrackingDataFormats::Metadata *metadata;

  // Assign memory to the pointers which is 4K aligned
  // 4K alignment is required to avoid extra memory copy from the host to the
  // device
  posix_memalign((void **)&scLocalPosition, 4096,
                 sizeof(float) * MAX_CLUSTER_NUM);
  posix_memalign((void **)&scLocalCovariance, 4096,
                 sizeof(float) * MAX_CLUSTER_NUM);
  posix_memalign((void **)&scIdHash, 4096,
                 sizeof(unsigned int) * MAX_CLUSTER_NUM);
  posix_memalign((void **)&scId, 4096,
                 sizeof(long unsigned int) * MAX_CLUSTER_NUM);
  posix_memalign((void **)&scGlobalPosition, 4096,
                 sizeof(float) * MAX_CLUSTER_NUM * 3);
  posix_memalign((void **)&scRdoList, 4096,
                 sizeof(unsigned long long) * MAX_CLUSTER_NUM);
  posix_memalign((void **)&scChannelsInPhi, 4096,
                 sizeof(int) * MAX_CLUSTER_NUM);

  posix_memalign((void **)&pcLocalPosition, 4096,
                 sizeof(float) * MAX_CLUSTER_NUM * 2);
  posix_memalign((void **)&pcLocalCovariance, 4096,
                 sizeof(float) * MAX_CLUSTER_NUM * 2);
  posix_memalign((void **)&pcIdHash, 4096,
                 sizeof(unsigned int) * MAX_CLUSTER_NUM);
  posix_memalign((void **)&pcId, 4096,
                 sizeof(long unsigned int) * MAX_CLUSTER_NUM);
  posix_memalign((void **)&pcGlobalPosition, 4096,
                 sizeof(float) * MAX_CLUSTER_NUM * 3);
  posix_memalign((void **)&pcRdoList, 4096,
                 sizeof(unsigned long long) * MAX_CLUSTER_NUM);
  posix_memalign((void **)&pcChannelsInPhi, 4096,
                 sizeof(int) * MAX_CLUSTER_NUM);
  posix_memalign((void **)&pcChannelsInEta, 4096,
                 sizeof(int) * MAX_CLUSTER_NUM);
  posix_memalign((void **)&pcWidthInEta, 4096, sizeof(float) * MAX_CLUSTER_NUM);
  posix_memalign((void **)&pcOmegaX, 4096, sizeof(float) * MAX_CLUSTER_NUM);
  posix_memalign((void **)&pcOmegaY, 4096, sizeof(float) * MAX_CLUSTER_NUM);
  posix_memalign((void **)&pcTotList, 4096, sizeof(int) * MAX_CLUSTER_NUM);
  posix_memalign((void **)&pcTotalToT, 4096, sizeof(int) * MAX_CLUSTER_NUM);
  posix_memalign((void **)&pcChargeList, 4096, sizeof(float) * MAX_CLUSTER_NUM);
  posix_memalign((void **)&pcTotalCharge, 4096,
                 sizeof(float) * MAX_CLUSTER_NUM);
  posix_memalign((void **)&pcEnergyLoss, 4096, sizeof(float) * MAX_CLUSTER_NUM);
  posix_memalign((void **)&pcIsSplit, 4096, sizeof(char) * MAX_CLUSTER_NUM);
  posix_memalign((void **)&pcSplitProbability1, 4096,
                 sizeof(float) * MAX_CLUSTER_NUM);
  posix_memalign((void **)&pcSplitProbability2, 4096,
                 sizeof(float) * MAX_CLUSTER_NUM);
  posix_memalign((void **)&pcLvl1a, 4096, sizeof(int) * MAX_CLUSTER_NUM);

  posix_memalign((void **)&metadata, 4096,
                 sizeof(EFTrackingDataFormats::Metadata));

  // opencl error code
  cl_int err = 0;

  // Prepare buffers
  ATH_MSG_DEBUG("Preparing buffers...");

  // Input buffers
  cl::Buffer acc_stripClusters(
      m_context, CL_MEM_READ_ONLY,
      sizeof(EFTrackingDataFormats::StripCluster) * ef_sc.size(), NULL, &err);
  cl::Buffer acc_pixelClusters(
      m_context, CL_MEM_READ_ONLY,
      sizeof(EFTrackingDataFormats::PixelCluster) * ef_pc.size(), NULL, &err);

  // Output StripCluster buffers
  cl::Buffer acc_scLocalPosition(
      m_context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
      sizeof(float) * MAX_CLUSTER_NUM, scLocalPosition, &err);
  cl::Buffer acc_scLocalCovariance(
      m_context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
      sizeof(float) * MAX_CLUSTER_NUM, scLocalCovariance, &err);
  cl::Buffer acc_scIdHash(m_context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
                          sizeof(unsigned int) * MAX_CLUSTER_NUM, scIdHash,
                          &err);
  cl::Buffer acc_scId(m_context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
                      sizeof(long unsigned int) * MAX_CLUSTER_NUM, scId, &err);
  cl::Buffer acc_scGlobalPosition(
      m_context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
      sizeof(float) * MAX_CLUSTER_NUM * 3, scGlobalPosition, &err);
  cl::Buffer acc_scRdoList(m_context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
                           sizeof(unsigned long long) * MAX_CLUSTER_NUM,
                           scRdoList, &err);
  cl::Buffer acc_scChannelsInPhi(
      m_context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
      sizeof(int) * MAX_CLUSTER_NUM, scChannelsInPhi, &err);

  // Output PixelCluster buffers
  cl::Buffer acc_pcLocalPosition(
      m_context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
      sizeof(float) * MAX_CLUSTER_NUM * 2, pcLocalPosition, &err);
  cl::Buffer acc_pcLocalCovariance(
      m_context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
      sizeof(float) * MAX_CLUSTER_NUM * 2, pcLocalCovariance, &err);
  cl::Buffer acc_pcIdHash(m_context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
                          sizeof(unsigned int) * MAX_CLUSTER_NUM, pcIdHash,
                          &err);
  cl::Buffer acc_pcId(m_context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
                      sizeof(long unsigned int) * MAX_CLUSTER_NUM, pcId, &err);
  cl::Buffer acc_pcGlobalPosition(
      m_context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
      sizeof(float) * MAX_CLUSTER_NUM * 3, pcGlobalPosition, &err);
  cl::Buffer acc_pcRdoList(m_context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
                           sizeof(unsigned long long) * MAX_CLUSTER_NUM,
                           pcRdoList, &err);
  cl::Buffer acc_pcChannelsInPhi(
      m_context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
      sizeof(int) * MAX_CLUSTER_NUM, pcChannelsInPhi, &err);
  cl::Buffer acc_pcChannelsInEta(
      m_context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
      sizeof(int) * MAX_CLUSTER_NUM, pcChannelsInEta, &err);
  cl::Buffer acc_pcWidthInEta(
      m_context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
      sizeof(float) * MAX_CLUSTER_NUM, pcWidthInEta, &err);
  cl::Buffer acc_pcOmegaX(m_context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
                          sizeof(float) * MAX_CLUSTER_NUM, pcOmegaX, &err);
  cl::Buffer acc_pcOmegaY(m_context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
                          sizeof(float) * MAX_CLUSTER_NUM, pcOmegaY, &err);
  cl::Buffer acc_pcTotList(m_context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
                           sizeof(int) * MAX_CLUSTER_NUM, pcTotList, &err);
  cl::Buffer acc_pcTotalToT(m_context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
                            sizeof(int) * MAX_CLUSTER_NUM, pcTotalToT, &err);
  cl::Buffer acc_pcChargeList(
      m_context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
      sizeof(float) * MAX_CLUSTER_NUM, pcChargeList, &err);
  cl::Buffer acc_pcTotalCharge(
      m_context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
      sizeof(float) * MAX_CLUSTER_NUM, pcTotalCharge, &err);
  cl::Buffer acc_pcEnergyLoss(
      m_context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
      sizeof(float) * MAX_CLUSTER_NUM, pcEnergyLoss, &err);
  cl::Buffer acc_pcIsSplit(m_context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
                           sizeof(char) * MAX_CLUSTER_NUM, pcIsSplit, &err);
  cl::Buffer acc_pcSplitProbability1(
      m_context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
      sizeof(float) * MAX_CLUSTER_NUM, pcSplitProbability1, &err);
  cl::Buffer acc_pcSplitProbability2(
      m_context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
      sizeof(float) * MAX_CLUSTER_NUM, pcSplitProbability2, &err);
  cl::Buffer acc_pcLvl1a(m_context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
                         sizeof(int) * MAX_CLUSTER_NUM, pcLvl1a, &err);

  cl::Buffer acc_metadata(m_context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
                          sizeof(EFTrackingDataFormats::Metadata), metadata,
                          &err);

  // Prepare kernel
  ATH_MSG_DEBUG("Preparing kernel...");
  cl::Kernel acc_kernel(m_program, m_kernelName.value().data(), &err);
  // check error code
  if (err != CL_SUCCESS) {
    ATH_MSG_ERROR("Error making cl::Kernel. Error code: " << err);
    return StatusCode::FAILURE;
  }

  // Set kernel arguments
  ATH_MSG_DEBUG("Setting kernel arguments...");
  // This should be done before any enqueue command such that buffers
  // are link to the proper DDR bank automatically
  acc_kernel.setArg<cl::Buffer>(0, acc_stripClusters);
  acc_kernel.setArg<int>(1, ef_sc.size());
  acc_kernel.setArg<cl::Buffer>(2, acc_scLocalPosition);
  acc_kernel.setArg<cl::Buffer>(3, acc_scLocalCovariance);
  acc_kernel.setArg<cl::Buffer>(4, acc_scIdHash);
  acc_kernel.setArg<cl::Buffer>(5, acc_scId);
  acc_kernel.setArg<cl::Buffer>(6, acc_scGlobalPosition);
  acc_kernel.setArg<cl::Buffer>(7, acc_scRdoList);
  acc_kernel.setArg<cl::Buffer>(8, acc_scChannelsInPhi);
  acc_kernel.setArg<cl::Buffer>(9, acc_pixelClusters);
  acc_kernel.setArg<int>(10, ef_pc.size());
  acc_kernel.setArg<cl::Buffer>(11, acc_pcLocalPosition);
  acc_kernel.setArg<cl::Buffer>(12, acc_pcLocalCovariance);
  acc_kernel.setArg<cl::Buffer>(13, acc_pcIdHash);
  acc_kernel.setArg<cl::Buffer>(14, acc_pcId);
  acc_kernel.setArg<cl::Buffer>(15, acc_pcGlobalPosition);
  acc_kernel.setArg<cl::Buffer>(16, acc_pcRdoList);
  acc_kernel.setArg<cl::Buffer>(17, acc_pcChannelsInPhi);
  acc_kernel.setArg<cl::Buffer>(18, acc_pcChannelsInEta);
  acc_kernel.setArg<cl::Buffer>(19, acc_pcWidthInEta);
  acc_kernel.setArg<cl::Buffer>(20, acc_pcOmegaX);
  acc_kernel.setArg<cl::Buffer>(21, acc_pcOmegaY);
  acc_kernel.setArg<cl::Buffer>(22, acc_pcTotList);
  acc_kernel.setArg<cl::Buffer>(23, acc_pcTotalToT);
  acc_kernel.setArg<cl::Buffer>(24, acc_pcChargeList);
  acc_kernel.setArg<cl::Buffer>(25, acc_pcTotalCharge);
  acc_kernel.setArg<cl::Buffer>(26, acc_pcEnergyLoss);
  acc_kernel.setArg<cl::Buffer>(27, acc_pcIsSplit);
  acc_kernel.setArg<cl::Buffer>(28, acc_pcSplitProbability1);
  acc_kernel.setArg<cl::Buffer>(29, acc_pcSplitProbability2);
  acc_kernel.setArg<cl::Buffer>(30, acc_pcLvl1a);
  acc_kernel.setArg<cl::Buffer>(31, acc_metadata);

  // Make OpenCL command queue for this event
  cl::CommandQueue acc_queue(m_context, m_accelerator);

  ATH_MSG_DEBUG("Writing buffers...");
  // Use CL_MIGRATE_MEM_OBJECT_CONTENT_UNDEFINED to avoid DMA access for output
  // buffers
  err = acc_queue.enqueueMigrateMemObjects(
      {acc_stripClusters, acc_pixelClusters, acc_scLocalPosition,
       acc_scLocalCovariance, acc_scIdHash, acc_scId, acc_scGlobalPosition,
       acc_scRdoList, acc_scChannelsInPhi,
       // pixel
       acc_pcId, acc_pcIdHash, acc_pcLocalPosition, acc_pcLocalCovariance,
       acc_pcGlobalPosition, acc_pcRdoList, acc_pcChannelsInPhi,
       acc_pcChannelsInEta, acc_pcWidthInEta, acc_pcOmegaX, acc_pcOmegaY,
       acc_pcTotList, acc_pcTotalToT, acc_pcChargeList, acc_pcTotalCharge,
       acc_pcEnergyLoss, acc_pcIsSplit, acc_pcSplitProbability1,
       acc_pcSplitProbability2, acc_pcLvl1a, acc_metadata},
      CL_MIGRATE_MEM_OBJECT_CONTENT_UNDEFINED);

  acc_queue.enqueueTask(acc_kernel);

  // read back data
  acc_queue.enqueueMigrateMemObjects(
      {acc_scLocalPosition, acc_scLocalCovariance, acc_scIdHash, acc_scId,
       acc_scGlobalPosition, acc_scRdoList, acc_scChannelsInPhi},
      CL_MIGRATE_MEM_OBJECT_HOST);
  acc_queue.finish();
  // print scLocalPosition
  for (int i = 0; i < 10; i++) {
    ATH_MSG_DEBUG("scLocalPosition[" << i << "] = " << scLocalPosition[i]);
  }

  // Free memory
  free(scLocalPosition);
  free(scLocalCovariance);
  free(scIdHash);
  free(scId);
  free(scGlobalPosition);
  free(scRdoList);
  free(scChannelsInPhi);

  free(pcLocalPosition);
  free(pcLocalCovariance);
  free(pcIdHash);
  free(pcId);
  free(pcGlobalPosition);
  free(pcRdoList);
  free(pcChannelsInPhi);
  free(pcChannelsInEta);
  free(pcWidthInEta);
  free(pcOmegaX);
  free(pcOmegaY);
  free(pcTotList);
  free(pcTotalToT);
  free(pcChargeList);
  free(pcTotalCharge);
  free(pcEnergyLoss);
  free(pcIsSplit);
  free(pcSplitProbability1);
  free(pcSplitProbability2);
  free(pcLvl1a);

  free(metadata);

  return StatusCode::SUCCESS;
}

StatusCode DataPreparationPipeline::transferSW(
    const std::vector<EFTrackingDataFormats::StripCluster> &inputSC,
    EFTrackingDataFormats::StripClusterOutput &ef_scOutput,
    // PixelCluster
    const std::vector<EFTrackingDataFormats::PixelCluster> &inputPC,
    EFTrackingDataFormats::PixelClusterOutput &ef_pcOutput,
    // StripSpacePoint
    const std::vector<EFTrackingDataFormats::SpacePoint> &inputSSP,
    EFTrackingDataFormats::SpacePointOutput &ef_sspOutput,
    // PixelSpacePoint
    const std::vector<EFTrackingDataFormats::SpacePoint> &inputPSP,
    EFTrackingDataFormats::SpacePointOutput &ef_pspOutput,
    // Metadata
    EFTrackingDataFormats::Metadata *metadata) const {
  // return input
  int rdoIndex_counter = 0;

  unsigned int inputscRdoIndexSize = 0;
  unsigned int inputpcRdoIndexSize = 0;

  // transfer inputSC
  // trasnsfer_sc:
  for (size_t i = 0; i < inputSC.size(); i++) {
    ef_scOutput.scLocalPosition[i] = inputSC[i].localPosition;
    ef_scOutput.scLocalCovariance[i] = inputSC[i].localCovariance;
    ef_scOutput.scIdHash[i] = inputSC[i].idHash;
    ef_scOutput.scId[i] = inputSC[i].id;
    ef_scOutput.scGlobalPosition[i * 3] = inputSC[i].globalPosition[0];
    ef_scOutput.scGlobalPosition[i * 3 + 1] = inputSC[i].globalPosition[1];
    ef_scOutput.scGlobalPosition[i * 3 + 2] = inputSC[i].globalPosition[2];

    inputscRdoIndexSize += inputSC[i].sizeOfRDOList;

    for (int j = 0; j < inputSC[i].sizeOfRDOList; j++) {
      ef_scOutput.scRdoList[rdoIndex_counter + j] = inputSC[i].rdoList[j];
    }
    // update the index counter
    ef_scOutput.scChannelsInPhi[i] = inputSC[i].channelsInPhi;
    rdoIndex_counter += inputSC[i].sizeOfRDOList;
    metadata[0].scRdoIndex[i] = inputSC[i].sizeOfRDOList;
  }

  // transfer inputPC
  rdoIndex_counter = 0;
  int totIndex_counter = 0;
  int chargeIndex_counter = 0;

  unsigned int inputpcTotListsize = 0;
  unsigned int inputpcChargeListsize = 0;

  // trasnsfer_pc:
  for (size_t i = 0; i < inputPC.size(); i++) {
    ef_pcOutput.pcLocalPosition[i * 2] = inputPC[i].localPosition[0];
    ef_pcOutput.pcLocalPosition[i * 2 + 1] = inputPC[i].localPosition[1];
    ef_pcOutput.pcLocalCovariance[i * 2] = inputPC[i].localCovariance[0];
    ef_pcOutput.pcLocalCovariance[i * 2 + 1] = inputPC[i].localCovariance[1];
    ef_pcOutput.pcIdHash[i] = inputPC[i].idHash;
    ef_pcOutput.pcId[i] = inputPC[i].id;
    ef_pcOutput.pcGlobalPosition[i * 3] = inputPC[i].globalPosition[0];
    ef_pcOutput.pcGlobalPosition[i * 3 + 1] = inputPC[i].globalPosition[1];
    ef_pcOutput.pcGlobalPosition[i * 3 + 2] = inputPC[i].globalPosition[2];

    inputpcRdoIndexSize += inputPC[i].sizeOfRDOList;

    for (int j = 0; j < inputPC[i].sizeOfRDOList; j++) {
      ef_pcOutput.pcRdoList[rdoIndex_counter + j] = inputPC[i].rdoList[j];
    }
    ef_pcOutput.pcChannelsInPhi[i] = inputPC[i].channelsInPhi;
    ef_pcOutput.pcChannelsInEta[i] = inputPC[i].channelsInEta;
    ef_pcOutput.pcWidthInEta[i] = inputPC[i].widthInEta;
    ef_pcOutput.pcOmegaX[i] = inputPC[i].omegaX;
    ef_pcOutput.pcOmegaY[i] = inputPC[i].omegaY;

    inputpcTotListsize += inputPC[i].sizeOfTotList;

    for (int j = 0; j < inputPC[i].sizeOfTotList; j++) {
      ef_pcOutput.pcTotList[totIndex_counter + j] = inputPC[i].totList[j];
    }
    ef_pcOutput.pcTotalToT[i] = inputPC[i].totalToT;

    inputpcChargeListsize += inputPC[i].sizeOfChargeList;

    for (int j = 0; j < inputPC[i].sizeOfChargeList; j++) {
      ef_pcOutput.pcChargeList[chargeIndex_counter + j] =
          inputPC[i].chargeList[j];
    }
    ef_pcOutput.pcTotalCharge[i] = inputPC[i].totalCharge;
    ef_pcOutput.pcEnergyLoss[i] = inputPC[i].energyLoss;
    ef_pcOutput.pcIsSplit[i] = inputPC[i].isSplit;
    ef_pcOutput.pcSplitProbability1[i] = inputPC[i].splitProbability1;
    ef_pcOutput.pcSplitProbability2[i] = inputPC[i].splitProbability2;
    ef_pcOutput.pcLvl1a[i] = inputPC[i].lvl1a;

    // update the index counter
    rdoIndex_counter += inputPC[i].sizeOfRDOList;
    totIndex_counter += inputPC[i].sizeOfTotList;
    chargeIndex_counter += inputPC[i].sizeOfChargeList;
    metadata[0].pcRdoIndex[i] = inputPC[i].sizeOfRDOList;
    metadata[0].pcTotIndex[i] = inputPC[i].sizeOfTotList;
    metadata[0].pcChargeIndex[i] = inputPC[i].sizeOfChargeList;
  }

  // transfer strip space points
  for (size_t i = 0; i < inputSSP.size(); i++) {
    ef_sspOutput.spIdHash[i * 2] = inputSSP[i].idHash[0];
    ef_sspOutput.spIdHash[i * 2 + 1] = inputSSP[i].idHash[1];
    ef_sspOutput.spGlobalPosition[i * 3] = inputSSP[i].globalPosition[0];
    ef_sspOutput.spGlobalPosition[i * 3 + 1] = inputSSP[i].globalPosition[1];
    ef_sspOutput.spGlobalPosition[i * 3 + 2] = inputSSP[i].globalPosition[2];
    ef_sspOutput.spRadius[i] = inputSSP[i].radius;
    ef_sspOutput.spVarianceR[i] = inputSSP[i].cov_r;
    ef_sspOutput.spVarianceZ[i] = inputSSP[i].cov_z;
    ef_sspOutput.spTopHalfStripLength[i] = inputSSP[i].topHalfStripLength;
    ef_sspOutput.spBottomHalfStripLength[i] = inputSSP[i].bottomHalfStripLength;
    ef_sspOutput.spTopStripDirection[i * 3] = inputSSP[i].topStripDirection[0];
    ef_sspOutput.spTopStripDirection[i * 3 + 1] =
        inputSSP[i].topStripDirection[1];
    ef_sspOutput.spTopStripDirection[i * 3 + 2] =
        inputSSP[i].topStripDirection[2];
    ef_sspOutput.spBottomStripDirection[i * 3] =
        inputSSP[i].bottomStripDirection[0];
    ef_sspOutput.spBottomStripDirection[i * 3 + 1] =
        inputSSP[i].bottomStripDirection[1];
    ef_sspOutput.spBottomStripDirection[i * 3 + 2] =
        inputSSP[i].bottomStripDirection[2];
    ef_sspOutput.spStripCenterDistance[i * 3] =
        inputSSP[i].stripCenterDistance[0];
    ef_sspOutput.spStripCenterDistance[i * 3 + 1] =
        inputSSP[i].stripCenterDistance[1];
    ef_sspOutput.spStripCenterDistance[i * 3 + 2] =
        inputSSP[i].stripCenterDistance[2];
    ef_sspOutput.spTopStripCenter[i * 3] = inputSSP[i].topStripCenter[0];
    ef_sspOutput.spTopStripCenter[i * 3 + 1] = inputSSP[i].topStripCenter[1];
    ef_sspOutput.spTopStripCenter[i * 3 + 2] = inputSSP[i].topStripCenter[2];
  }

  // transfer pixel space points
  for (size_t i = 0; i < inputPSP.size(); i++) {
    ef_pspOutput.spIdHash[i] = inputPSP[i].idHash[0];
    ef_pspOutput.spGlobalPosition[i * 3] = inputPSP[i].globalPosition[0];
    ef_pspOutput.spGlobalPosition[i * 3 + 1] = inputPSP[i].globalPosition[1];
    ef_pspOutput.spGlobalPosition[i * 3 + 2] = inputPSP[i].globalPosition[2];
    ef_pspOutput.spRadius[i] = inputPSP[i].radius;
    ef_pspOutput.spVarianceR[i] = inputPSP[i].cov_r;
    ef_pspOutput.spVarianceZ[i] = inputPSP[i].cov_z;
  }

  // transfer metadata
  metadata[0].numOfStripClusters = inputSC.size();
  metadata[0].numOfPixelClusters = inputPC.size();
  metadata[0].numOfStripSpacePoints = inputSSP.size();
  metadata[0].numOfPixelSpacePoints = inputPSP.size();
  metadata[0].scRdoIndexSize = inputscRdoIndexSize;
  metadata[0].pcRdoIndexSize = inputpcRdoIndexSize;
  metadata[0].pcTotIndexSize = inputpcTotListsize;
  metadata[0].pcChargeIndexSize = inputpcChargeListsize;

  return StatusCode::SUCCESS;
}
