/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
// Builds one seed jet per tau, for the jet tools that need the tau seed jets
// when these are not available, e.g. the truth labelling in derivations.
//
// The tau keeps the clusters of its seed jet as clusterLinks, which become
// the constituents of the seed jet. The ghosts are associated by clustering
// these clusters again, together with the surrounding clusters and the ghost
// truth particles, using the seed jet definition. The ghosts that end up in
// the jet holding the tau clusters are associated to the seed jet, in the
// same way as in the jet reconstruction.
//
// Each tau is clustered on its own, so unlike the full event ghost
// association a truth particle can be associated to more than one seed jet.

#ifndef PARTICLEJETTOOLS_TAUSEEDJETALG_H
#define PARTICLEJETTOOLS_TAUSEEDJETALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "JetInterface/IJetModifier.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include "xAODBase/IParticleContainer.h"
#include "xAODJet/JetContainer.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODTruth/TruthParticleContainer.h"

#include <string>
#include <vector>

namespace ParticleJetTools {

class TauSeedJetAlg : public AthReentrantAlgorithm {
public:
  TauSeedJetAlg(const std::string& name, ISvcLocator* loc);
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

private:
  SG::ReadHandleKey<xAOD::TauJetContainer> m_tauKey{
    this, "TauJets", "TauJets", "Taus to build the seed jets for"};
  SG::ReadHandleKey<xAOD::IParticleContainer> m_constituentKey{
    this, "Constituents", "LCOriginTopoClusters",
    "Seed jet constituents, index-parallel to the container of the tau clusterLinks"};
  SG::ReadHandleKeyArray<xAOD::TruthParticleContainer> m_ghostKeys{
    this, "GhostCollections",
    {"TruthLabelBHadronsFinal", "TruthLabelCHadronsFinal",
     "TruthLabelTausFinal", "TruthLabelPartons"},
    "Ghost truth particles, associated as Ghost<name without TruthLabel>"};
  SG::WriteHandleKey<xAOD::JetContainer> m_jetKey{
    this, "SeedJets", "TauSeedJets", "Seed jets, one per tau in the same order"};
  SG::WriteDecorHandleKey<xAOD::TauJetContainer> m_linkKey{
    this, "SeedJetLink", "", "Optional tau decoration linking to its seed jet"};
  SG::WriteDecorHandleKey<xAOD::TauJetContainer> m_statusKey{
    this, "StatusDecoration", "",
    "Optional, e.g. GhostSeedJetStatus: 0 one jet holds exactly the tau clusters, "
    "1 it holds other clusters as well, 2 tau clusters split over jets, -1 no clusters"};
  ToolHandleArray<IJetModifier> m_modifiers{
    this, "Modifiers", {}, "Jet modifiers run on the seed jets, e.g. the truth labelling"};

  Gaudi::Property<float> m_jetRadius{
    this, "JetRadius", 0.4, "Radius of the anti-kt seed jet definition"};
  Gaudi::Property<float> m_neighbourDR{
    this, "NeighbourDR", 1.0,
    "Constituents within this dR of the seed axis are clustered with the tau clusters"};
  Gaudi::Property<float> m_ghostDR{
    this, "GhostDR", 1.0, "Only ghosts within this rapidity-phi dR of the seed axis are clustered"};

  std::vector<std::string> m_ghostLabels;
};

}

#endif
