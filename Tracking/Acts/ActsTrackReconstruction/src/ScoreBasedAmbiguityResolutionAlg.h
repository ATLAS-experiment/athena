/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKFINDING_SCOREBASEDAMBIGUITYRESOLUTIONALG_H
#define ACTSTRKFINDING_SCOREBASEDAMBIGUITYRESOLUTIONALG_H 1

// Base Class
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

// Gaudi includes
#include "Gaudi/Property.h"
#include "GaudiKernel/ToolHandle.h"

// ACTS
#include "Acts/AmbiguityResolution/ScoreBasedAmbiguityResolution.hpp"
#include "Acts/Definitions/Units.hpp"
#include "Acts/Utilities/Logger.hpp"
#include "ActsEvent/TrackContainer.h"

// Athena
#include "ActsGeometryInterfaces/IActsTrackingGeometryTool.h"
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"
#include "InDetRecToolInterfaces/IInDetEtaDependentCutsSvc.h"


// Handle Keys
#include <memory>
#include <string>

#include "ActsEvent/TrackContainerHandlesHelper.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

namespace ActsTrk {

class ScoreBasedAmbiguityResolutionAlg : public AthReentrantAlgorithm {
 public:
  ScoreBasedAmbiguityResolutionAlg(const std::string &name,
                                   ISvcLocator *pSvcLocator);

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext &ctx) const override;

 private:
  ToolHandle<GenericMonitoringTool> m_monTool{this, "MonTool", "",
                                              "Monitoring tool"};
  ToolHandle<IActsTrackingGeometryTool> m_trackingGeometryTool{
      this, "TrackingGeometryTool", ""};

  SG::ReadHandleKey<ActsTrk::TrackContainer> m_tracksKey{
      this, "TracksLocation", "", "Input track collection"};
  ActsTrk::MutableTrackContainerHandlesHelper m_resolvedTracksBackendHandles;
  SG::WriteHandleKey<ActsTrk::TrackContainer> m_resolvedTracksKey{
      this, "ResolvedTracksLocation", "",
      "Ambiguity resolved output track collection"};

  Gaudi::Property<double> m_minScore{this, "MinScore", 0.0,
                                     "Minimum score for track selection."};
  Gaudi::Property<double> m_minScoreSharedTracks{
      this, "MinScoreSharedTracks", 0.0,
      "Minimum score for shared track selection."};
  Gaudi::Property<std::size_t> m_maxSharedTracksPerMeasurement{
      this, "MaxSharedTracksPerMeasurement", 10,
      "Maximum number of shared tracks per measurement."};
  Gaudi::Property<std::size_t> m_maxShared{
      this, "MaxShared", 5, "Maximum number of shared hit per track."};

  Gaudi::Property<double> m_pTMin{this, "PTMin", 0 * Acts::UnitConstants::GeV,
                                  "Minimum transverse momentum."};
  Gaudi::Property<double> m_pTMax{this, "PTMax", 1e5 * Acts::UnitConstants::GeV,
                                  "Maximum transverse momentum."};

  Gaudi::Property<double> m_phiMin{this, "PhiMin",
                                   -M_PI *Acts::UnitConstants::rad,
                                   "Minimum azimuthal angle."};
  Gaudi::Property<double> m_phiMax{this, "PhiMax",
                                   M_PI *Acts::UnitConstants::rad,
                                   "Maximum azimuthal angle."};

  Gaudi::Property<double> m_etaMin{this, "EtaMin", -5,
                                   "Minimum pseudorapidity."};
  Gaudi::Property<double> m_etaMax{this, "EtaMax", 5,
                                   "Maximum pseudorapidity."};

  Gaudi::Property<bool> m_useAmbiguityFunction{
      this, "UseAmbiguityFunction", false,
      "Flag to enable/disable ambiguity function."};

  Gaudi::Property<std::string> m_jsonFileName{
      this, "jsonFileName", "ScoreBasedAmbiguity_Config.json",
      "Name of the JSON file that contains the config file."};

  std::unique_ptr<Acts::ScoreBasedAmbiguityResolution> m_ambi{nullptr};

  /** ITk eta-dependent cuts*/
  ServiceHandle<InDet::IInDetEtaDependentCutsSvc> m_etaDependentCutsSvc{
      this, "InDetEtaDependentCutsSvc", ""};
};

}  // namespace ActsTrk

#endif
