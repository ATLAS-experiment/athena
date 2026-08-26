/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "GeneratorFilters/xAODMultiLeptonFilter.h"
#include "TruthUtils/HepMCHelpers.h"
#include "AthContainers/ConstAccessor.h"
#include "CLHEP/Vector/LorentzVector.h"
#include <limits>

StatusCode xAODMultiLeptonFilter::filterInitialize()
{
  CHECK(m_truthElectronContKey.initialize());
  CHECK(m_truthMuonContKey.initialize());
  CHECK(m_truthTauContKey.initialize());

  if (!m_countElectrons && !m_countMuons && !m_countTaus) {
    ATH_MSG_ERROR("countElectrons, countMuons and countTaus are all False -- no lepton flavour is being counted, so this filter can never pass. Enable at least one flavour.");
    return StatusCode::FAILURE;
  }

  if (m_NLeptons <= 0) {
    ATH_MSG_ERROR("NLeptons = " << m_NLeptons.value() << " is not valid; it must be >= 1.");
    return StatusCode::FAILURE;
  }

  // Sanity-check for common/per-flavour cut combination for each enabled flavour: error out if both
  // are set (ambiguous -- ask the user to provide only one), warn if neither is set (no cut is applied),
  // and warn if a per-flavour cut was set for a flavour that isn't being counted (it will be ignored).
  bool cutsOk = true;
  auto checkCuts = [this, &cutsOk](const char* label, bool enabled, double flavourPt,
                                    double flavourEta) {
    const bool ptSet  = flavourPt  >= 0.;
    const bool etaSet = flavourEta >= 0.;
    if (!enabled) {
      if (ptSet || etaSet) {
        ATH_MSG_WARNING(label << ": a per-flavour cut was set but count" << label
                        << " is False, so this cut will be ignored.");
      }
      return;
    }
    if (ptSet && m_Ptmin >= 0.) {
      ATH_MSG_ERROR(label << ": both PtCut (" << m_Ptmin.value() << ") and a per-flavour pt cut ("
                    << flavourPt << ") are set; please provide only one of them.");
      cutsOk = false;
    } else if (!ptSet && m_Ptmin < 0.) {
      ATH_MSG_WARNING(label << ": neither PtCut nor a per-flavour pt cut is set; no pt cut will be applied for this flavour.");
    }
    if (etaSet && m_EtaRange >= 0.) {
      ATH_MSG_ERROR(label << ": both EtaCut (" << m_EtaRange.value() << ") and a per-flavour eta cut ("
                    << flavourEta << ") are set; please provide only one of them.");
      cutsOk = false;
    } else if (!etaSet && m_EtaRange < 0.) {
      ATH_MSG_WARNING(label << ": neither EtaCut nor a per-flavour eta cut is set; no eta cut will be applied for this flavour.");
    }
  };

  checkCuts("Electrons", m_countElectrons, m_ElePtCut.value(), m_EleEtaCut.value());
  checkCuts("Muons", m_countMuons, m_MuonPtCut.value(), m_MuonEtaCut.value());
  checkCuts("Taus", m_countTaus, m_TauPtCut.value(), m_TauEtaCut.value());

  if (!cutsOk) return StatusCode::FAILURE;

  return StatusCode::SUCCESS;
}


StatusCode xAODMultiLeptonFilter::filterEvent(const EventContext& ctx) {

  int numLeptons = 0;

  // A per-flavour cut, if set (>= 0), always wins; otherwise fall back to the common cut. If neither
  // is set, apply no cut at all: 0 for a pt lower bound, +infinity for an eta upper bound.
  const double noPtCut  = 0.0;
  const double noEtaCut = std::numeric_limits<double>::infinity();

  const double elePtCut  = (m_ElePtCut  >= 0.) ? m_ElePtCut.value()  : (m_Ptmin >= 0. ? m_Ptmin.value() : noPtCut);
  const double muonPtCut = (m_MuonPtCut >= 0.) ? m_MuonPtCut.value() : (m_Ptmin >= 0. ? m_Ptmin.value() : noPtCut);
  const double tauPtCut  = (m_TauPtCut  >= 0.) ? m_TauPtCut.value()  : (m_Ptmin >= 0. ? m_Ptmin.value() : noPtCut);

  const double eleEtaCut  = (m_EleEtaCut  >= 0.) ? m_EleEtaCut.value()  : (m_EtaRange >= 0. ? m_EtaRange.value() : noEtaCut);
  const double muonEtaCut = (m_MuonEtaCut >= 0.) ? m_MuonEtaCut.value() : (m_EtaRange >= 0. ? m_EtaRange.value() : noEtaCut);
  const double tauEtaCut  = (m_TauEtaCut  >= 0.) ? m_TauEtaCut.value()  : (m_EtaRange >= 0. ? m_EtaRange.value() : noEtaCut);

  if (m_countElectrons) {
    // Retrieve TruthElectrons container
    SG::ReadHandle<xAOD::TruthParticleContainer> xTruthParticleContainerElectron{m_truthElectronContKey, ctx};
    CHECK(xTruthParticleContainerElectron.isValid());
    for (const xAOD::TruthParticle* part : *xTruthParticleContainerElectron) {
      if (MC::isStable(part) && MC::isElectron(part)) { //electron
        if(part->pt() >= elePtCut && part->abseta() <= eleEtaCut) {
          numLeptons += 1;
          if(numLeptons >= m_NLeptons) {
            setFilterPassed(true, ctx);
            return StatusCode::SUCCESS;
          }
        }
      }
    }
  }

  if (m_countMuons) {
    // Retrieve TruthMuons container
    SG::ReadHandle<xAOD::TruthParticleContainer> xTruthParticleContainerMuon{m_truthMuonContKey, ctx};
    CHECK(xTruthParticleContainerMuon.isValid());
    for (const xAOD::TruthParticle* part : *xTruthParticleContainerMuon) {
      if (MC::isStable(part) && MC::isMuon(part)) { //Muon
        if(part->pt() >= muonPtCut && part->abseta() <= muonEtaCut) {
          numLeptons += 1;
          if(numLeptons >= m_NLeptons) {
            setFilterPassed(true, ctx);
            return StatusCode::SUCCESS;
          }
        }
      }
    }
  }

  if (m_countTaus) {
    // Retrieve TruthTaus container. Only hadronic decays are counted here (tauType==0); leptonic tau
    // decays (tauType==1/2) are already counted above via the resulting stable electron/muon.
    static const SG::ConstAccessor<int> tauTypeAcc("tauType");
    static const SG::ConstAccessor<CLHEP::HepLorentzVector> nuVectorAcc("nuVector");
    SG::ReadHandle<xAOD::TruthParticleContainer> xTruthParticleContainerTau{m_truthTauContKey, ctx};
    CHECK(xTruthParticleContainerTau.isValid());
    for (const xAOD::TruthParticle* part : *xTruthParticleContainerTau) {
      if (!MC::isTau(part) || !MC::isPhysical(part)) continue;
      if (tauTypeAcc(*part) != 0) continue;
      const CLHEP::HepLorentzVector& nuVec = nuVectorAcc(*part);
      CLHEP::HepLorentzVector visTau(part->px() - nuVec.px(), part->py() - nuVec.py(),
                                      part->pz() - nuVec.pz(), part->e() - nuVec.e());
      if (visTau.perp() >= tauPtCut && std::abs(visTau.eta()) <= tauEtaCut) {
        numLeptons += 1;
        if(numLeptons >= m_NLeptons) {
          setFilterPassed(true, ctx);
          return StatusCode::SUCCESS;
        }
      }
    }
  }

  setFilterPassed(false, ctx);
  return StatusCode::SUCCESS;

}
