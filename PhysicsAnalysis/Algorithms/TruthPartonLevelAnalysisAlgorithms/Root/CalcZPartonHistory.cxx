/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>
/// @author Steffen Korn <steffen.korn@cern.ch>

#include "PartonHistory/CalcPartonHistory.h"
#include "VectorHelpers/DecoratorHelpers.h"
#include "VectorHelpers/LorentzHelper.h"

#include "AthContainers/ConstAccessor.h"
#include "MCTruthClassifier/IMCTruthClassifier.h"

#include <array>

namespace CP {
using ROOT::Math::PtEtaPhiMVector;

bool CalcPartonHistory::getZ(const std::string& str_lep1,
                             const std::string& str_lep2, PtEtaPhiMVector& p1,
                             int& pdgId1, PtEtaPhiMVector& p2, int& pdgId2) {
  // Off-shell / non-resonant Z reconstruction from two same-flavour
  // opposite-sign lepton candidates. Uses MCTruthClassifier decorations
  // (classifierParticleOrigin, classifierParticleType) to restrict to
  // prompt, isolated leptons from a Z decay, rejecting fakes and non-prompt
  // background. Only electrons and muons are considered (not taus); for
  // Z→ττ use getZFromTaus instead. Returns the first pair that passes all
  // conditions (same convention as getW).
  static const SG::ConstAccessor<unsigned int> acc_classifierParticleOrigin(
      "classifierParticleOrigin");
  static const SG::ConstAccessor<unsigned int> acc_classifierParticleType(
      "classifierParticleType");
  std::vector<const xAOD::TruthParticle*> Z_offshell_decay1_candidates;
  std::vector<const xAOD::TruthParticle*> Z_offshell_decay2_candidates;
  // str_lep1/2 are full m_particleMap keys (caller passes m_prefix + "_" + ...)
  bool has_candidates =
      (RetrieveParticleInfo(str_lep1, Z_offshell_decay1_candidates) &&
       RetrieveParticleInfo(str_lep2, Z_offshell_decay2_candidates));
  if (has_candidates) {
    for (const auto* pDecay1 : Z_offshell_decay1_candidates) {
      for (const auto* pDecay2 : Z_offshell_decay2_candidates) {
        // Condition 1: Opposite charge — pdgId product must be negative
        // (e.g. e-=11, e+=−11 → product −121 < 0).
        if ((pDecay1->pdgId() * pDecay2->pdgId()) > 0)
          continue;
        // Condition 2: Same flavour — both leptons must have the same |pdgId|
        // (e.g. both electrons or both muons).
        if (pDecay1->absPdgId() != pDecay2->absPdgId())
          continue;
        // Leptons without MCTruthClassifier decorations cannot be classified
        // and are skipped.
        if (!acc_classifierParticleOrigin.isAvailable(*pDecay1) ||
            !acc_classifierParticleOrigin.isAvailable(*pDecay2) ||
            !acc_classifierParticleType.isAvailable(*pDecay1) ||
            !acc_classifierParticleType.isAvailable(*pDecay2))
          continue;
        const unsigned int o1 = acc_classifierParticleOrigin(*pDecay1);
        const unsigned int o2 = acc_classifierParticleOrigin(*pDecay2);
        const unsigned int t1 = acc_classifierParticleType(*pDecay1);
        const unsigned int t2 = acc_classifierParticleType(*pDecay2);
        // Condition 3: Origin ZBoson from MCTruthClassifier. Ensures both
        // leptons are truth-matched to a Z decay and not to backgrounds such
        // as photon conversions or heavy-flavour semileptonic decays.
        if (!(o1 == MCTruthPartClassifier::ZBoson &&
              o2 == MCTruthPartClassifier::ZBoson))
          continue;
        // Condition 4: Type IsoElectron or IsoMuon from MCTruthClassifier.
        // Selects prompt isolated leptons, rejecting non-isolated or
        // non-prompt contributions.
        if (!((t1 == MCTruthPartClassifier::IsoElectron &&
               t2 == MCTruthPartClassifier::IsoElectron) ||
              (t1 == MCTruthPartClassifier::IsoMuon &&
               t2 == MCTruthPartClassifier::IsoMuon)))
          continue;

        p1 = GetPtEtaPhiMfromTruth(pDecay1);
        pdgId1 = pDecay1->pdgId();
        p2 = GetPtEtaPhiMfromTruth(pDecay2);
        pdgId2 = pDecay2->pdgId();
        return true;
      }
    }
  }
  return false;
}

CalcPartonHistory::ZTauTauDecay CalcPartonHistory::getZFromTaus(
    const std::string& fsr) {
  // taus[0] is the tau- ("l"), taus[1] the tau+ ("lbar"). Their decay
  // products decay1..3 are the charged lepton followed by the two neutrinos.
  // All keys are full m_particleMap keys once the prefix is prepended.
  static constexpr std::array<const char*, 2> tauKeys{{"MC_l_", "MC_lbar_"}};
  static constexpr std::array<std::array<const char*, 3>, 2> tauDecayKeys{
      {{{"MC_l_l_", "MC_l_nubar_", "MC_l_nu_"}},
       {{"MC_lbar_lbar_", "MC_lbar_nu_", "MC_lbar_nubar_"}}}};

  ZTauTauDecay result;
  for (std::size_t i = 0; i < tauKeys.size(); ++i) {
    std::vector<const xAOD::TruthParticle*> tau_candidates;
    if (RetrieveParticleInfo(m_prefix + "_" + tauKeys[i] + fsr,
                             tau_candidates)) {
      result.taus[i] = {GetPtEtaPhiMfromTruth(tau_candidates.at(0)),
                        tau_candidates.at(0)->pdgId(), true};
    }
    // The tau decay products are only used if all three are found.
    std::array<std::vector<const xAOD::TruthParticle*>, 3> decay_candidates;
    bool has_decay_candidates = true;
    for (std::size_t j = 0; j < decay_candidates.size(); ++j) {
      has_decay_candidates =
          has_decay_candidates &&
          RetrieveParticleInfo(m_prefix + "_" + tauDecayKeys[i][j] + fsr,
                               decay_candidates[j]);
    }
    if (has_decay_candidates) {
      for (std::size_t j = 0; j < decay_candidates.size(); ++j) {
        result.tauDecays[i][j] = {
            GetPtEtaPhiMfromTruth(decay_candidates[j].at(0)),
            decay_candidates[j].at(0)->pdgId(), true};
      }
    }
  }
  return result;
}

// For filling Z->ee,mumu
void CalcPartonHistory::setZ(const std::string& fsr, int nZs) {
  PtEtaPhiMVector Z, Z_decay1, Z_decay2;
  int Z_decay1_pdgId, Z_decay2_pdgId;
  m_dec.decorateCustom("MC_Z_IsOnShell", 0);
  // getZ receives full m_particleMap keys; m_dec.decorate* receives bare names.
  bool has_Z =
      getZ(m_prefix + "_" + "MC_l_" + fsr, m_prefix + "_" + "MC_lbar_" + fsr,
           Z_decay1, Z_decay1_pdgId, Z_decay2, Z_decay2_pdgId);
  if (nZs == 1) {
    if (has_Z) {
      Z = Z_decay1 + Z_decay2;
      m_dec.decorateParticle("MC_Z_" + fsr, Z, 23);
      m_dec.decorateParticle("MC_Zdecay1_" + fsr, Z_decay1, Z_decay1_pdgId);
      m_dec.decorateParticle("MC_Zdecay2_" + fsr, Z_decay2, Z_decay2_pdgId);
    }
  } else
    ANA_MSG_ERROR("Reconstruction of multiple Zs is not supported yet!");
}

// For filling Z->tautau
void CalcPartonHistory::setZtautau(const std::string& fsr, int nZs) {
  const ZTauTauDecay decay = getZFromTaus(fsr);
  const bool has_Z = decay.taus[0].found && decay.taus[1].found;
  if (nZs == 1) {
    if (has_Z) {
      m_dec.decorateParticle("MC_Z_" + fsr,
                             decay.taus[0].p4 + decay.taus[1].p4, 23);
      for (std::size_t i = 0; i < decay.taus.size(); ++i) {
        const std::string zDecay = "MC_Zdecay" + std::to_string(i + 1);
        m_dec.decorateParticle(zDecay + "_" + fsr, decay.taus[i].p4,
                               decay.taus[i].pdgId);
        for (std::size_t j = 0; j < decay.tauDecays[i].size(); ++j) {
          const std::string name =
              zDecay + "_decay" + std::to_string(j + 1) + "_" + fsr;
          const ZTauTauProduct& product = decay.tauDecays[i][j];
          // Tau decay products that were not found (e.g. hadronic tau
          // decays) get the sentinel defaults rather than a zero vector.
          if (product.found)
            m_dec.decorateParticle(name, product.p4, product.pdgId);
          else
            m_dec.decorateDefault(name);
        }
      }
    }
  } else
    ANA_MSG_ERROR("Reconstruction of multiple Zs is not supported yet!");
}

void CalcPartonHistory::FillZPartonHistory(const std::string& parent, int nZs,
                                           const std::string& mode) {
  std::string parentstring = parent.empty() ? "" : "_from_" + parent;
  // mapPrefix: full m_particleMap key base (with m_prefix) — used only for
  // ExistsInMap.
  std::string mapPrefix =
      m_prefix + "_" + "MC_" + (parent.empty() ? "Z" : parent + "_Z");
  // decPrefix: bare suffix passed to FillGenericPartonHistory as
  // retrievalstring; FillGenericPartonHistory prepends m_prefix + "_"
  // internally.
  std::string decPrefix = "MC_" + (parent.empty() ? "Z" : parent + "_Z");

  if (mode == "resonant") {
    if (nZs == 1) {
      if (ExistsInMap(mapPrefix + "_beforeFSR")) {
        m_dec.decorateCustom("MC_Z_IsOnShell", 1);
        FillGenericPartonHistory(decPrefix + "_beforeFSR",
                                 "MC_Z" + parentstring + "_beforeFSR", 0);
        FillGenericPartonHistory(decPrefix + "Decay1_beforeFSR",
                                 "MC_Zdecay1" + parentstring + "_beforeFSR", 0);
        FillGenericPartonHistory(decPrefix + "Decay2_beforeFSR",
                                 "MC_Zdecay2" + parentstring + "_beforeFSR", 0);
        FillGenericPartonHistory(decPrefix + "_afterFSR",
                                 "MC_Z" + parentstring + "_afterFSR", 0);
        FillGenericPartonHistory(decPrefix + "Decay1_afterFSR",
                                 "MC_Zdecay1" + parentstring + "_afterFSR", 0);
        FillGenericPartonHistory(decPrefix + "Decay2_afterFSR",
                                 "MC_Zdecay2" + parentstring + "_afterFSR", 0);
      } else {
        setZ("beforeFSR", nZs);
        setZ("afterFSR", nZs);
      }
    } else {
      for (int idx = 0; idx < nZs; idx++) {
        const std::string idxStr = std::to_string(idx + 1);
        m_dec.decorateCustom("MC_Z" + idxStr + "_IsOnShell", 1);
        FillGenericPartonHistory(decPrefix + "_beforeFSR",
                                 "MC_Z" + idxStr + parentstring + "_beforeFSR",
                                 idx);
        FillGenericPartonHistory(
            decPrefix + "Decay1_beforeFSR",
            "MC_Z" + idxStr + "decay1" + parentstring + "_beforeFSR", idx);
        FillGenericPartonHistory(
            decPrefix + "Decay2_beforeFSR",
            "MC_Z" + idxStr + "decay2" + parentstring + "_beforeFSR", idx);
        FillGenericPartonHistory(decPrefix + "_afterFSR",
                                 "MC_Z" + idxStr + parentstring + "_afterFSR",
                                 idx);
        FillGenericPartonHistory(
            decPrefix + "Decay1_afterFSR",
            "MC_Z" + idxStr + "decay1" + parentstring + "_afterFSR", idx);
        FillGenericPartonHistory(
            decPrefix + "Decay2_afterFSR",
            "MC_Z" + idxStr + "decay2" + parentstring + "_afterFSR", idx);
      }
    }
  } else if (mode == "non_resonant") {
    setZ("beforeFSR", nZs);
    setZ("afterFSR", nZs);
  }
}

void CalcPartonHistory::FillZtautauPartonHistory(const std::string& parent,
                                                 int nZs,
                                                 const std::string& mode) {
  std::string parentstring = parent.empty() ? "" : "_from_" + parent;
  // decPrefix: bare suffix for FillGenericPartonHistory retrieval strings.
  std::string decPrefix = "MC_" + (parent.empty() ? "Z" : parent + "_Z");

  FillZPartonHistory(parent, nZs, mode);
  m_dec.decorateCustom("MC_Z_IsOnShell",
                       0);  // default; overwritten to 1 below if resonant
  if (mode == "resonant") {
    if (nZs == 1) {
      m_dec.decorateCustom("MC_Z_IsOnShell", 1);
      FillGenericPartonHistory(
          decPrefix + "Decay1_Decay1_beforeFSR",
          "MC_Zdecay1_decay1" + parentstring + "_beforeFSR", 0);
      FillGenericPartonHistory(
          decPrefix + "Decay1_Decay2_beforeFSR",
          "MC_Zdecay1_decay2" + parentstring + "_beforeFSR", 0);
      FillGenericPartonHistory(
          decPrefix + "Decay1_Decay3_beforeFSR",
          "MC_Zdecay1_decay3" + parentstring + "_beforeFSR", 0);
      FillGenericPartonHistory(decPrefix + "Decay1_Decay1_afterFSR",
                               "MC_Zdecay1_decay1" + parentstring + "_afterFSR",
                               0);
      FillGenericPartonHistory(decPrefix + "Decay1_Decay2_afterFSR",
                               "MC_Zdecay1_decay2" + parentstring + "_afterFSR",
                               0);
      FillGenericPartonHistory(decPrefix + "Decay1_Decay3_afterFSR",
                               "MC_Zdecay1_decay3" + parentstring + "_afterFSR",
                               0);
    } else {
      for (int idx = 0; idx < nZs; idx++) {
        const std::string idxStr = std::to_string(idx + 1);
        m_dec.decorateCustom("MC_Z" + idxStr + "_IsOnShell", 1);
        FillGenericPartonHistory(
            decPrefix + "Decay1_Decay1_beforeFSR",
            "MC_Z" + idxStr + "decay1_decay1" + parentstring + "_beforeFSR",
            idx);
        FillGenericPartonHistory(
            decPrefix + "Decay1_Decay2_beforeFSR",
            "MC_Z" + idxStr + "decay1_decay2" + parentstring + "_beforeFSR",
            idx);
        FillGenericPartonHistory(
            decPrefix + "Decay1_Decay3_beforeFSR",
            "MC_Z" + idxStr + "decay1_decay3" + parentstring + "_beforeFSR",
            idx);
        FillGenericPartonHistory(
            decPrefix + "Decay1_Decay1_afterFSR",
            "MC_Z" + idxStr + "decay1_decay1" + parentstring + "_afterFSR",
            idx);
        FillGenericPartonHistory(
            decPrefix + "Decay1_Decay2_afterFSR",
            "MC_Z" + idxStr + "decay1_decay2" + parentstring + "_afterFSR",
            idx);
        FillGenericPartonHistory(
            decPrefix + "Decay1_Decay3_afterFSR",
            "MC_Z" + idxStr + "decay1_decay3" + parentstring + "_afterFSR",
            idx);
      }
    }
  } else if (mode == "non_resonant") {
    setZtautau("beforeFSR", nZs);
    setZtautau("afterFSR", nZs);
  }
}

}  // namespace CP
