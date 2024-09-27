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
#include "EFTrackingDataFormats.h"
#include "IntegrationBase.h"
#include "xAODContainerMaker.h"

// Athena include
#include "StoreGate/ReadHandleKey.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/SpacePointContainer.h"
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
  StatusCode execute(const EventContext &ctx) const override final;

  /**
   * @brief Convert the strip cluster from xAOD container to simple std::vector
   * of EFTrackingDataFormats::StripCluster.
   *
   * This is needed for the kernel input.
   */
  StatusCode getInputClusterData(
      const xAOD::StripClusterContainer *sc,
      std::vector<EFTrackingDataFormats::StripCluster> &ef_sc,
      long unsigned int N) const;

  /**
   * @brief Convert the pixel cluster from xAOD container to simple std::vector
   * of EFTrackingDataFormats::PixelCluster.
   *
   * This is needed for the kernel input.
   */
  StatusCode getInputClusterData(
      const xAOD::PixelClusterContainer *pc,
      std::vector<EFTrackingDataFormats::PixelCluster> &ef_pc,
      long unsigned int N) const;

  /**
   * @brief Convert the space point from xAOD container to simple std::vector
   * of EFTrackingDataFormats::SpacePoint.
   *
   * This is needed for the kernel input.
   */
  StatusCode getInputSpacePointData(
      const xAOD::SpacePointContainer *sp,
      std::vector<EFTrackingDataFormats::SpacePoint> &ef_sp,
      std::vector<std::vector<const xAOD::UncalibratedMeasurement *>> &sp_meas,
      long unsigned int N, bool isStrip) const;

  /**
   * @brief Run the software version of the transfer kernel. This doesn't
   * require the FPGA accelerator in the machine.
   */
  StatusCode runSW(
      const std::vector<EFTrackingDataFormats::StripCluster> &ef_sc,
      const std::vector<EFTrackingDataFormats::PixelCluster> &ef_pc,
      const std::vector<EFTrackingDataFormats::SpacePoint> &ef_ssp,
      const std::vector<std::vector<const xAOD::UncalibratedMeasurement *>>
          &ssp_mes,
      const std::vector<EFTrackingDataFormats::SpacePoint> &ef_psp,
      const std::vector<std::vector<const xAOD::UncalibratedMeasurement *>>
          &psp_mes,
      const EventContext &ctx) const;

  /**
   * @brief Run the hardware version of the transfer kernel. This requires the
   * FPGA accelerator in the machine and will perform all necessary OpenCL
   * setups.
   */
  StatusCode runHW(
      const std::vector<EFTrackingDataFormats::StripCluster> &ef_sc,
      const std::vector<EFTrackingDataFormats::PixelCluster> &ef_pc,
      const std::vector<EFTrackingDataFormats::SpacePoint> &ef_ssp,
      const std::vector<std::vector<const xAOD::UncalibratedMeasurement *>>
          &ssp_mes,
      const std::vector<EFTrackingDataFormats::SpacePoint> &ef_psp,
      const std::vector<std::vector<const xAOD::UncalibratedMeasurement *>>
          &psp_mes) const;

  /**
   * @brief Software version of the transfer kernel. The purse of this function
   * is to mimic the FPGA output at software level.
   *
   * It takes the EFTrackingDataFormats::StripCluster
   * EFTrackingDataFormats::PixelCluster, and EFTrackingDataFormat::SpacePoints
   * as input arguments and mimic the transfer kernel by giving array output.
   */
  StatusCode transferSW(
      const std::vector<EFTrackingDataFormats::StripCluster> &inputSC,
      EFTrackingDataFormats::StripClusterOutput &outputSC,
      // PixelCluster
      const std::vector<EFTrackingDataFormats::PixelCluster> &inputPC,
      EFTrackingDataFormats::PixelClusterOutput &outputPC,
      // Strip SpacePoint
      const std::vector<EFTrackingDataFormats::SpacePoint> &inputSSP,
      EFTrackingDataFormats::SpacePointOutput &outputSSP,
      // Pixel SpacePoint
      const std::vector<EFTrackingDataFormats::SpacePoint> &inputPSP,
      EFTrackingDataFormats::SpacePointOutput &outputPSP,
      // Metadata
      EFTrackingDataFormats::Metadata *metadata)
      const;  // mimic the tranfser kernel

 private:
  // At this stage of development we need
  // xAOD::Strip/PixelCluster/SpacePointContainer as our input
  SG::ReadHandleKey<xAOD::StripClusterContainer> m_stripClustersKey{
      this, "StripClusterContainerKey", "FPGAITkStripClusters",
      "Key for Strip Cluster Containers"};
  SG::ReadHandleKey<xAOD::PixelClusterContainer> m_pixelClustersKey{
      this, "PixelClusterContainerKey", "FPGAITkPixelClusters",
      "Key for Pixel Cluster Containers"};
  SG::ReadHandleKey<xAOD::SpacePointContainer> m_stripSpacePointsKey{
      this, "StripSpacePointContainerKey", "FPGAITkStripSpacePoints",
      "Key for Strip SpacePoint Containers"};
  SG::ReadHandleKey<xAOD::SpacePointContainer> m_pixelSpacePointsKey{
      this, "PixelSpacePointContainerKey", "FPGAITkPixelSpacePoints",
      "Key for Pixel SpacePoint Containers"};
  Gaudi::Property<std::string> m_xclbin{
      this, "xclbin", "",
      "xclbin path and name"};  //!< Path and name of the xclbin file
  Gaudi::Property<std::string> m_kernelName{this, "KernelName", "",
                                            "Kernel name"};  //!< Kernel name
  Gaudi::Property<bool> m_runSW{
      this, "RunSW", true,
      "Run software mode"};  //!< Software mode, not running on the FPGA

  // Conver kernel outputs into xAOD containers
  ToolHandle<xAODContainerMaker> m_xAODContainerMaker{
      this, "xAODMaker", "xAODContainerMaker",
      "tool to make cluster"};  //!< Tool handle for xAODContainerMaker
};

#endif  // EFTRACKING_FPGA_INTEGRATION_DATAPREPARATIONPIPELINE_H
