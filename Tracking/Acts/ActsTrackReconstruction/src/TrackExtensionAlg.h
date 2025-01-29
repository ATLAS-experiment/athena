/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSTRACKRECONSTRUCTION_TRACKEXTENSIONALG_H
#define ACTSTRACKRECONSTRUCTION_TRACKEXTENSIONALG_H
// Base Class
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

// Gaudi includes
#include "GaudiKernel/ToolHandle.h"

// Tools
#include "ActsGeometryInterfaces/IActsExtrapolationTool.h"
#include "ActsGeometryInterfaces/IActsTrackingGeometryTool.h"

// ACTS
#include "Acts/EventData/TrackContainer.hpp"

// ActsTrk
#include "ActsEvent/ProtoTrackCollection.h"
#include "ActsEvent/TrackContainer.h"
#include "ActsGeometry/DetectorElementToActsGeometryIdMap.h"
#include "ActsToolInterfaces/IOnTrackCalibratorTool.h"
#include "ActsToolInterfaces/IFitterTool.h"

// Athena
#include "GaudiKernel/EventContext.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODTruth/TruthParticleContainer.h"

// STL
#include <memory>
#include <string>

// Handle Keys
#include "ActsEvent/TrackContainerHandlesHelper.h"
#include "StoreGate/WriteHandleKey.h"
#include "src/detail/Definitions.h"
#include "src/detail/DuplicateSeedDetector.h"
#include "src/detail/TrackFindingMeasurements.h"
#include "src/TrackStatePrinterTool.h"

/**
 * @class TrackExtensionAlg
 * @brief Alg that starts from proto tracks and runs CFK on a (sub)set of
 *provided pixel measurements
 **/
namespace ActsTrk {
class TrackExtensionAlg : public AthReentrantAlgorithm {
 public:
  TrackExtensionAlg(const std::string& name, ISvcLocator* pSvcLocator);

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& context) const override;
  using CKFOptions = Acts::CombinatorialKalmanFilterOptions<
    ActsTrk::detail::UncalibSourceLinkAccessor::Iterator, detail::RecoTrackContainer>;

 private:
  SG::ReadHandleKey<xAOD::PixelClusterContainer> m_pixelClusters{
      this, "PixelClusterContainer", "", "the pix clusters"};
  SG::ReadHandleKey<ActsTrk::ProtoTrackCollection> m_protoTrackCollectionKey{
      this, "ProtoTracksLocation", "", "Input proto tracks"};
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthParticlesKey{
      this, "TruthLocation", "",
      "Truth container (to be enabled only for debugging)"};
  SG::WriteHandleKey<ActsTrk::TrackContainer> m_trackContainerKey{
      this, "ACTSTracksLocation", "",
      "Output track collection (ActsTrk variant)"};
  ActsTrk::MutableTrackContainerHandlesHelper m_tracksBackendHandlesHelper;
  SG::ReadCondHandleKey<ActsTrk::DetectorElementToActsGeometryIdMap>
      m_detectorElementToGeometryIdMapKey{
          this, "DetectorElementToActsGeometryIdMapKey",
          "DetectorElementToActsGeometryIdMap",
          "Map which associates detector elements to Acts Geometry IDs"};

  ToolHandle<IActsTrackingGeometryTool> m_trackingGeometryTool{
      this, "TrackingGeometryTool", ""};
  ToolHandle<IActsExtrapolationTool> m_extrapolationTool{
      this, "ExtrapolationTool", ""};
  ToolHandle<ActsTrk::IOnTrackCalibratorTool<detail::RecoTrackStateContainer>>
      m_pixelCalibTool{this, "PixelCalibrator", "",
                       "Opt. pixel measurement calibrator"};
  ToolHandle<ActsTrk::IOnTrackCalibratorTool<detail::RecoTrackStateContainer>>
      m_stripCalibTool{this, "StripCalibrator", "",
                       "Opt. strip measurement calibrator"};
  ToolHandle<ActsTrk::IOnTrackCalibratorTool<detail::RecoTrackStateContainer>>
      m_hgtdCalibTool{this, "HGTDCalibrator", "",
                       "Opt. HGTD measurement calibrator"};                       
  ToolHandle<ActsTrk::TrackStatePrinterTool> m_trackStatePrinter{
      this, "TrackStatePrinter", "", "optional track state printer"};
  ToolHandle<ActsTrk::IFitterTool> m_actsFitter{
      this, "ActsFitter", "", "Choice of Acts Fitter (Kalman by default)"};

  Gaudi::Property<bool> m_propagateForward{this, "PropagateForward", false,
                                           "If true propagate forward"};

  std::unique_ptr<detail::CKF_config> m_ckfConfig;
  std::unique_ptr<const Acts::Logger> m_logger;

  detail::TrackFindingMeasurements collectMeasurements(
      const EventContext& context,
      const ActsTrk::DetectorElementToActsGeometryIdMap&
          detectorElementToGeometryIdMap) const;

  Acts::CalibrationContext
      m_calibrationContext;  // this will change in future to be updatable event
                             // by event
};
}  // namespace ActsTrk
#endif  // ACTSTRACKRECONSTRUCTION_TRACKEXTENSIONALG_H
