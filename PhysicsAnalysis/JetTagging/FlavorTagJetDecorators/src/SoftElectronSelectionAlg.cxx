/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "SoftElectronSelectionAlg.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/WriteDecorHandle.h"

#include "xAODEgamma/Electron.h"
#include "xAODEgamma/EgammaEnums.h"
#include "xAODTracking/TrackParticle.h"

#include "TLorentzVector.h"

namespace FlavorTagJetDecorators {

  SoftElectronSelectionAlg::SoftElectronSelectionAlg(
    const std::string& name, ISvcLocator* loc)
    : AthReentrantAlgorithm(name, loc) {}

  StatusCode SoftElectronSelectionAlg::initialize() {
    ATH_CHECK(m_jetContainerKey.initialize());
    ATH_CHECK(m_electronContainerKey.initialize());
    ATH_CHECK(m_ghostElectronsKey.initialize());
    ATH_CHECK(m_selectedElectronsKey.initialize());
    ATH_CHECK(m_energyOverPKey.initialize());
    ATH_CHECK(m_etKey.initialize());
    ATH_CHECK(m_isoOverPtKey.initialize());
    ATH_CHECK(m_dpopKey.initialize());
    return StatusCode::SUCCESS;
  }

  StatusCode SoftElectronSelectionAlg::execute(
    const EventContext& ctx) const
  {
    SG::ReadHandle<xAOD::JetContainer> jets(m_jetContainerKey, ctx);
    ATH_CHECK(jets.isValid());

    SG::ReadHandle<xAOD::ElectronContainer> electrons(
      m_electronContainerKey, ctx);
    ATH_CHECK(electrons.isValid());

    SG::WriteDecorHandle<xAOD::JetContainer, std::vector<ElementLink<xAOD::IParticleContainer>>> selectedOut(
      m_selectedElectronsKey, ctx);

    SG::ReadDecorHandle<xAOD::JetContainer, std::vector<ElementLink<xAOD::IParticleContainer>>> ghostElectrons(
      m_ghostElectronsKey, ctx);

    SG::ReadDecorHandle<xAOD::ElectronContainer, float> energyOverP(
      m_energyOverPKey, ctx);
    SG::ReadDecorHandle<xAOD::ElectronContainer, float> et(
      m_etKey, ctx);
    SG::ReadDecorHandle<xAOD::ElectronContainer, float> isoOverPt(
      m_isoOverPtKey, ctx);
    SG::ReadDecorHandle<xAOD::ElectronContainer, float> dpop(
      m_dpopKey, ctx);

    for (const xAOD::Jet* jet : *jets) {
      std::vector<ElementLink<xAOD::IParticleContainer>> selected;

      for (const auto& link : ghostElectrons(*jet)) {
        const auto* el = dynamic_cast<const xAOD::Electron*>(*link);
        if (!el) continue;

        if (passedCuts(*jet, *el,
                       energyOverP(*el), et(*el),
                       isoOverPt(*el), dpop(*el))) {
          selected.push_back(
            ElementLink<xAOD::IParticleContainer>(
              *electrons, el->index()));
        }
      }

      selectedOut(*jet) = selected;
    }

    return StatusCode::SUCCESS;
  }

  bool SoftElectronSelectionAlg::passedCuts(
    const xAOD::Jet& jet,
    const xAOD::Electron& el,
    float energyOverP,
    float et,
    float isoOverPt,
    float dpop) const
  {
    TLorentzVector jet4;
    jet4.SetPtEtaPhiE(jet.pt(), jet.eta(), jet.phi(), jet.e());
    TLorentzVector el4;
    el4.SetPtEtaPhiE(el.pt(), el.eta(), el.phi(), el.e());

    if (m_maxDeltaR > 0. && jet4.DeltaR(el4) > m_maxDeltaR)
      return false;

    if (std::abs(el.eta()) > m_absEtaMaximum)
      return false;
    if (el.pt() <= m_ptMinimum)
      return false;
    if (el.pt() >= m_ptMaximum)
      return false;

    const auto* track = el.trackParticle();
    if (!track) return false;

    if (std::isfinite(m_d0Maximum) &&
        std::abs(track->d0()) >= m_d0Maximum)
      return false;

    // E/p and ET from pre-computed decorations (not caloCluster)
    if (std::abs(energyOverP) > m_eopMaximum)
      return false;
    if (std::abs(et) > m_etMaximum)
      return false;

    if (std::abs(isoOverPt) > m_isoptMaximum)
      return false;

    float ptrel = el4.Vect().Perp(jet4.Vect());
    if (std::abs(ptrel) > m_ptrelMaximum)
      return false;

    float rhad1 = el.showerShapeValue(xAOD::EgammaParameters::Rhad1);
    float wstot = el.showerShapeValue(xAOD::EgammaParameters::wtots1);
    float rphi  = el.showerShapeValue(xAOD::EgammaParameters::Rphi);
    float reta  = el.showerShapeValue(xAOD::EgammaParameters::Reta);
    float deta1 = el.trackCaloMatchValue(
      xAOD::EgammaParameters::deltaEta1);

    if (std::abs(rhad1) > m_rhad1Maximum)
      return false;
    if (std::abs(wstot) > m_wstotMaximum)
      return false;
    if (std::abs(rphi) > m_rphiMaximum)
      return false;
    if (std::abs(reta) > m_retaMaximum)
      return false;
    if (std::abs(deta1) > m_deta1Maximum)
      return false;
    if (std::abs(dpop) > m_dpopMaximum)
      return false;

    return true;
  }

}  // namespace FlavorTagJetDecorators
