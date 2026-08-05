/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "CaloIsolationDecorAlg.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteDecorHandle.h"

#include "xAODMuonViews/ContainerDecorator.h"
#include <format>
//**********************************************************************
namespace DerivationFramework {
//**********************************************************************

StatusCode CaloIsolationDecorAlg::initialize() {
    ATH_CHECK(m_isoTool.retrieve());

    m_calo_corr.calobitset.set(static_cast<unsigned int>(xAOD::Iso::coreCone));
    m_calo_corr.calobitset.set(static_cast<unsigned int>(xAOD::Iso::pileupCorrection));
    // isolation types to run. The ptcones each also imply the respective ptvarcone.

    ATH_CHECK(m_trk_key.initialize());
    for (const std::string& decor : m_trkSel_Decors) m_trkSel_keys.emplace_back(m_trk_key, decor);
    ATH_CHECK(m_trkSel_keys.initialize());
    m_topocone20_key = std::format("topoetcone20{:}{:}" ,m_customName.empty() ? "" : "_", m_customName.value());
    m_topocone30_key = std::format("topoetcone30{:}{:}" ,m_customName.empty() ? "" : "_", m_customName.value());
    m_topocone40_key = std::format("topoetcone40{:}{:}" ,m_customName.empty() ? "" : "_", m_customName.value());
    m_corr_key       = std::format("etcore_correction{:}{:}", m_customName.empty() ? "" : "_", m_customName.value());
    ATH_CHECK(m_topocone20_key.initialize());
    ATH_CHECK(m_topocone30_key.initialize());
    ATH_CHECK(m_topocone40_key.initialize());
    ATH_CHECK(m_corr_key.initialize());
    ATH_MSG_DEBUG("Decorate " << m_trk_key.fullKey() << " using '" << m_customName << "' as suffix.");
    return StatusCode::SUCCESS;
}

//**********************************************************************

StatusCode CaloIsolationDecorAlg::execute(const EventContext& ctx) const {
    const xAOD::TrackParticleContainer* tracks{nullptr};
    ATH_CHECK(SG::get(tracks, m_trk_key, ctx));
    

    using FloatDecor = xAOD::ContainerDecorator<xAOD::TrackParticleContainer, float>;
    using SelDecorator = SG::ReadDecorHandle<xAOD::TrackParticleContainer, std::uint8_t>;
    
    std::vector<SelDecorator> selDecors;
    for (const SG::ReadDecorHandleKey<xAOD::TrackParticleContainer>& key : m_trkSel_keys) {
        selDecors.emplace_back(key, ctx);
    }

    FloatDecor topocone40_dec{m_topocone40_key, ctx, -Gaudi::Units::GeV};
    FloatDecor topocone30_dec{m_topocone30_key, ctx, -Gaudi::Units::GeV};
    FloatDecor topocone20_dec{m_topocone20_key, ctx, -Gaudi::Units::GeV};
    FloatDecor corr_dec{m_corr_key, ctx};

    for (const xAOD::TrackParticle* trk : *tracks) {
        if (trk->pt() < m_pt_min) continue;
        if (!selDecors.empty() && std::find_if(selDecors.begin(), selDecors.end(), [trk](const SelDecorator& dec){
                return dec(*trk);
        }) == selDecors.end()) continue;
        ATH_MSG_DEBUG("Recomputing isolation by hand");
        xAOD::CaloIsolation resultCalo;
        if (!m_isoTool->caloTopoClusterIsolation(resultCalo, *trk, m_calo_isos, m_calo_corr)) {
            ATH_MSG_ERROR("Failed to compute calorimeter isolation");
            return StatusCode::FAILURE;
        }
        topocone40_dec(*trk) = resultCalo.etcones[0];
        topocone30_dec(*trk) = resultCalo.etcones[1];
        topocone20_dec(*trk) = resultCalo.etcones[2];
        corr_dec(*trk) = resultCalo.coreCorrections[xAOD::Iso::coreCone][xAOD::Iso::coreEnergy];
    }
    return StatusCode::SUCCESS;
}
}
//**********************************************************************
