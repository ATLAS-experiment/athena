/*
    Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file include/xAODClusterMaker.h
 * @author zhaoyuan.cui@cern.ch
 * @author yuan-tang.chou@cern.ch
 * @author levi.samuel.evans@cern.ch
 * @date Mar. 11, 2025
 */

#ifndef EFTRACKINGFPGAINTEGRATION_XAODCLUSTERMAKER_H
#define EFTRACKINGFPGAINTEGRATION_XAODCLUSTERMAKER_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/EventContext.h"
#include "StoreGate/WriteHandleKey.h"

#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"

#include "EFTrackingFPGAUtility/IEFTrackingFPGAIntegrationTool.h"
#include "EFTrackingFPGAUtility/EFTrackingTransient.h"

#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/IChronoStatSvc.h"

/**
 * @class xAODClusterMaker
 * @brief Creates xAOD pixel and strip cluster containers from FPGA input
 */
class xAODClusterMaker : public extends<AthAlgTool, IEFTrackingFPGAIntegrationTool> {
public:
  using extends::extends;

  /**
   * @brief Initialise the tool
   */
  StatusCode initialize() override;

  /**
   * @brief Make the strip cluster container
   * @param scAux Input strip cluster data
   * @param metadata Input metadata
   * @param ctx 
   * @return StatusCode
   */
  StatusCode makeStripClusterContainer(
      const EFTrackingTransient::StripClusterAuxInput &scAux,
      const EFTrackingTransient::Metadata *metadata,
      const EventContext &ctx) const;

  /**
   * @brief Make the pixel cluster container
   * @param pxAux Input pixel cluster data
   * @param metadata Input metadata
   * @param ctx
   * @return StatusCode
   */
  StatusCode makePixelClusterContainer(
      const EFTrackingTransient::PixelClusterAuxInput &pxAux,
      const EFTrackingTransient::Metadata *metadata,
      const EventContext &ctx) const;

private:
  /// Key for the pixel clusters container to be created
  SG::WriteHandleKey<xAOD::PixelClusterContainer> m_pixelClustersKey{
      this, "PixelClusterContainerKey", "FPGAPixelClusters",
      "Key for output pixel cluster container"};

  /// Key for the strip clusters container to be created
  SG::WriteHandleKey<xAOD::StripClusterContainer> m_stripClustersKey{
      this, "StripClusterContainerKey", "FPGAStripClusters",
      "Key for output strip cluster container"};

  Gaudi::Property<bool> m_doBulkCopy{this, "DoBulkCopy", true, "Do bulk copy"}; //!< Do bulk copy method

  ServiceHandle<IChronoStatSvc> m_chronoSvc{this, "ChronoStatSvc", "ChronoStatSvc"};
};

#endif 
