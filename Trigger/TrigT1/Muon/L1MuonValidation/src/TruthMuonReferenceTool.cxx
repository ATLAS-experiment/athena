/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TruthMuonReferenceTool.h"

#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenParticle.h"
#include "L1MuonValidation/TruthMuonSelection.h"
#include "StoreGate/ReadHandle.h"

#include <cmath>

namespace L1Muon {

TruthMuonReferenceTool::TruthMuonReferenceTool(const std::string& type,
                                               const std::string& name,
                                               const IInterface* parent)
    : AthAlgTool(type, name, parent) {
  declareInterface<ITruthMuonReferenceTool>(this);
}

StatusCode TruthMuonReferenceTool::initialize() {
  if (m_minTruthPt.value() < 0.0 || m_minTruthAbsEta.value() < 0.0 ||
      m_maxTruthAbsEta.value() <= m_minTruthAbsEta.value() ||
      m_maxAbsTruthBarcode.value() < 0) {
    ATH_MSG_ERROR("Invalid truth-muon reference selection configuration");
    return StatusCode::FAILURE;
  }
  ATH_CHECK(m_truthEventKey.initialize());
  return StatusCode::SUCCESS;
}

StatusCode TruthMuonReferenceTool::selectedMuons(
    const EventContext& ctx,
    std::vector<TruthMuonReference>& references) const {
  references.clear();

  SG::ReadHandle<McEventCollection> truthEvents{m_truthEventKey, ctx};
  if (!truthEvents.isValid()) {
    ATH_MSG_ERROR("Failed to retrieve " << m_truthEventKey.fullKey());
    return StatusCode::FAILURE;
  }

  TruthMuonSelectionConfig selection{};
  selection.minPtMeV = m_minTruthPt.value();
  selection.minAbsEta = m_minTruthAbsEta.value();
  selection.maxAbsEta = m_maxTruthAbsEta.value();
  selection.requiredStatus = m_requiredTruthStatus.value();
  selection.maxAbsBarcode = m_maxAbsTruthBarcode.value();

  const auto processParticle = [&](const auto& particle) {
    if (!particle) {
      return;
    }

    const auto& momentum = particle->momentum();
    const int pdgId = particle->pdg_id();
    const int barcode = HepMC::barcode(particle);
    const double ptMeV = momentum.perp();
    const double eta = momentum.eta();
    const double phi = momentum.phi();
    if (!acceptTruthMuon(pdgId, particle->status(), barcode, ptMeV, eta, phi,
                         selection)) {
      return;
    }

    references.push_back(TruthMuonReference{
        pdgId, barcode, ptMeV, eta, phi, pdgId == 13 ? -1 : 1});
  };

  for (const HepMC::GenEvent* event : *truthEvents) {
    if (event == nullptr) {
      continue;
    }
#if __has_include("HepMC3/GenEvent.h")
    for (const auto& particle : event->particles()) {
      processParticle(particle);
    }
#else
    for (auto particle = event->particles_begin();
         particle != event->particles_end(); ++particle) {
      processParticle(*particle);
    }
#endif
  }
  return StatusCode::SUCCESS;
}

std::string TruthMuonReferenceTool::inputKey() const {
  return m_truthEventKey.key();
}

}  // namespace L1Muon
