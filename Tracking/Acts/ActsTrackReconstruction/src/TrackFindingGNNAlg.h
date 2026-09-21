/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_TRACKFINDINGGNNALG_H
#define ACTSTRACKRECONSTRUCTION_TRACKFINDINGGNNALG_H

// Ensure that the ATLAS eigen plugin is loaded first
#include "GeoPrimitives/GeoPrimitives.h"

// Base Class
#include "AthenaBaseComps/AthReentrantAlgorithm.h"


// Tools
#include "ActsToolInterfaces/IGnnPipelineTool.h"
#include "ActsGeometryInterfaces/ITrackingGeometrySvc.h"
#include "ActsToolInterfaces/ITrackParamsEstimationTool.h"
#include "ActsCalibrators/xAODUncalibMeasSurfAcc.h"

// ACTS
#include "Acts/TrackFinding/TrackSelector.hpp"

// ActsTrk
#include "ActsEvent/SeedContainer.h"
#include "ActsEvent/TrackContainer.h"
#include "ActsEvent/ContextUtility.h"
#include "ActsInterop/Logger.h"
#include "ActsToolInterfaces/IFitterTool.h"

// Athena
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/IChronoStatSvc.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"

// STL
#include <memory>
#include <string>

// Handle Keys
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "src/detail/Definitions.h"
#include "src/detail/OnTrackCalibrator.h"

namespace ActsTrk {
class TrackFindingGNNAlg : public AthReentrantAlgorithm {
public:
  TrackFindingGNNAlg(const std::string &name, ISvcLocator *pSvcLocator);
  virtual ~TrackFindingGNNAlg();

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext &ctx) const override;

private:
  // Tool Handles
  ToolHandle<GenericMonitoringTool> m_monTool{this, "MonTool", "",
                                              "Monitoring tool"};
  ServiceHandle<ActsTrk::ITrackingGeometrySvc> m_trackingGeometrySvc{this, "TrackingGeometrySvc", "ActsTrackingGeometrySvc"};
  ToolHandle<ITrackParamsEstimationTool> m_paramEstimationTool{
      this, "TrackParamsEstimationTool", "", "Track Param Estimation from Seeds"};
  ToolHandle<IFitterTool> m_fitterTool{
      this, "FitterTool", "", "Track fitting tool"};
  ToolHandle<ActsTrk::IGnnPipelineTool> m_gnnPipelineTool{
      this, "GnnPipelineTool", "", "GNN seeding pipeline"};

  /** @brief Utility to fetch the geometry, magnetic field and calibration context in the event */
  ContextUtility m_ctxProvider{this};
  ServiceHandle<IChronoStatSvc> m_chronoSvc{"ChronoStatSvc", name()};

  detail::xAODUncalibMeasSurfAcc m_uncalibMeasSurfAccessor{};
  detail::OnTrackCalibrator<MutableTrackStateBackend>
      m_uncalibMeasCalibrator{};

  // Input: Spacepoint containers
  SG::ReadHandleKey<xAOD::SpacePointContainer>
      m_xaodPixelSpacePointContainerKey{this, "xAODInputPixelSpacePoints",
                                        "ITkPixelSpacePoints"};
  SG::ReadHandleKey<xAOD::SpacePointContainer>
      m_xaodStripSpacePointContainerKey{
          this, "xAODInputSpacePointsContainerKey", "ITkStripSpacePoints"};
  SG::ReadHandleKey<xAOD::SpacePointContainer>
      m_xaodStripSpacePointOverlapContainerKey{
          this, "xAODInputSpacePointsOverlapContainerKey",
          "ITkStripOverlapSpacePoints"};

  SG::WriteHandleKey<TrackContainer> m_trackContainerKey{
      this, "ACTSTracksLocation", "",
      "Output track collection (ActsTrk variant)"};

  // Configuration
  Gaudi::Property<unsigned int> m_maxPropagationStep{
      this, "maxPropagationStep", 1000,
      "Maximum number of steps for one propagate call"};

  Gaudi::Property<double> m_varianceInflation{
      this, "varianceInflation", 1.0,
      "Factor that is multiplied to all initial variances"};

  Gaudi::Property<bool> m_tightSeeds{
      this, "tightSeeds", false,
      "Use tight seeds instead of spread seeds for param estimation"};

  Gaudi::Property<bool> m_relaxCentralHoleSel{
      this, "relaxCentralHoleSel", false, "Relax holes from 2 to 4 in central region"};

  Gaudi::Property<bool> m_relaxMeasurementSel{
      this, "relaxMeasurementSel", true, "Apply 7,7,7 measurement sel"};

  Gaudi::Property<bool> m_offlineZ0Sel{
      this, "offlineZ0Sel", false, "Apply offline z0 selection"};

  Acts::TrackSelector::EtaBinnedConfig m_trackSelectorConfig;

  /// Private access to the logger
  const Acts::Logger &logger() const { return *m_logger; }

  /// logging instance
  std::unique_ptr<const Acts::Logger> m_logger;
};

} // namespace ActsTrk

#endif
