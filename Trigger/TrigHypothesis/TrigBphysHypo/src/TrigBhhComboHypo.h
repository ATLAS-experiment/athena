/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIG_TrigBhhComboHypo_H
#define TRIG_TrigBhhComboHypo_H

#include <string>
#include <vector>
#include <utility>

#include "Gaudi/Property.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODTracking/VertexAuxContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODTrigger/TrigComposite.h"
#include "xAODTrigBphys/TrigBphys.h"
#include "xAODTrigBphys/TrigBphysContainer.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include "TrigCompositeUtils/TrigCompositeUtils.h"
#include "DecisionHandling/ComboHypo.h"

#include "TrkVKalVrtFitter/TrkVKalVrtFitter.h"
#include "InDetConversionFinderTools/VertexPointEstimator.h"

#include "AthenaMonitoringKernel/Monitored.h"
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"

#include "ITrigBphysState.h"
#include "TrigBmumuxComboHypoTool.h"

#include "Constants.h"
typedef struct PDG20 PDG;


/**
 * @class TrigBhhState
 * @brief State class for TrigBhhComboHypo algorithm
 */
class TrigBhhState: public ::ITrigBphysState {
 public:
  TrigBhhState() = delete;
  TrigBhhState(const EventContext& context,
               const TrigCompositeUtils::DecisionContainer& previousDecisions,
               TrigCompositeUtils::DecisionContainer& decisions,
               xAOD::TrigBphysContainer* trigBphysCollection = nullptr,
               const InDet::BeamSpotData* beamSpotData = nullptr)
      : ITrigBphysState(context, previousDecisions, decisions, trigBphysCollection, beamSpotData) {}
  virtual ~TrigBhhState() = default;

  // EFCB muon candidates from mergeMuonsFromDecisions()
  struct Muon {
    ElementLink<xAOD::MuonContainer> link;
    std::vector<ElementLink<TrigCompositeUtils::DecisionContainer>> decisionLinks;
    TrigCompositeUtils::DecisionIDContainer decisionIDs;
  };
  std::vector<Muon> muons;

  // tracks from mergeTracksFromViews()
  std::vector<ElementLink<xAOD::TrackParticleContainer>> tracks;
};


/**
 * @class TrigBhhComboHypo
 * @brief EF hypothesis algorithm for B -> h+ h- decays (h = K, pi):
 *        B0 -> K+ pi-
 *        B_s0 -> K+ K- etc.
 */
class TrigBhhComboHypo: public ::ComboHypo {
 public:
  TrigBhhComboHypo(const std::string& name, ISvcLocator* pSvcLocator);
  TrigBhhComboHypo() = delete;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& context) const override;

 private:
  /**
   * @brief Go through state.previousDecisions(), fetch xAOD::Muons objects attached to decisions
   * and save links to them in state.muons().
   */
  StatusCode mergeMuonsFromDecisions(TrigBhhState&) const;

  /**
   * @brief Go through state.previousDecisions() and fetch xAOD::TrackParticle objects associated with the nearest SG::View.
   * Enable overlap removal to get collection of unique objects at state.tracks().
   * Tracks associated with state.muons() are also removed from track collection.
   */
  StatusCode mergeTracksFromViews(TrigBhhState&) const;

  /**
   * @brief Make all possible combinations from state.tracks(), fit tracks to the common vertex,
   * create xAOD::TrigBphys objects and put them into state.trigBphysCollection()
   */
  StatusCode findBhhCandidates(TrigBhhState&) const;

  /**
   * @brief Create a decision for each xAOD::TrigBphys object from state.trigBphysCollection() and
   * use hypoTools() to assign correct decisionIDs.
   */
  StatusCode createDecisionObjects(TrigBhhState&) const;

  /**
   * @brief Perform a vertex fit on selected tracks
   * @param context the event context used to make the vertex threadsafe
   * @param trackParticleLinks the trackParticles to fit
   * @param trkMass track mass hypothesis for mass calculations
   * @return The fitted vertex - null if fit fails or is very low quality
   */
  std::unique_ptr<xAOD::Vertex> fit(
      const EventContext& context,
      const std::vector<ElementLink<xAOD::TrackParticleContainer>>& trackParticleLinks,
      const std::vector<double>& trkMass = {PDG::mKaon, PDG::mKaon}) const;

  /**
   * @brief Fill the trigger object that may be stored for debugging or matching.
   * @param triggerObject the trigger object
   * @param vertex the fitted decay vertex
   * @param productionVertex position of the beamspot
   * @param trkMass track mass hypothesis for mass calculations
   * @return StatusCode to indicate success or failure
   */
  StatusCode fillTriggerObject(
      xAOD::TrigBphys& triggerObject,
      const xAOD::Vertex& vertex,
      const Amg::Vector3D& productionVertex,
      const std::vector<double>& trkMass = {PDG::mKaon, PDG::mKaon}) const;

  /**
   * @brief Returns false for the tracks with opposite charges.
   * Otherwise calculates the deltaR(lhs, rhs) and compares it with threshold value m_deltaR.
   * Should be used to remove muon InDet track from the track collection and to resolve ambiguity for muons from different views.
   */
  bool isIdenticalTracks(const xAOD::TrackParticle* lhs, const xAOD::TrackParticle* rhs) const;
  bool isIdenticalTracks(const xAOD::Muon* lhs, const xAOD::Muon* rhs) const;

  /**
   * @brief Returns the transverse decay length of a particle Lxy in [mm].
   * It is defined as the transverse distance between the production and decay vertices projected along the transverse momentum of the particle.
   */
  double Lxy(const Amg::Vector3D& productionVertex, const Amg::Vector3D& decayVertex, const std::vector<xAOD::TrackParticle::GenVecFourMom_t>& momenta) const;

  SG::ReadHandleKey<xAOD::TrackParticleContainer> m_trackParticleContainerKey {this,
    "TrackCollectionKey", "InDetTrackParticles", "input TrackParticle container name"};
  SG::WriteHandleKey<xAOD::TrigBphysContainer> m_trigBphysContainerKey {this,
    "TrigBphysCollectionKey", "TrigBphysContainer", "output TrigBphysContainer name"};
  SG::ReadCondHandleKey<InDet::BeamSpotData>
    m_beamSpotKey {this, "BeamSpotKey", "BeamSpotData", "SG key for beam spot"};

  // general properties
  Gaudi::Property<bool> m_applyMuonRemoval {this, "ApplyMuonRemoval", false,
    "if True, remove tracks accosiated with muons from m_trackParticleContainer"};
  Gaudi::Property<double> m_deltaR {this,
    "DeltaR", 0.01, "minimum deltaR between same-sign tracks (overlap removal)"};
  Gaudi::Property<double> m_trkPt {this,
    "Bhh_trackPtThreshold", 2000., "minimum track transverse momenta"};
  Gaudi::Property<std::pair<double, double>> m_massRange {this,
    "Bhh_massRange", {4500., 6500.}, "B0/B_s0 mass range"};
  Gaudi::Property<double> m_chi2 {this,
    "Bhh_chi2", 20., "maximum chi2 of the fitted B0 vertex"};
  Gaudi::Property<size_t> m_fitAttemptsWarningThreshold {this,
    "FitAttemptsWarningThreshold", 200, "Events processing this many calls of the vertex fitter will generate a WARNING message (time-out protect)"};
  Gaudi::Property<size_t> m_fitAttemptsBreakThreshold {this,
    "FitAttemptsBreakThreshold", 1000, "Events processing this many calls of the vertex fitter will generate a second WARNING message and the loop over combinations will be terminated at this point (time-out protect)"};

  // external tools
  ToolHandle<InDet::VertexPointEstimator> m_vertexPointEstimator {this,
    "VertexPointEstimator", "", "tool to find starting point for the vertex fitter"};
  ToolHandle<Trk::TrkVKalVrtFitter> m_vertexFitter {this,
    "VertexFitter", "", "VKalVrtFitter tool to fit tracks into the common vertex"};
  ToolHandle<GenericMonitoringTool> m_monTool {this,
    "MonTool", "", "monitoring tool"};

  TrigCompositeUtils::DecisionIDContainer m_allowedIDs;
};

#endif  // TRIG_TrigBhhComboHypo_H
