/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSEVENTCNV_ActsToTrkConverterTool_H
#define ACTSEVENTCNV_ActsToTrkConverterTool_H

#include <tuple>

// ATHENA
#include "AthenaBaseComps/AthAlgTool.h"

#include "GaudiKernel/EventContext.h"
#include "TrkParameters/TrackParameters.h" //typedef, cannot fwd declare
#include "xAODTracking/TrackJacobianContainer.h"
#include "xAODTracking/TrackParametersContainer.h"
#include "xAODTracking/TrackStateContainer.h"
#include "xAODTracking/TrackMeasurementContainer.h"
#include "TrkEventPrimitives/PdgToParticleHypothesis.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"

// PACKAGE
#include "ActsEventCnv/IActsToTrkConverterTool.h"
#include "ActsGeometryInterfaces/IActsTrackingGeometryTool.h"
#include "Acts/EventData/TrackParameters.hpp"
#include "MuonReadoutGeometry/MuonDetectorManager.h"

namespace ActsTrk {
class ActsToTrkConverterTool : public extends<AthAlgTool, IActsToTrkConverterTool>
{

public:
  virtual StatusCode initialize() override;

  using base_class::base_class;



  /// Find the ATLAS surface corresponding to the Acts surface 
  /// Only work if the Acts surface has an associated detector element
  /// (Pixel and SCT)
  virtual 
  const Trk::Surface&
  actsSurfaceToTrkSurface(const Acts::Surface &actsSurface) const override;

  /// Find the Acts surface corresponding to the ATLAS surface 
  /// Use a map associating ATLAS ID to Acts surfaces
  /// (Pixel and SCT)
  virtual
  const Acts::Surface&
  trkSurfaceToActsSurface(const Trk::Surface &atlasSurface) const override;

  /// Create an SourceLink from an ATLAS measurment
  /// Works for 1 and 2D measurmenent.
  /// A pointer to the measurment is kept in the SourceLink
  virtual
  Acts::SourceLink
  trkMeasurementToSourceLink(const Acts::GeometryContext& gctx, const Trk::MeasurementBase &measurement) const override;

  /// Transform an ATLAS track into a vector of SourceLink to be use in the avts tracking
  /// Transform both measurement and outliers.
  virtual std::vector<Acts::SourceLink> trkTrackToSourceLinks(
                       const Acts::GeometryContext& gctx, const Trk::Track& track) const override;

  /// Create Acts TrackParameter from ATLAS one.
  /// Take care of unit conversion between the two.  
  virtual
  const Acts::BoundTrackParameters
  trkTrackParametersToActsParameters(const Trk::TrackParameters &atlasParameter, const Acts::GeometryContext& gctx, Trk::ParticleHypothesis = Trk::pion) const override;

  /// Create ATLAS TrackParameter from Acts one.
  /// Take care of unit conversion between the two.  
  virtual
  std::unique_ptr<Trk::TrackParameters>
  actsTrackParametersToTrkParameters(const Acts::BoundTrackParameters &actsParameter, const Acts::GeometryContext& gctx) const override;

  /** Convert TrackCollection to Acts track container. 
   * @param tc The track container to fill
  */
  virtual 
  void trkTrackCollectionToActsTrackContainer(ActsTrk::MutableTrackContainer &tc, const TrackCollection& trackColl, const Acts::GeometryContext& gctx) const override;

  virtual
  const IActsTrackingGeometryTool*
  trackingGeometryTool() const override
  {
    return m_trackingGeometryTool.get();
  };

private:
  bool actsTrackParameterPositionCheck(
     const Acts::BoundTrackParameters& actsParameter,
     const Trk::TrackParameters& tsos, const Acts::GeometryContext& gctx) const;

  ToolHandle<IActsTrackingGeometryTool> m_trackingGeometryTool{this, "TrackingGeometryTool", 
                                                               "ActsTrackingGeometryTool"};
  
  std::shared_ptr<const Acts::TrackingGeometry> m_trackingGeometry{};
  std::unordered_map<Identifier, const Acts::Surface*> m_actsSurfaceMap{};

  Gaudi::Property<bool> m_visualDebugOutput{
     this, "VisualDebugOutput", false,
     "Print additional output for debug plots"};

  Trk::PdgToParticleHypothesis m_pdgToParticleHypothesis;

  Gaudi::Property<bool> m_extractMuonSurfaces{
     this, "ExtractMuonSurfaces", false,
     "If True, use the MuonDetectorManager to extract the Muon surfaces"};
  
  ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
  
  /** @brief Detector manager to fetch the legacy Trk surfaces */
  SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_muonMgrKey{this, "MuonManagerKey", "MuonDetectorManager"};

};

}; // namespace ActsTrk

#endif
