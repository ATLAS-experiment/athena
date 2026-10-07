/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRUTH_TAU_DECORATOR_ALG_H
#define TRUTH_TAU_DECORATOR_ALG_H

#include "JetTagDerivationUtils/VariableMule.h"

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"

#include "xAODJet/JetContainer.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODBase/IParticleContainer.h"

#include "AthLinks/ElementLink.h"

#include <array>
#include <atomic>
#include <string>
#include <vector>

namespace ftag {

  /// Match jets to their ghost-associated truth taus and decorate the jets
  /// with the tau properties.
  ///
  /// The ghost association defines the match, the leading and subleading tau
  /// fill one decoration slot each, and the visible quantities are taken from
  /// the matching TruthTaus entry.
  class TruthTauDecoratorAlg final : public AthReentrantAlgorithm {

  public:
    TruthTauDecoratorAlg(const std::string& name, ISvcLocator* pSvcLocator);

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;
    virtual StatusCode finalize() override;

  private:
    using JetDecorKey = SG::WriteDecorHandleKey<xAOD::JetContainer>;

    // Copied variables go truth tau -> jet, so the mules work through the
    // generic IParticle interface, as do the ghost links.
    using GhostLinks = std::vector<ElementLink<xAOD::IParticleContainer>>;

    // One ranked tau: the quantities this algorithm computes itself, plus the
    // variables copied straight off the matched TruthTaus entry. The slot name
    // is the decoration suffix, so lead and sublead never collide.
    struct Slot {
      JetDecorKey matched, deltaR, deltaPt, dEta, dPhi, pt, m, charge;

      VariableMule<float, xAOD::IParticleContainer> floats{NAN};
      VariableMule<double, xAOD::IParticleContainer> doubles{NAN};
      VariableMule<int, xAOD::IParticleContainer> ints{-1};
      VariableMule<uint, xAOD::IParticleContainer> uints{0};
      VariableMule<ulong, xAOD::IParticleContainer> ulongs{0};
      VariableMule<char, xAOD::IParticleContainer> chars{-1};

      template <class OWNER>
      Slot(OWNER* owner, const SG::VarHandleKey& jets, const std::string& slot)
        : matched {owner, slot + "MatchedKey", jets, "matchedTo"  + slot, "Whether the jet is matched to a truth tau"},
          deltaR  {owner, slot + "DeltaRKey",  jets, "deltaRTo"   + slot, "Delta R between the jet axis and the visible tau"},
          deltaPt {owner, slot + "DeltaPtKey", jets, "deltaPtTo"  + slot, "Jet pT minus the visible tau pT"},
          dEta    {owner, slot + "DEtaKey",    jets, "dEtaTo"     + slot, "Visible tau eta wrt the jet axis"},
          dPhi    {owner, slot + "DPhiKey",    jets, "dPhiTo"     + slot, "Visible tau phi wrt the jet axis"},
          pt      {owner, slot + "PtKey",      jets, "ptFrom"     + slot, "Total (neutrino-inclusive) tau pT"},
          m       {owner, slot + "MKey",       jets, "mFrom"      + slot, "Total (neutrino-inclusive) tau mass"},
          charge  {owner, slot + "ChargeKey",  jets, "chargeFrom" + slot, "Tau charge"} {}

      std::array<JetDecorKey*, 8> computed() {
        return {&matched, &deltaR, &deltaPt, &dEta, &dPhi, &pt, &m, &charge};
      }
    };

    // Per-event writer; defined in the .cxx.
    struct SlotDecor;

    StatusCode initializeSlot(Slot& slot, const std::string& prefix);

    // Declared first: the decoration keys below reference it.
    SG::ReadHandleKey<xAOD::JetContainer> m_jetKey {
      this, "jets", "", "Jet container to decorate"};
    SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthTausKey {
      this, "truthTaus", "TruthTaus",
      "TruthTaus container providing the visible tau 4-momenta"};
    SG::ReadDecorHandleKey<xAOD::JetContainer> m_ghostTauKey {
      this, "ghostTauAssocName", m_jetKey, "GhostTausFinal",
      "Ghost-associated tau links on the jets"};

    Gaudi::Property<bool> m_requireIsolatedTau {
      this, "requireIsolatedTau", true,
      "Only consider taus with classifierParticleType == IsoTau, i.e. drop "
      "taus from b- and c-hadron decays"};
    Gaudi::Property<float> m_minTruthTauPt {
      this, "minTruthTauPt", 0.0f,
      "Minimum visible tau pT in MeV. The ghost association already applies a "
      "5 GeV cut on the total tau pT, so this can only tighten it"};

    Slot m_lead{this, m_jetKey, "TruthTaus"};
    Slot m_sublead{this, m_jetKey, "SubleadTruthTaus"};

    JetDecorKey m_nGhostTausKey {
      this, "nGhostTaus", m_jetKey, "nGhostTaus",
      "Number of ghost-associated truth taus considered for this jet"};

    // Ghost taus with no TruthTaus match, reported in finalize().
    mutable std::atomic<unsigned long long> m_nTausNoVisMatch{0};
  };

} // end namespace ftag

#endif
