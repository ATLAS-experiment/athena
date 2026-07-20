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
#include "ActsGeometryInterfaces/IExtrapolationTool.h"
#include "ActsGeometryInterfaces/ITrackingGeometryTool.h"
#include "ActsToolInterfaces/ITrackParamsEstimationTool.h"
#include "src/TrackStatePrinterTool.h"
#include "ActsCalibrators/xAODUncalibMeasSurfAcc.h"

// ACTS
#include "Acts/EventData/ProxyAccessor.hpp"
#include "Acts/EventData/TrackContainer.hpp"
#include "Acts/EventData/VectorTrackContainer.hpp"

// ActsTrk
#include "ActsEvent/SeedContainer.h"
#include "ActsEvent/TrackContainer.h"
#include "ActsEvent/TrackParameters.h"
#include "ActsEvent/TrackParametersContainer.h"
#include "ActsEvent/ContextUtility.h"
#include "ActsGeometry/DetectorElementToActsGeometryIdMap.h"
#include "ActsToolInterfaces/IFitterTool.h"
#include "ActsToolInterfaces/IOnTrackCalibratorTool.h"
#include "IMeasurementSelector.h"

// Athena
#include "AthenaKernel/Chrono.h"
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"
#include "GaudiKernel/EventContext.h"
#include "InDetIdentifier/SCT_ID.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "xAODMeasurementBase/UncalibratedMeasurement.h"

// STL
#include <memory>
#include <optional>
#include <semaphore>
#include <string>

// Handle Keys
#include "ActsEvent/TrackContainerHandlesHelper.h"
#include "StoreGate/CondHandleKeyArray.h"
#include "StoreGate/WriteHandleKey.h"
#include "src/detail/Definitions.h"
#include "src/detail/DuplicateSeedDetector.h"
#include "src/detail/OnTrackCalibrator.h"

namespace ActsPlugins {
class GnnPipeline;
}

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
  PublicToolHandle<ITrackingGeometryTool> m_trackingGeometryTool{
      this, "TrackingGeometryTool", ""};
  ToolHandle<ITrackParamsEstimationTool> m_paramEstimationTool{
      this, "TrackParamsEstimationTool", "", "Track Param Estimation from Seeds"};
  ToolHandle<IFitterTool> m_fitterTool{
      this, "FitterTool", "", "Track fitting tool"};

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

  Gaudi::Property<std::string> m_moduleMapPath{this, "moduleMapPath", "",
                                               "Path to the module map files"};

  Gaudi::Property<std::string> m_gnnPath{this, "gnnPath", "",
                                         "Path to the gnn file"};

  Gaudi::Property<bool> m_usePhiOverlapSps{
      this, "usePhiOverlapSps", false, "Wether to use phi overlap spacepoints"};

  Gaudi::Property<unsigned int> m_maxGpuInstances{
      this, "maxGpuInstances", 1,
      "Number of events that can be on GPU in parallel"};

  Gaudi::Property<unsigned int> m_numTrtContexts{
      this, "numTrtContexts", 1, "Number of TensorRT contexts to allocate"};

  Gaudi::Property<double> m_varianceInflation{
      this, "varianceInflation", 1.0,
      "Factor that is multiplied to all initial variances"};

  Gaudi::Property<bool> m_tightSeeds{
      this, "tightSeeds", false,
      "Use tight seeds instead of spread seeds for param estimation"};

  Gaudi::Property<double> m_edgeCut{this, "edgeCut", 0.5,
                                    "Edge cut to apply after the GNN"};

  Gaudi::Property<unsigned int> m_minCandidateMeasurements{
      this, "minCandidateMeasurements", 7,
      "Minimum number of spacepoints to cut for in the GNN candidates"};

  Gaudi::Property<double> m_minDeltaR{
      this, "minDeltaR", 10.0,
      "Minimum difference in R to build the initial parameters"};

  Gaudi::Property<bool> m_relaxCentralHoleSel{
      this, "relaxCentralHoleSel", false, "Relax holes from 2 to 4 in central region"};

  Gaudi::Property<bool> m_relaxMeasurementSel{
      this, "relaxMeasurementSel", true, "Apply 7,7,7 measurement sel"};

  Gaudi::Property<bool> m_offlineZ0Sel{
      this, "offlineZ0Sel", false, "Apply offline z0 selection"};

  Gaudi::Property<int> m_cudaDeviceIndex{this, "cudaDeviceIndex", 0,
                                         "CUDA device index for GNN inference"};

  std::unique_ptr<ActsPlugins::GnnPipeline> m_gnnPipeline;
  
  const SCT_ID *m_stripIdHelper = nullptr;

  Acts::TrackSelector::EtaBinnedConfig m_trackSelectorConfig;

  /// Private access to the logger
  const Acts::Logger &logger() const { return *m_logger; }

  /// logging instance
  std::unique_ptr<const Acts::Logger> m_logger;

  mutable std::optional<std::counting_semaphore<>> m_gpuInstanceCount
      ATLAS_THREAD_SAFE{};
};

} // namespace ActsTrk

#endif
