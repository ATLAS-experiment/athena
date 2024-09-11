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

#include "AthContainers/debug.h"
#include "xAODInDetMeasurement/StripClusterAuxContainer.h"

#define MAX_CLUSTER_NUM \
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
  ATH_CHECK(m_clusterContainerMaker.retrieve());

  return StatusCode::SUCCESS;
}

StatusCode DataPreparationPipeline::execute(const EventContext &ctx) const {

  // Retrieve the strip and pixel cluster container from the event store
  SG::ReadHandle<xAOD::StripClusterContainer> inputStripClusters(
      m_stripClustersKey, ctx);
  SG::ReadHandle<xAOD::PixelClusterContainer> inputPixelClusters(
      m_pixelClustersKey, ctx);

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

  if (msgLvl(MSG::DEBUG)) {

    ATH_MSG_DEBUG("There are " << inputStripClusters->size()
                               << " strip cluster in this event");
    ATH_MSG_DEBUG("There are " << inputPixelClusters->size()
                               << " pixel cluster in this event");

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
  }

  // Prepare the input data for the kernel
  // This is to "remake" the cluster but in a kernel compatible format using
  // the struct defined in EFTrackingDataFormats.h
  std::vector<EFTrackingDataFormats::StripCluster>
      ef_stripClusters;  // Strip clusters as kernel input argument
  std::vector<EFTrackingDataFormats::PixelCluster>
      ef_pixelClusters;  // Pixel clusters as kernel input argument

  // Currently we only take the first 10 clusters for testing, this will be
  // removed in the future
  ATH_CHECK(
      getInputClusterData(inputStripClusters.ptr(), ef_stripClusters, 10));
  ATH_CHECK(
      getInputClusterData(inputPixelClusters.ptr(), ef_pixelClusters, 10));

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
    ATH_CHECK(runSW(ef_stripClusters, ef_pixelClusters, ctx));
  } else {
    ATH_CHECK(runHW(ef_stripClusters, ef_pixelClusters));
  }

  return StatusCode::SUCCESS;
}

StatusCode DataPreparationPipeline::getInputClusterData(
    const xAOD::StripClusterContainer *sc,
    std::vector<EFTrackingDataFormats::StripCluster> &ef_sc,
    long unsigned int N) const{
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

StatusCode DataPreparationPipeline::runSW(
    const std::vector<EFTrackingDataFormats::StripCluster> &ef_sc,
    const std::vector<EFTrackingDataFormats::PixelCluster> &ef_pc,
    const EventContext &ctx) const {
  ATH_MSG_DEBUG("Running the software version of the kernel");

  std::unique_ptr<EFTrackingDataFormats::Metadata> metadata = std::make_unique<EFTrackingDataFormats::Metadata>();
  std::vector<float> scLocalPosition(MAX_CLUSTER_NUM);
  std::vector<float> scLocalCovariance(MAX_CLUSTER_NUM);
  std::vector<unsigned int> scIdHash(MAX_CLUSTER_NUM);
  std::vector<long unsigned int> scId(MAX_CLUSTER_NUM);
  std::vector<float> scGlobalPosition(MAX_CLUSTER_NUM * 3);
  std::vector<unsigned long long> scRdoList(MAX_CLUSTER_NUM * 1000);
  std::vector<int> scChannelsInPhi(MAX_CLUSTER_NUM);

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

  ATH_CHECK(transferSW(
      ef_sc.data(), ef_sc.size(), scLocalPosition.data(),
      scLocalCovariance.data(), scIdHash.data(), scId.data(),
      scGlobalPosition.data(), scRdoList.data(), scChannelsInPhi.data(),
      ef_pc.data(), ef_pc.size(), pcLocalPosition.data(),
      pcLocalCovariance.data(), pcIdHash.data(), pcId.data(),
      pcGlobalPosition.data(), pcRdoList.data(), pcChannelsInPhi.data(),
      pcChannelsInEta.data(), pcWidthInEta.data(), pcOmegaX.data(),
      pcOmegaY.data(), pcTotList.data(), pcTotalToT.data(), pcChargeList.data(),
      pcTotalCharge.data(), pcEnergyLoss.data(), pcIsSplit.data(),
      pcSplitProbability1.data(), pcSplitProbability2.data(), pcLvl1a.data(),
      metadata.get()));

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

  // print all strip clusters
  for (unsigned i = 0; i < metadata->numOfStripClusters; i++) {
    ATH_MSG_DEBUG("scLocalPosition[" << i << "] = " << scLocalPosition[i]);
    ATH_MSG_DEBUG("scLocalCovariance[" << i << "] = " << scLocalCovariance[i]);
    ATH_MSG_DEBUG("scIdHash[" << i << "] = " << scIdHash[i]);
    ATH_MSG_DEBUG("scId[" << i << "] = " << scId[i]);
    ATH_MSG_DEBUG("scGlobalPosition[" << i << "] = " << scGlobalPosition[i * 3]
                                      << ", " << scGlobalPosition[i * 3 + 1]
                                      << ", " << scGlobalPosition[i * 3 + 2]);
    ATH_MSG_DEBUG("scRdoList[" << i << "] = " << scRdoList[i]);
    ATH_MSG_DEBUG("scChannelsInPhi[" << i << "] = " << scChannelsInPhi[i]);
  }

  // print all pixel clusters
  for (unsigned i = 0; i < metadata->numOfPixelClusters; i++) {

    ATH_MSG_DEBUG("pcLocalPosition[" << i << "] = " << pcLocalPosition[i * 2]
                                     << ", " << pcLocalPosition[i * 2 + 1]);
    ATH_MSG_DEBUG("pcLocalCovariance["
                  << i << "] = " << pcLocalCovariance[i * 2] << ", "
                  << pcLocalCovariance[i * 2 + 1]);
    ATH_MSG_DEBUG("pcIdHash[" << i << "] = " << pcIdHash[i]);
    ATH_MSG_DEBUG("pcId[" << i << "] = " << pcId[i]);
    ATH_MSG_DEBUG("pcGlobalPosition[" << i << "] = " << pcGlobalPosition[i * 3]
                                      << ", " << pcGlobalPosition[i * 3 + 1]
                                      << ", " << pcGlobalPosition[i * 3 + 2]);
    // ATH_MSG_DEBUG("pcRdoList[" << i << "] = " << pcRdoList[i]);
    ATH_MSG_DEBUG("pcChannelsInPhi[" << i << "] = " << pcChannelsInPhi[i]);
    ATH_MSG_DEBUG("pcChannelsInEta[" << i << "] = " << pcChannelsInEta[i]);
    ATH_MSG_DEBUG("pcWidthInEta[" << i << "] = " << pcWidthInEta[i]);
    ATH_MSG_DEBUG("pcOmegaX[" << i << "] = " << pcOmegaX[i]);
    ATH_MSG_DEBUG("pcOmegaY[" << i << "] = " << pcOmegaY[i]);
    // ATH_MSG_DEBUG("pcTotList[" << i << "] = " << pcTotList[i]);
    ATH_MSG_DEBUG("pcTotalToT[" << i << "] = " << pcTotalToT[i]);
    // ATH_MSG_DEBUG("pcChargeList[" << i << "] = " << pcChargeList[i]);
    ATH_MSG_DEBUG("pcTotalCharge[" << i << "] = " << pcTotalCharge[i]);
    ATH_MSG_DEBUG("pcEnergyLoss[" << i << "] = " << pcEnergyLoss[i]);
    ATH_MSG_DEBUG("pcIsSplit[" << i << "] = " << pcIsSplit[i]);
    ATH_MSG_DEBUG("pcSplitProbability1[" << i
                                         << "] = " << pcSplitProbability1[i]);
    ATH_MSG_DEBUG("pcSplitProbability2[" << i
                                         << "] = " << pcSplitProbability2[i]);
    ATH_MSG_DEBUG("pcLvl1a[" << i << "] = " << pcLvl1a[i]);
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

  ATH_CHECK(m_clusterContainerMaker->makeStripClusterContainer(
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

  ATH_CHECK(m_clusterContainerMaker->makePixelClusterContainer(
      ef_pc.size(), pxAux, metadata.get(), ctx));

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
    const std::vector<EFTrackingDataFormats::PixelCluster> &ef_pc) const {
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
    const EFTrackingDataFormats::StripCluster *inputSC, int inputSCSize,
    float *scLocalPosition, float *scLocalCovariance, unsigned int *scIdHash,
    long unsigned int *scId, float *scGlobalPosition,
    unsigned long long *scRdoList, int *scChannelsInPhi,
    // PixelCluster
    const EFTrackingDataFormats::PixelCluster *inputPC, int inputPCSize,
    float *pcLocalPosition, float *pcLocalCovariance, unsigned int *pcIdHash,
    long unsigned int *pcId, float *pcGlobalPosition,
    unsigned long long *pcRdoList, int *pcChannelsInPhi, int *pcChannelsInEta,
    float *pcWidthInEta, float *pcOmegaX, float *pcOmegaY, int *pcTotList,
    int *pcTotalToT, float *pcChargeList, float *pcTotalCharge,
    float *pcEnergyLoss, char *pcIsSplit, float *pcSplitProbability1,
    float *pcSplitProbability2, int *pcLvl1a,
    EFTrackingDataFormats::Metadata *metadata) const {
  // return input
  int rdoIndex_counter = 0;

  unsigned int inputscRdoIndexSize = 0;
  unsigned int inputpcRdoIndexSize = 0;

  // transfer inputSC
  // trasnsfer_sc:
  for (int i = 0; i < inputSCSize; i++) {
    scLocalPosition[i] = inputSC[i].localPosition;
    scLocalCovariance[i] = inputSC[i].localCovariance;
    scIdHash[i] = inputSC[i].idHash;
    scId[i] = inputSC[i].id;
    scGlobalPosition[i * 3] = inputSC[i].globalPosition[0];
    scGlobalPosition[i * 3 + 1] = inputSC[i].globalPosition[1];
    scGlobalPosition[i * 3 + 2] = inputSC[i].globalPosition[2];

    inputscRdoIndexSize += inputSC[i].sizeOfRDOList;

    for (int j = 0; j < inputSC[i].sizeOfRDOList; j++) {
      scRdoList[rdoIndex_counter + j] = inputSC[i].rdoList[j];
    }
    // update the index counter
    scChannelsInPhi[i] = inputSC[i].channelsInPhi;
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
  for (int i = 0; i < inputPCSize; i++) {
    pcLocalPosition[i * 2] = inputPC[i].localPosition[0];
    pcLocalPosition[i * 2 + 1] = inputPC[i].localPosition[1];
    pcLocalCovariance[i * 2] = inputPC[i].localCovariance[0];
    pcLocalCovariance[i * 2 + 1] = inputPC[i].localCovariance[1];
    pcIdHash[i] = inputPC[i].idHash;
    pcId[i] = inputPC[i].id;
    pcGlobalPosition[i * 3] = inputPC[i].globalPosition[0];
    pcGlobalPosition[i * 3 + 1] = inputPC[i].globalPosition[1];
    pcGlobalPosition[i * 3 + 2] = inputPC[i].globalPosition[2];

    inputpcRdoIndexSize += inputPC[i].sizeOfRDOList;

    for (int j = 0; j < inputPC[i].sizeOfRDOList; j++) {
      pcRdoList[rdoIndex_counter + j] = inputPC[i].rdoList[j];
    }
    pcChannelsInPhi[i] = inputPC[i].channelsInPhi;
    pcChannelsInEta[i] = inputPC[i].channelsInEta;
    pcWidthInEta[i] = inputPC[i].widthInEta;
    pcOmegaX[i] = inputPC[i].omegaX;
    pcOmegaY[i] = inputPC[i].omegaY;

    inputpcTotListsize += inputPC[i].sizeOfTotList;

    for (int j = 0; j < inputPC[i].sizeOfTotList; j++) {
      pcTotList[totIndex_counter + j] = inputPC[i].totList[j];
    }
    pcTotalToT[i] = inputPC[i].totalToT;

    inputpcChargeListsize += inputPC[i].sizeOfChargeList;

    for (int j = 0; j < inputPC[i].sizeOfChargeList; j++) {
      pcChargeList[chargeIndex_counter + j] = inputPC[i].chargeList[j];
    }
    pcTotalCharge[i] = inputPC[i].totalCharge;
    pcEnergyLoss[i] = inputPC[i].energyLoss;
    pcIsSplit[i] = inputPC[i].isSplit;
    pcSplitProbability1[i] = inputPC[i].splitProbability1;
    pcSplitProbability2[i] = inputPC[i].splitProbability2;
    pcLvl1a[i] = inputPC[i].lvl1a;

    // update the index counter
    rdoIndex_counter += inputPC[i].sizeOfRDOList;
    totIndex_counter += inputPC[i].sizeOfTotList;
    chargeIndex_counter += inputPC[i].sizeOfChargeList;
    metadata[0].pcRdoIndex[i] = inputPC[i].sizeOfRDOList;
    metadata[0].pcTotIndex[i] = inputPC[i].sizeOfTotList;
    metadata[0].pcChargeIndex[i] = inputPC[i].sizeOfChargeList;
  }

  // transfer metadata
  metadata[0].numOfStripClusters = inputSCSize;
  metadata[0].numOfPixelClusters = inputPCSize;
  metadata[0].scRdoIndexSize = inputscRdoIndexSize;
  metadata[0].pcRdoIndexSize = inputpcRdoIndexSize;
  metadata[0].pcTotIndexSize = inputpcTotListsize;
  metadata[0].pcChargeIndexSize = inputpcChargeListsize;

  return StatusCode::SUCCESS;
}
