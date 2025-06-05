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
#include "ActsGeometryInterfaces/ITrackingGeometryTool.h"
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "InDetRecToolInterfaces/IInDetEtaDependentCutsSvc.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"

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
  virtual StatusCode finalize() override;
  virtual StatusCode execute(const EventContext &ctx) const override;

 private:
  ToolHandle<GenericMonitoringTool> m_monTool{this, "MonTool", "",
                                              "Monitoring tool"};
  PublicToolHandle<ActsTrk::ITrackingGeometryTool> m_trackingGeometryTool{
      this, "TrackingGeometryTool", ""};

  SG::ReadHandleKey<ActsTrk::TrackContainer> m_tracksKey{
      this, "TracksLocation", "", "Input track collection"};
  ActsTrk::MutableTrackContainerHandlesHelper m_resolvedTracksBackendHandles{this};
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
  Gaudi::Property<std::size_t> m_minUnshared{
      this, "MinUnshared", 5, "Minimum number of unshared hits per track."};

  Gaudi::Property<bool> m_useAmbiguityScoring{
      this, "UseAmbiguityScoring", false,
      "Flag to enable/disable ambiguity function."};

  Gaudi::Property<std::string> m_jsonFileName{
      this, "jsonFileName", "ScoreBasedAmbiguity_Config.json",
      "Name of the JSON file that contains the config file."};

  Gaudi::Property<bool> m_countSharedHits{this, "countSharedHits", true, "add shared hit flags to tracks"};

  std::unique_ptr<Acts::ScoreBasedAmbiguityResolution> m_ambi{nullptr};

  /** ITk eta-dependent cuts*/
  ServiceHandle<InDet::IInDetEtaDependentCutsSvc> m_etaDependentCutsSvc{
      this, "InDetEtaDependentCutsSvc", ""};

  public:
    enum EStat {
      kNInputTracks,
      kNResolvedTracks,
      kNSharedHits,
      kNStat
    };

  private:
    mutable std::array<std::atomic<unsigned int>, kNStat> m_stat ATLAS_THREAD_SAFE{};
};

}  // namespace ActsTrk

#endif
