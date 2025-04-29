/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "IDTrackCaloDepositsDecoratorAlg.h"
#include "DerivationFrameworkMuons/Utils.h"
#include "CaloEvent/CaloCellContainer.h"
#include "muonEvent/DepositInCalo.h"
#include "xAODMuon/Muon.h"
#include "xAODTracking/TrackParticle.h"
#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/WriteDecorHandle.h"

namespace DerivationFramework {
IDTrackCaloDepositsDecoratorAlg::IDTrackCaloDepositsDecoratorAlg(const std::string& n, ISvcLocator* p):
    AthReentrantAlgorithm(n, p) {
 }

StatusCode IDTrackCaloDepositsDecoratorAlg::initialize() {
    ATH_CHECK(m_trkDepositInCalo.retrieve());
    
    ATH_CHECK(m_partKey.initialize());   
    for (const std::string& decor : m_trkSelDecors) {
        m_trkSelKeys.emplace_back(m_partKey, decor);       
    }
    ATH_CHECK(m_trkSelKeys.initialize());
    ATH_CHECK(m_depositKey.initialize());
    ATH_CHECK(m_elossKey.initialize());
    ATH_CHECK(m_typeKey.initialize());
    return StatusCode::SUCCESS;
}

StatusCode IDTrackCaloDepositsDecoratorAlg::execute(const EventContext& ctx) const {
    
    SG::ReadHandle<xAOD::IParticleContainer> tracks{m_partKey, ctx};
    if (!tracks.isPresent()) {
        ATH_MSG_FATAL("Failed to retrieve "<< m_partKey.fullKey());
         return StatusCode::FAILURE;
    }
    auto dec_deposit = makeHandle<std::vector<float>>(ctx, m_depositKey);
    auto dec_eloss = makeHandle<std::vector<float>>(ctx, m_elossKey);
    auto dec_type = makeHandle<std::vector<uint16_t>>(ctx, m_typeKey);
    
    using SelDecorator = SG::ReadDecorHandle<xAOD::IParticleContainer, bool>;
    
    std::vector<SelDecorator> selDecors;
    for (const SG::ReadDecorHandleKey<xAOD::IParticleContainer>& key: m_trkSelKeys) {
        selDecors.emplace_back(key, ctx);
    }
    for (const xAOD::IParticle* particle : *tracks) {
        if (particle->pt() < m_ptMin) continue;
        if (!selDecors.empty() && std::find_if(selDecors.begin(), selDecors.end(), 
                                               [particle](const SelDecorator& dec){
                                                   return dec(*particle);
                                               }) == selDecors.end()) continue;
        ATH_MSG_DEBUG("Recomputing calo deposition by hand");

        const xAOD::TrackParticle* track_part = nullptr;
        if (particle->type() == xAOD::Type::ObjectType::TrackParticle) {
            track_part = static_cast<const xAOD::TrackParticle*>(particle);
        } else if (particle->type() == xAOD::Type::ObjectType::Muon) {
            const xAOD::Muon* muon = static_cast<const xAOD::Muon*>(particle);
            track_part = muon->trackParticle(xAOD::Muon::InnerDetectorTrackParticle);
            if (!track_part) {
                ATH_MSG_VERBOSE("The muon does not have an associated ID track");
                track_part = muon->trackParticle(xAOD::Muon::Primary);
            }
        }
        if (!track_part) {
            ATH_MSG_ERROR("Unable to retrieve xAOD::TrackParticle from probe object");
            return StatusCode::FAILURE;
        }

        const CaloCellContainer* caloCellCont = nullptr;
        std::vector<DepositInCalo> deposits = m_trkDepositInCalo->getDeposits(&(track_part->perigeeParameters()), caloCellCont);
        std::vector<float>& dep_val{dec_deposit(*particle)};
        std::vector<float>& eloss_val{dec_eloss(*particle)};
        std::vector<uint16_t>& types{dec_type(*particle)};
        dep_val.reserve(deposits.size());
        eloss_val.reserve(deposits.size());
        types.reserve(deposits.size());
        for (const DepositInCalo& it : deposits) {
            dep_val.push_back(it.energyDeposited());
            eloss_val.push_back(it.muonEnergyLoss());
            types.push_back(it.subCaloId());
        }
    }
    return StatusCode::SUCCESS;
}
}