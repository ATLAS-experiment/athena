/*
    Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file src/DataPreparationPipeline.h
 * @author zhaoyuan.cui@cern.ch
 * @author yuan-tang.chou@cern.ch
 * @date Feb. 18, 2024
 * @brief Class for the data preparation pipeline
 */

#ifndef EFTRACKING_FPGA_INTEGRATION_DATAPREPARATIONPIPELINE_H
#define EFTRACKING_FPGA_INTEGRATION_DATAPREPARATIONPIPELINE_H

// EFTracking include
#include "ClusterContainerMaker.h"
#include "EFTrackingDataFormats.h"
#include "IntegrationBase.h"

// Athena include
#include "StoreGate/ReadHandleKey.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"

/**
 * @brief This is the class for the data preparation pipeline.
 *
 * The output of this pipeline are the xAOD::StripCluster(Container) and
 * xAOD::PixelCluster(Container). These containers are to be used in other
 * CPU-based algorithms.
 */
class DataPreparationPipeline : public IntegrationBase {
 public:
  using IntegrationBase::IntegrationBase;
  StatusCode initialize() override final;
  StatusCode execute() override final;

  /**
   * @brief Convert the strip cluster from xAOD container to simple std::vector
   * of EFTrackingDataFormats::StripCluster.
   *
   * This is needed for the kernel input.
   */
  StatusCode getInputClusterData(
      const xAOD::StripClusterContainer *sc,
      std::vector<EFTrackingDataFormats::StripCluster> &ef_sc,
      long unsigned int N);

  /**
   * @brief Convert the pixel cluster from xAOD container to simple std::vector
   * of EFTrackingDataFormats::PixelCluster.
   *
   * This is needed for the kernel input.
   */
  StatusCode getInputClusterData(
      const xAOD::PixelClusterContainer *pc,
      std::vector<EFTrackingDataFormats::PixelCluster> &ef_pc,
      long unsigned int N);

  /**
   * @brief Run the software version of the transfer kernel. This doesn't
   * require the FPGA accelerator in the machine.
   */
  StatusCode runSW(
      const std::vector<EFTrackingDataFormats::StripCluster> &ef_sc,
      const std::vector<EFTrackingDataFormats::PixelCluster> &ef_pc,
      const EventContext &ctx);

  /**
   * @brief Run the hardware version of the transfer kernel. This requires the
   * FPGA accelerator in the machine and will perform all necessary OpenCL
   * setups.
   */
  StatusCode runHW(
      const std::vector<EFTrackingDataFormats::StripCluster> &ef_sc,
      const std::vector<EFTrackingDataFormats::PixelCluster> &ef_pc);

  /**
   * @brief Software version of the transfer kernel. The purse of this function
   * is to mimic the FPGA output at software level.
   *
   * It takes the EFTrackingDataFormats::StripCluster and
   * EFTrackingDataFormats::PixelCluster as input arguments and mimic the
   * transfer kernel by giving array output.
   */
  StatusCode transferSW(
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
      EFTrackingDataFormats::Metadata *metadata);  // mimic the tranfser kernel

 private:
  // At this stage of development we need xAOD::Strip/PixelClusterContainer as
  // our input
  SG::ReadHandleKey<xAOD::StripClusterContainer> m_stripClustersKey{
      this, "StripClusterContainerKey", "FPGAITkStripClusters",
      "Key for Strip Cluster Containers"};
  SG::ReadHandleKey<xAOD::PixelClusterContainer> m_pixelClustersKey{
      this, "PixelClusterContainerKey", "FPGAITkPixelClusters",
      "Key for Pixel Cluster Containers"};
  Gaudi::Property<std::string> m_xclbin{
      this, "xclbin", "",
      "xclbin path and name"};  //!< Path and name of the xclbin file
  Gaudi::Property<std::string> m_kernelName{this, "KernelName", "",
                                            "Kernel name"};  //!< Kernel name
  Gaudi::Property<bool> m_runSW{
      this, "RunSW", true,
      "Run software mode"};  //!< Software mode, not running on the FPGA

  // Conver kernel outputs into xAOD containers
  ToolHandle<ClusterContainerMaker> m_clusterContainerMaker{
      this, "ClusterMaker", "ClusterContainerMaker",
      "tool to make cluster"};  //!< Tool handle for ClusterContainerMaker

  std::unique_ptr<EFTrackingDataFormats::Metadata> m_metadata;
};

#endif  // EFTRACKING_FPGA_INTEGRATION_DATAPREPARATIONPIPELINE_H
