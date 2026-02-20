/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
// Runs on muons without a truth particle link.
// Finds the nearest stable truth particle and adds its info to the muon.
#include "MuonTruthClassifierFallback.h"

#include "FourMomUtils/xAODP4Helpers.h"
#include "TruthUtils/HepMCHelpers.h"
#include "xAODMuon/MuonContainer.h"

namespace {
    static const SG::Decorator<float> decorator_dR("MCTFallback_dR");
    static const SG::Decorator<int> decorator_type("MCTFallback_truthType");
    static const SG::Decorator<int> decorator_origin("MCTFallback_truthOrigin");

    static const SG::Decorator<float> decorator_pu_dR("MCTFallbackPU_dR");
    static const SG::Decorator<int> decorator_pu_type("MCTFallbackPU_truthType");
    static const SG::Decorator<int> decorator_pu_origin("MCTFallbackPU_truthOrigin");
  // TODO Check if these last two should be ReadDecorHandles instead
    static const SG::Accessor<int> acc_tT("truthType");
    static const SG::Accessor<int> acc_tO("truthOrigin");

}  // namespace

// Athena initialize
StatusCode DerivationFramework::MuonTruthClassifierFallback::initialize() {
    ATH_MSG_VERBOSE("initialize() ...");
    ATH_CHECK(m_mcTruthClassifier.retrieve());

    ATH_CHECK(m_containerKey.initialize());
    ATH_CHECK(m_truthSGKey.initialize());
    ATH_CHECK(m_truthPileupSGKey.initialize());
    ATH_CHECK(m_truthMuonSGKey.initialize());

    // FIXME These WriteDecorHandles are not being used. The
    // Decorators above are (incorrectly) used instead.
    ATH_CHECK(m_Truth_dR_Key.initialize());
    ATH_CHECK(m_Truth_type_Key.initialize());
    ATH_CHECK(m_Truth_origin_Key.initialize());

    ATH_CHECK(m_Truth_PU_dR_Key.initialize());
    ATH_CHECK(m_Truth_PU_type_Key.initialize());
    ATH_CHECK(m_Truth_PU_origin_Key.initialize());

    return StatusCode::SUCCESS;
}

StatusCode DerivationFramework::MuonTruthClassifierFallback::addBranches(const EventContext& ctx) const {
    // Retrieve main particle collection

    SG::ReadHandle<xAOD::IParticleContainer> parts{m_containerKey, ctx};
    if (!parts.isValid()) {
        ATH_MSG_ERROR("No Muon collection with name " << m_containerKey.fullKey() << " found in StoreGate!");
        return StatusCode::FAILURE;
    }
    SG::ReadHandle<xAOD::TruthEventContainer> tec{m_truthSGKey, ctx};
    if (!tec.isValid()) {
        ATH_MSG_ERROR("No truth collection with name " << m_truthSGKey.fullKey() << " found in StoreGate!");
        return StatusCode::FAILURE;
    }
    SG::ReadHandle<xAOD::TruthPileupEventContainer> tpec{m_truthPileupSGKey, ctx};
    if (!tpec.isValid()) {
        ATH_MSG_DEBUG("No truth pileup collection with name " << m_truthPileupSGKey.fullKey() << " found in StoreGate. Pile-up information will not be filled");
    }

    SG::ReadHandle<xAOD::TruthParticleContainer> truthMuons{m_truthMuonSGKey, ctx};
    if (!truthMuons.isValid()) {
        ATH_MSG_ERROR("No truth muon collection with name " << m_truthMuonSGKey.fullKey() << " found in StoreGate!");
        return StatusCode::FAILURE;
    }

    // Set up the decorator

    for (const xAOD::IParticle* part : *parts) {
        const xAOD::TruthParticle* closest = nullptr;
        float minDR{FLT_MAX};
        for (const xAOD::TruthParticle* muTruth : *truthMuons) {
            const float dR = xAOD::P4Helpers::deltaR2(muTruth, part);
            if (dR > minDR) continue;
            closest = muTruth;
            minDR = dR;
        }

        for (const xAOD::TruthEvent* event : *tec) {
            for (size_t parti = 0; parti < event->nTruthParticles(); parti++) {
                const xAOD::TruthParticle* tpart = event->truthParticle(parti);
                if (!tpart || !MC::isStable(tpart) || HepMC::is_simulation_particle(tpart) || !tpart->charge() || tpart->isMuon() ||
                    tpart->pt() < m_minPt)
                    continue;
                const float dR = xAOD::P4Helpers::deltaR2(tpart, part);
                if (dR > minDR) continue;
                closest = tpart;
                minDR = dR;
            }
        }

        decorator_dR(*part) = closest ? std::sqrt(minDR) : -1;

        int newType{-1}, newOrigin{-1};
        if (closest && closest->isMuon()) {
            newType = acc_tT(*closest);
            newOrigin = acc_tO(*closest);
        } else if (closest) {
            auto res = m_mcTruthClassifier->particleTruthClassifier(closest);
            newType = res.first;
            newOrigin = res.second;
        }
        decorator_type(*part) = newType;
        decorator_origin(*part) = newOrigin;

        decorator_pu_dR(*part) = -1;
        decorator_pu_type(*part) = -1;
        decorator_pu_origin(*part) = -1;

        minDR = FLT_MAX;

        if (tpec.isValid()) { 
            const xAOD::TruthParticle* closestPileup = nullptr;
            for (auto event : *tpec) {
                for (size_t parti = 0; parti < event->nTruthParticles(); parti++) {
                    const xAOD::TruthParticle* tpart = event->truthParticle(parti);
                    if (!tpart || !MC::isStable(tpart) || HepMC::is_simulation_particle(tpart) || !tpart->charge() || tpart->isMuon() ||
                        tpart->pt() < m_minPt)
                        continue;
                    const float dR = xAOD::P4Helpers::deltaR2(tpart, part);
                    if (dR > minDR) continue;
                    closestPileup = tpart;
                    minDR = dR;
                }
            }
            decorator_pu_dR(*part) = (closestPileup ? std::sqrt(minDR) : -1);
            int newPileupType{-1}, newPileupOrigin{-1};
            if (closestPileup) {
                auto res = m_mcTruthClassifier->particleTruthClassifier(closestPileup);
                newPileupType = res.first;
                newPileupOrigin = res.second;
            }
            decorator_pu_type(*part) = newPileupType;
            decorator_pu_origin(*part) = newPileupOrigin;
        }
    }

    return StatusCode::SUCCESS;
}
