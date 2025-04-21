/*
    Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file include/xAODSpacePointMaker.h
 * @author zhaoyuan.cui@cern.ch
 * @author yuan-tang.chou@cern.ch
 * @author levi.samuel.evans@cern.ch
 * @date Mar. 11, 2025
 */

#ifndef EFTRACKINGFPGAINTEGRATION_XAODSPACEPOINTMAKER_H
#define EFTRACKINGFPGAINTEGRATION_XAODSPACEPOINTMAKER_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/EventContext.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"
#include "xAODInDetMeasurement/SpacePointContainer.h"

#include "EFTrackingFPGAUtility/IEFTrackingFPGAIntegrationTool.h"
#include "EFTrackingFPGAUtility/EFTrackingTransient.h"

/**
 * @class xAODSpacePointMaker
 * @brief Creates xAOD space point containers from FPGA input and existing clusters
 */
class xAODSpacePointMaker : public extends<AthAlgTool, IEFTrackingFPGAIntegrationTool> {
public:
  using extends::extends;

  /**
   * @brief Initialise the space point maker tool
   */
  StatusCode initialize() override;

  /**
   * @brief Make the pixel space point container
   * @param spAux Input space point data
   * @param metadata Input metadata
   * @param ctx
   * @return StatusCode
   */
  StatusCode makePixelSpacePointContainer(
      const EFTrackingTransient::SpacePointAuxInput &spAux,
      const EFTrackingTransient::Metadata *metadata,
      const EventContext &ctx) const;

  /**
   * @brief Make the strip space point container
   * @param sspAux Input space point data
   * @param metadata Input metadata
   * @param ctx
   * @return StatusCode
   */
  StatusCode makeStripSpacePointContainer(
      const EFTrackingTransient::SpacePointAuxInput &sspAux,
      const EFTrackingTransient::Metadata *metadata,
      const EventContext &ctx) const;

private:
  /// Key for the pixel cluster container to read from
  SG::ReadHandleKey<xAOD::PixelClusterContainer> m_pixelClusterKey{
      this, "PixelClusterContainerKey", "FPGAPixelClusters",
      "Key for input pixel cluster container"};

  /// Key for the strip cluster container to read from
  SG::ReadHandleKey<xAOD::StripClusterContainer> m_stripClusterKey{
      this, "StripClusterContainerKey", "FPGAStripClusters",
      "Key for input strip cluster container"};

  /// Key for the pixel space points container to be created
  SG::WriteHandleKey<xAOD::SpacePointContainer> m_pixelSpacePointsKey{
      this, "PixelSpacePointContainerKey", "FPGAPixelSpacePoints",
      "Key for output pixel space point container"};

  /// Key for the strip space points container to be created
  SG::WriteHandleKey<xAOD::SpacePointContainer> m_stripSpacePointsKey{
      this, "StripSpacePointContainerKey", "FPGAStripSpacePoints",
      "Key for output strip space point container"};
};

#endif 
