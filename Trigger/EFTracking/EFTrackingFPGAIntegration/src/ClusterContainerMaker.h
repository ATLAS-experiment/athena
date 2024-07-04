/*
    Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file src/ClusterContainerMaker.h
 * @author zhaoyuan.cui@cern.ch
 * @author yuan-tang.chou@cern.ch
 * @date Apr. 15, 2024
 * @brief Tool for making xAOD cluster container
 */

#ifndef EFTRACKING_FPGA_INTEGRATION__CLUSTER_CONTAINER_MAKER_H
#define EFTRACKING_FPGA_INTEGRATION__CLUSTER_CONTAINER_MAKER_H

#include "EFTrackingDataFormats.h"
#include "EFTrackingFPGAIntegration/IEFTrackingFPGAIntegrationTool.h"

#include "AthenaBaseComps/AthAlgTool.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"
#include "StoreGate/WriteHandleKey.h"

class ClusterContainerMaker
    : public extends<AthAlgTool, IEFTrackingFPGAIntegrationTool> {
 public:
  using extends::extends;

  StatusCode initialize() override;

  /**
   * @brief Create xAOD::StripClusterContainer by creating xAOD::StripCluster
   * objects one by one
   */
  StatusCode makeStripClusterContainer(
      const int numClusters,
      const EFTrackingDataFormats::StripClusterAuxInput &scAux,
      const EFTrackingDataFormats::Metadata *meta,
      const EventContext &ctx) const;

  /**
   * @brief Create xAOD::PixelClusterContainer by creating xAOD::PixelCluster
   * objects one by one
   */
  StatusCode makePixelClusterContainer(
      const int numClusters,
      const EFTrackingDataFormats::PixelClusterAuxInput &pxAux,
      const EFTrackingDataFormats::Metadata *meta,
      const EventContext &ctx) const;

 private:
  SG::WriteHandleKey<xAOD::PixelClusterContainer> m_pixelClustersKey{
      this, "OutputPixelName", "fpgaPixelClusters",
      "Output container name"};  //!< Key for the pixel cluster container
  SG::WriteHandleKey<xAOD::StripClusterContainer> m_stripClustersKey{
      this, "OutputStripName", "fpgaStripClusters",
      "Output container name"};  //!< Key for the strip cluster container
};

#endif  // FTRACKING_FPGA_INTEGRATION__CLUSTER_CONTAINER_MAKER_H