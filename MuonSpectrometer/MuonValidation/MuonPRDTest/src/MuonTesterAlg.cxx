/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonTesterAlg.h"

#include "MuonTesterTree/TrackDetailBranches.h"
#include "MuonTesterTree/LinkerBranch.h"

namespace MuonVal {
    StatusCode MuonTesterAlg::initialize(){
        ATH_CHECK(m_muonKey.initialize());
        m_muonBr = std::make_shared<IParticleFourMomBranch>(m_tree, "muons");
        m_muonBr->addVariable<float>("MSFieldIntegral",
                                     "spectrometerFieldIntegral");
        m_muonBr->addVariable<float>("SCS", "scatteringCurvatureSignificance");
        m_muonBr->addVariable<float>("SNS", "scatteringNeighbourSignificance");
        m_muonBr->addVariable<float>("PtImbalSig",
                                     "momentumBalanceSignificance");

        m_muonBr->addVariable<float>(FLT_MAX, "segmentDeltaEta");
        m_muonBr->addVariable<float>(FLT_MAX, "segmentDeltaPhi");
        m_muonBr->addVariable<float>(FLT_MAX, "segmentChi2OverDoF");
        
        auto msTracks = std::make_unique<IParticleFourMomBranch>(m_tree, "MsTrks");

        msTracks->addVariable(std::make_unique<TrackChi2Branch>(*msTracks));
        msTracks->addVariable(std::make_unique<EnergylossBranch>(*msTracks));
        msTracks->addVariable(std::make_unique<ScatteringBranch>(*msTracks));
        
        m_muonBr->addVariable(std::make_unique<LinkerBranch>(*m_muonBr, std::move(msTracks), 
                              [](const xAOD::IParticle*p) {
                                return static_cast<const xAOD::Muon*>(p)->trackParticle(xAOD::Muon::TrackParticleType::MuonSpectrometerTrackParticle);
                              }));

        m_tree.addBranch(m_muonBr);
        ATH_CHECK(m_tree.init(this));
        return StatusCode::SUCCESS;
    }
    StatusCode MuonTesterAlg::finalize(){
        ATH_CHECK(m_tree.write());
        return StatusCode::SUCCESS;
    }
    StatusCode MuonTesterAlg::execute(const EventContext& ctx){
        const xAOD::MuonContainer* muons{nullptr};
        ATH_CHECK(SG::get(muons, m_muonKey, ctx));
        for (const xAOD::Muon* muon : *muons) {
            m_muonBr->push_back(muon);
        }
         if (!m_tree.fill(ctx)) {
            return StatusCode::FAILURE;
         }
        return StatusCode::SUCCESS;
    }
   
}