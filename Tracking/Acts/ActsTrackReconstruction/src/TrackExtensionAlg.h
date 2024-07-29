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
#include "ActsEvent/TrackContainer.h"
#include "ActsEvent/ProtoTrackCollection.h"
#include "ActsEventCnv/IActsToTrkConverterTool.h"
#include "ActsToolInterfaces/IOnTrackCalibratorTool.h"

// Athena
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "GaudiKernel/EventContext.h"

// STL
#include <string>
#include <memory>

// Handle Keys
#include "StoreGate/WriteHandleKey.h"
#include "ActsEvent/TrackContainerHandlesHelper.h"
#include "TrackFindingData.h"


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
      ActsTrk::UncalibSourceLinkAccessor::Iterator, detail::RecoTrackContainer>;

 private:
  SG::ReadHandleKey<xAOD::PixelClusterContainer> m_PixelClusters{
      this, "PixelClusterContainer", "", "the pix clusters"};
  SG::ReadHandleKey<ActsTrk::ProtoTrackCollection> m_protoTrackCollectionKey{
      this, "ProtoTracksLocation", "", "Input proto tracks"};
  SG::WriteHandleKey<ActsTrk::TrackContainer> m_trackContainerKey{
      this, "ACTSTracksLocation", "",
      "Output track collection (ActsTrk variant)"};
  ActsTrk::MutableTrackContainerHandlesHelper m_tracksBackendHandlesHelper;
  ToolHandle<ActsTrk::IActsToTrkConverterTool> m_ATLASConverterTool{
      this, "ATLASConverterTool", ""};
  SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection>
      m_pixelDetEleCollKey{this, "PixelDetEleCollKey",
                           "ITkPixelDetectorElementCollection",
                           "Key of SiDetectorElementCollection for Pixel"};
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

  std::unique_ptr<detail::CKF_config> m_ckfConfig;
  std::unique_ptr<const Acts::Logger> m_logger;

  detail::TrackFindingMeasurements collectMeasurements(
      const EventContext& context) const;

  Acts::CalibrationContext
      m_calibrationContext;  // this will change in future to be updatable event
                             // by event
};
}  // namespace ActsTrk
#endif  // ACTSTRACKRECONSTRUCTION_TRACKEXTENSIONALG_H
