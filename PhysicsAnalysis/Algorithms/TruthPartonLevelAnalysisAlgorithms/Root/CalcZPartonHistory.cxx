/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>
/// @author Steffen Korn <steffen.korn@cern.ch>

#include "PartonHistory/CalcPartonHistory.h"
#include "PartonHistory/PartonHistoryUtils.h"
#include "VectorHelpers/DecoratorHelpers.h"
#include "VectorHelpers/LorentzHelper.h"

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
  // Z→ττ use getZFromTaus instead.
  static const SG::Accessor<unsigned int> acc_classifierParticleOrigin(
      "classifierParticleOrigin");
  static const SG::Accessor<unsigned int> acc_classifierParticleType(
      "classifierParticleType");
  std::vector<const xAOD::TruthParticle*> Z_offshell_decay1_candidates;
  std::vector<const xAOD::TruthParticle*> Z_offshell_decay2_candidates;
  // str_lep1/2 are full m_particleMap keys (caller passes m_prefix + "_" + ...)
  bool has_candidates =
      (RetrieveParticleInfo(str_lep1, Z_offshell_decay1_candidates) &&
       RetrieveParticleInfo(str_lep2, Z_offshell_decay2_candidates));
  if (has_candidates) {
    const xAOD::TruthParticle* bp1 = nullptr;
    const xAOD::TruthParticle* bp2 = nullptr;
    for (const auto* p1 : Z_offshell_decay1_candidates) {
      for (const auto* p2 : Z_offshell_decay2_candidates) {
        unsigned int o1 = acc_classifierParticleOrigin(*p1);
        unsigned int o2 = acc_classifierParticleOrigin(*p2);
        unsigned int t1 = acc_classifierParticleType(*p1);
        unsigned int t2 = acc_classifierParticleType(*p2);
        // Condition 1: Opposite charge — pdgId product must be negative
        // (e.g. e-=11, e+=−11 → product −121 < 0).
        if ((p1->pdgId() * p2->pdgId()) > 0)
          continue;
        // Condition 2: Same flavour — both leptons must have the same |pdgId|
        // (e.g. both electrons or both muons).
        if (p1->absPdgId() != p2->absPdgId())
          continue;
        // Condition 3: Origin == 13 (ZBoson) from MCTruthClassifier
        // (see TruthUtils/TruthClasses.h). Ensures both leptons are
        // truth-matched to a Z decay and not to backgrounds such as photon
        // conversions or heavy-flavour semileptonic decays.
        if (!(o1 == 13 && o2 == 13))
          continue;
        // Condition 4: Type == 2 (IsoElectron) or 4 (IsoMuon) from
        // MCTruthClassifier. Selects prompt isolated leptons, rejecting
        // non-isolated or non-prompt contributions.
        if (!((t1 == 2 && t2 == 2) || (t1 == 4 && t2 == 4)))
          continue;
        bp1 = p1;
        bp2 = p2;
      }
    }
    if (bp1 && bp2) {
      p1 = GetPtEtaPhiMfromTruth(bp1);
      pdgId1 = bp1->pdgId();
      p2 = GetPtEtaPhiMfromTruth(bp2);
      pdgId2 = bp2->pdgId();
      return true;
    }
  }
  return false;
}

bool CalcPartonHistory::getZFromTaus(
    const std::string& fsr, PtEtaPhiMVector& Zdecay1, int& Zdecay1_pdgId,
    PtEtaPhiMVector& Zdecay2, int& Zdecay2_pdgId,
    PtEtaPhiMVector& Zdecay1_decay1, int& Zdecay1_decay1_pdgId,
    PtEtaPhiMVector& Zdecay1_decay2, int& Zdecay1_decay2_pdgId,
    PtEtaPhiMVector& Zdecay1_decay3, int& Zdecay1_decay3_pdgId,
    PtEtaPhiMVector& Zdecay2_decay1, int& Zdecay2_decay1_pdgId,
    PtEtaPhiMVector& Zdecay2_decay2, int& Zdecay2_decay2_pdgId,
    PtEtaPhiMVector& Zdecay2_decay3, int& Zdecay2_decay3_pdgId) {

  std::vector<const xAOD::TruthParticle*> Z_offshell_l_candidates;  // tau
  std::vector<const xAOD::TruthParticle*>
      Z_offshell_l_l_candidates;  // l from tau
  std::vector<const xAOD::TruthParticle*>
      Z_offshell_l_nubar_candidates;  // anti-nu from tau
  std::vector<const xAOD::TruthParticle*>
      Z_offshell_l_nu_candidates;  // nu from tau
  std::vector<const xAOD::TruthParticle*> Z_offshell_lbar_candidates;  // taubar
  std::vector<const xAOD::TruthParticle*>
      Z_offshell_lbar_lbar_candidates;  // lbar from taubar
  std::vector<const xAOD::TruthParticle*>
      Z_offshell_lbar_nu_candidates;  // nu from taubar
  std::vector<const xAOD::TruthParticle*>
      Z_offshell_lbar_nubar_candidates;  // anti-nu from taubar

  // All RetrieveParticleInfo calls use full m_particleMap keys (with prefix).
  bool has_taum_candidates = RetrieveParticleInfo(
      m_prefix + "_" + "MC_l_" + fsr, Z_offshell_l_candidates);
  bool has_taum_decay_candidates =
      (RetrieveParticleInfo(m_prefix + "_" + "MC_l_l_" + fsr,
                            Z_offshell_l_l_candidates) &&
       RetrieveParticleInfo(m_prefix + "_" + "MC_l_nubar_" + fsr,
                            Z_offshell_l_nubar_candidates) &&
       RetrieveParticleInfo(m_prefix + "_" + "MC_l_nu_" + fsr,
                            Z_offshell_l_nu_candidates));
  bool has_taup_candidates = RetrieveParticleInfo(
      m_prefix + "_" + "MC_lbar_" + fsr, Z_offshell_lbar_candidates);
  bool has_taup_decay_candidates =
      (RetrieveParticleInfo(m_prefix + "_" + "MC_lbar_lbar_" + fsr,
                            Z_offshell_lbar_lbar_candidates) &&
       RetrieveParticleInfo(m_prefix + "_" + "MC_lbar_nu_" + fsr,
                            Z_offshell_lbar_nu_candidates) &&
       RetrieveParticleInfo(m_prefix + "_" + "MC_lbar_nubar_" + fsr,
                            Z_offshell_lbar_nubar_candidates));

  if (has_taum_candidates) {
    Zdecay1 = GetPtEtaPhiMfromTruth(Z_offshell_l_candidates.at(0));
    Zdecay1_pdgId = Z_offshell_l_candidates.at(0)->pdgId();
  }
  if (has_taum_decay_candidates) {
    Zdecay1_decay1 = GetPtEtaPhiMfromTruth(Z_offshell_l_l_candidates.at(0));
    Zdecay1_decay1_pdgId = Z_offshell_l_l_candidates.at(0)->pdgId();
    Zdecay1_decay2 = GetPtEtaPhiMfromTruth(Z_offshell_l_nubar_candidates.at(0));
    Zdecay1_decay2_pdgId = Z_offshell_l_nubar_candidates.at(0)->pdgId();
    Zdecay1_decay3 = GetPtEtaPhiMfromTruth(Z_offshell_l_nu_candidates.at(0));
    Zdecay1_decay3_pdgId = Z_offshell_l_nu_candidates.at(0)->pdgId();
  }
  if (has_taup_candidates) {
    Zdecay2 = GetPtEtaPhiMfromTruth(Z_offshell_lbar_candidates.at(0));
    Zdecay2_pdgId = Z_offshell_lbar_candidates.at(0)->pdgId();
  }
  if (has_taup_decay_candidates) {
    Zdecay2_decay1 =
        GetPtEtaPhiMfromTruth(Z_offshell_lbar_lbar_candidates.at(0));
    Zdecay2_decay1_pdgId = Z_offshell_lbar_lbar_candidates.at(0)->pdgId();
    Zdecay2_decay2 = GetPtEtaPhiMfromTruth(Z_offshell_lbar_nu_candidates.at(0));
    Zdecay2_decay2_pdgId = Z_offshell_lbar_nu_candidates.at(0)->pdgId();
    Zdecay2_decay3 =
        GetPtEtaPhiMfromTruth(Z_offshell_lbar_nubar_candidates.at(0));
    Zdecay2_decay3_pdgId = Z_offshell_lbar_nubar_candidates.at(0)->pdgId();
  }
  return has_taum_candidates && has_taup_candidates;
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
  PtEtaPhiMVector Z, Zdecay1, Zdecay2;
  PtEtaPhiMVector Zdecay1_decay1, Zdecay1_decay2, Zdecay1_decay3;
  PtEtaPhiMVector Zdecay2_decay1, Zdecay2_decay2, Zdecay2_decay3;
  int Zdecay1_pdgId = 0, Zdecay2_pdgId = 0;
  int Zdecay1_decay1_pdgId = 0, Zdecay1_decay2_pdgId = 0,
      Zdecay1_decay3_pdgId = 0;
  int Zdecay2_decay1_pdgId = 0, Zdecay2_decay2_pdgId = 0,
      Zdecay2_decay3_pdgId = 0;

  bool has_Z =
      getZFromTaus(fsr, Zdecay1, Zdecay1_pdgId, Zdecay2, Zdecay2_pdgId,
                   Zdecay1_decay1, Zdecay1_decay1_pdgId, Zdecay1_decay2,
                   Zdecay1_decay2_pdgId, Zdecay1_decay3, Zdecay1_decay3_pdgId,
                   Zdecay2_decay1, Zdecay2_decay1_pdgId, Zdecay2_decay2,
                   Zdecay2_decay2_pdgId, Zdecay2_decay3, Zdecay2_decay3_pdgId);
  if (nZs == 1) {
    if (has_Z) {
      Z = Zdecay1 + Zdecay2;
      m_dec.decorateParticle("MC_Z_" + fsr, Z, 23);
      m_dec.decorateParticle("MC_Zdecay1_" + fsr, Zdecay1, Zdecay1_pdgId);
      m_dec.decorateParticle("MC_Zdecay2_" + fsr, Zdecay2, Zdecay2_pdgId);
      m_dec.decorateParticle("MC_Zdecay1_decay1_" + fsr, Zdecay1_decay1,
                             Zdecay1_decay1_pdgId);
      m_dec.decorateParticle("MC_Zdecay2_decay1_" + fsr, Zdecay2_decay1,
                             Zdecay2_decay1_pdgId);
      m_dec.decorateParticle("MC_Zdecay1_decay2_" + fsr, Zdecay1_decay2,
                             Zdecay1_decay2_pdgId);
      m_dec.decorateParticle("MC_Zdecay2_decay2_" + fsr, Zdecay2_decay2,
                             Zdecay2_decay2_pdgId);
      m_dec.decorateParticle("MC_Zdecay1_decay3_" + fsr, Zdecay1_decay3,
                             Zdecay1_decay3_pdgId);
      m_dec.decorateParticle("MC_Zdecay2_decay3_" + fsr, Zdecay2_decay3,
                             Zdecay2_decay3_pdgId);
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
  // mapPrefix: full key for ExistsInMap (used indirectly via
  // FillZPartonHistory).
  std::string mapPrefix =
      m_prefix + "_" + "MC_" + (parent.empty() ? "Z" : parent + "_Z");
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
