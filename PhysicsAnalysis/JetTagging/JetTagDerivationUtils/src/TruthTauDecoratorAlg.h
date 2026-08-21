/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRUTH_TAU_DECORATOR_ALG_H
#define TRUTH_TAU_DECORATOR_ALG_H

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

  /// Decorate jets with the kinematics of their leading and subleading
  /// ghost-associated truth taus, total and visible.
  class TruthTauDecoratorAlg final : public AthReentrantAlgorithm {

  public:
    using AthReentrantAlgorithm::AthReentrantAlgorithm;

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;
    virtual StatusCode finalize() override;

  private:
    using JetDecorKey = SG::WriteDecorHandleKey<xAOD::JetContainer>;

    // The four kinematic keys for one interpretation, so both share a single
    // description and a single fill (see KinDecor in the .cxx).
    struct KinKeys {
      JetDecorKey pt, deta, dphi, m;

      template <class OWNER>
      KinKeys(OWNER* owner, const SG::VarHandleKey& jets,
              const std::string& slot, const std::string& propTag,
              const std::string& decTag, const std::string& docPrefix)
        : pt   {owner, slot + "Pt"   + propTag, jets, "truthtau_" + slot + "_pt"   + decTag, docPrefix + "tau pT"},
          deta {owner, slot + "Deta" + propTag, jets, "truthtau_" + slot + "_deta" + decTag, docPrefix + "tau eta wrt jet axis"},
          dphi {owner, slot + "Dphi" + propTag, jets, "truthtau_" + slot + "_dphi" + decTag, docPrefix + "tau phi wrt jet axis"},
          m    {owner, slot + "M"    + propTag, jets, "truthtau_" + slot + "_m"    + decTag, docPrefix + "tau mass"} {}
    };

    // All keys for one tau slot.
    struct TauKeys {
      KinKeys total, vis;
      JetDecorKey numCharged, charge, isHadronic;

      template <class OWNER>
      TauKeys(OWNER* owner, const SG::VarHandleKey& jets,
              const std::string& slot)
        : total {owner, jets, slot, "",    "",     ""},
          vis   {owner, jets, slot, "Vis", "_vis", "visible "},
          numCharged {owner, slot + "NumCharged", jets, "truthtau_" + slot + "_numCharged", "tau prongness (numCharged)"},
          charge     {owner, slot + "Charge",     jets, "truthtau_" + slot + "_charge",     "tau charge (+-1)"},
          isHadronic {owner, slot + "IsHadronic", jets, "truthtau_" + slot + "_isHadronic", "1 for a hadronic tau decay, 0 for leptonic"} {}

      std::array<JetDecorKey*, 11> all() {
        return {&total.pt, &total.deta, &total.dphi, &total.m,
                &vis.pt,   &vis.deta,   &vis.dphi,   &vis.m,
                &numCharged, &charge, &isHadronic};
      }
    };

    // Per-event writer; defined in the .cxx.
    struct TauDecor;

    // Read through the generic IParticle interface.
    using GhostLinks = std::vector<ElementLink<xAOD::IParticleContainer>>;

    // Declared first: the decoration keys below reference it.
    SG::ReadHandleKey<xAOD::JetContainer> m_jetKey {
      this, "jets", "", "Jet container to decorate"};
    SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthTausKey {
      this, "truthTaus", "TruthTaus",
      "TruthTaus container providing the visible-tau 4-momenta and numCharged"};
    SG::ReadDecorHandleKey<xAOD::JetContainer> m_ghostTauKey {
      this, "ghostTauAssocName", m_jetKey, "GhostTausFinal",
      "Ghost-associated tau links on the jets"};

    TauKeys m_lead{this, m_jetKey, "lead"};
    TauKeys m_sublead{this, m_jetKey, "sublead"};
    JetDecorKey m_nGhostTausKey {
      this, "nGhostTaus", m_jetKey, "nGhostTaus",
      "Number of ghost-associated truth taus found on this jet"};

    // Ghost taus with no TruthTaus match, reported in finalize().
    mutable std::atomic<unsigned long long> m_nTausNoVisMatch{0};
  };

} // end namespace ftag

#endif
