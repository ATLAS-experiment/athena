/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSTRACKRECONSTRUCTION_TRACKEXTENSIONALG_H
#define ACTSTRACKRECONSTRUCTION_TRACKEXTENSIONALG_H
#include "GeoPrimitives/GeoPrimitives.h"
#include "Acts/EventData/TrackProxy.hpp"
#include "Acts/EventData/TrackParameters.hpp"
#include "Acts/TrackFinding/CombinatorialKalmanFilter.hpp"
#include "Acts/EventData/GenericBoundTrackParameters.hpp"


#include "ActsEvent/ProtoTrackCollection.h"
#include "ActsEvent/TrackContainerHandlesHelper.h"
#include "ActsEventCnv/IActsToTrkConverterTool.h"
#include "ActsGeometryInterfaces/IActsExtrapolationTool.h"
#include "ActsGeometryInterfaces/IActsTrackingGeometryTool.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "ActsInterop/Logger.h"

#include "TrackFindingData.h"
// Framework includes
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

// STL includes
#include <string>

/**
 * @class TrackExtensionAlg
 * @brief Alg that starts from proto tracks and runs CFK on a (sub)set of
 *provided pixel measurements
 **/
namespace ActsTrk{
class TrackExtensionAlg : public AthReentrantAlgorithm {
 public:
  TrackExtensionAlg(const std::string& name, ISvcLocator* pSvcLocator);

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& context) const override;
  using CKFOptions = Acts::CombinatorialKalmanFilterOptions<ActsTrk::UncalibSourceLinkAccessor::Iterator, detail::RecoTrackContainer>;

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
      m_pixelDetEleCollKey{this, "PixelDetEleCollKey", "",
                           "Key of SiDetectorElementCollection for Pixel"};
  ToolHandle<IActsTrackingGeometryTool> m_trackingGeometryTool{
      this, "TrackingGeometryTool", ""};
  ToolHandle<IActsExtrapolationTool> m_extrapolationTool{this, "ExtrapolationTool", ""};

  std::unique_ptr<detail::CKF_config> m_ckfConfig;
  std::unique_ptr<const Acts::Logger> m_logger;

  CKFOptions buildCKFOptions(const EventContext& context, const Acts::PerigeeSurface* perigeeSurface) const;
  detail::TrackFindingMeasurements collectMeasurements(const EventContext& context) const;

  Acts::CalibrationContext m_calibrationContext; // this will change in future to be updatable event by event



};
} // EOF namespace
#endif  // ACTSTRACKRECONSTRUCTION_TRACKEXTENSIONALG_H
