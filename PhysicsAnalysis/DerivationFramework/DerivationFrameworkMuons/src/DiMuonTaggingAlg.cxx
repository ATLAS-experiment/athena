/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "DiMuonTaggingAlg.h"
#include "DerivationFrameworkMuons/Utils.h"
#include "AthenaKernel/errorcheck.h"
#include "FourMomUtils/xAODP4Helpers.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/TrackingPrimitives.h"
#include "TruthUtils/HepMCHelpers.h"
namespace {
    using MuonPassDecor = SG::WriteDecorHandle<xAOD::MuonContainer, bool>;
    constexpr double MeVtoGeV = 1./ Gaudi::Units::GeV;

}
namespace DerivationFramework {
    DiMuonTaggingAlg::DiMuonTaggingAlg(const std::string& name, ISvcLocator* pSvcLocator) : 
        AthReentrantAlgorithm(name, pSvcLocator) {}

    // Athena initialize and finalize
    StatusCode DiMuonTaggingAlg::initialize() {
        ATH_MSG_VERBOSE("initialize() ...");

        // trigger decision tool, needed when there is trigger requirement
        if (!m_applyTrig) {
            m_orTrigs.clear();
            m_andTrigs.clear();
        }
        const bool has_trigs = m_orTrigs.size() || m_andTrigs.size();
        ATH_CHECK(m_matchingTool.retrieve(EnableTool{has_trigs}));
        
        ATH_CHECK(m_muonSGKey.initialize());
        ATH_CHECK(m_trackSGKey.initialize(m_useTrackProbe));
        ATH_CHECK(m_truthSGKey.initialize(m_isMC && !m_truthSGKey.empty()));
        ATH_CHECK(m_muonSelTool.retrieve(EnableTool{m_applyQualityMu1 || m_applyQualityMu2}));
        m_muonKeepKey = "pass" + m_br_prefix.value();
        m_trkKeepKey  = "pass" + m_br_prefix.value();
        m_skimmingKey = "DIMU_pass" + m_br_prefix;
        ATH_CHECK(m_muonKeepKey.initialize());
        ATH_CHECK(m_trkKeepKey.initialize(m_useTrackProbe));
        ATH_CHECK(m_skimmingKey.initialize());

        m_invariantMassLow2 = m_invariantMassLow * std::abs(m_invariantMassLow);
        m_invariantMassHigh2 = m_invariantMassHigh * std::abs(m_invariantMassHigh);
        m_thinningConeSize2 = m_thinningConeSize * std::abs(m_thinningConeSize);

        return StatusCode::SUCCESS;
    }

    StatusCode DiMuonTaggingAlg::execute(const EventContext& ctx) const {
        SG::WriteHandle<int> keepEventHandle{m_skimmingKey, ctx};
        ATH_CHECK(keepEventHandle.record(std::make_unique<int>(0)));
        int& keepEvent = *keepEventHandle;

        /// muon selection
        SG::ReadHandle<xAOD::MuonContainer> muons{m_muonSGKey, ctx};
        if (!muons.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve " << m_muonSGKey.fullKey());
            return StatusCode::FAILURE;
        }
        MuonPassDecor muo_decor{makeHandle<bool>(ctx, m_muonKeepKey)};
        ATH_MSG_VERBOSE("Event "<<ctx.eventID().event_number()<<" - Retrieved muon Container from "<<m_muonSGKey.fullKey()<<" which contains "
                        <<muons->size()<<" muons. Created "<<muo_decor.decorKey()
                        <<" "<<SG::AuxTypeRegistry::instance().getName(muo_decor.auxid()));

       
        /// Retrieve the truth particle container if it's available
        std::vector<const xAOD::TruthParticle*> truth{};
        if (!m_truthSGKey.empty()) {
            
            SG::ReadHandle<xAOD::TruthParticleContainer> handle{m_truthSGKey, ctx};
            if (!handle.isValid()) {
                ATH_MSG_FATAL("Failed to retrieve truth container " << m_truthSGKey.fullKey());
                return StatusCode::FAILURE;
            }
            truth.reserve(handle->size());
            std::copy_if(handle->begin(), handle->end(), std::back_inserter(truth), 
                        [](const xAOD::TruthParticle* tpart) {
                            return MC::isStable(tpart) &&
                                    !HepMC::is_simulation_particle(tpart) &&
                                    tpart->isMuon();
                        });
        }
        for (const xAOD::Muon* mu_itr1 : *muons) {
            /// Save all muons coming from truth            
             if(std::find_if(truth.begin(), truth.end(), 
                [&](const xAOD::TruthParticle* truth_itr) {
                return  xAOD::P4Helpers::deltaR2(truth_itr, mu_itr1) < m_thinningConeSize2;
                }) != truth.end()){
                muo_decor(*mu_itr1) = true;
            }

            ATH_MSG_VERBOSE("Check tag muon: "<<mu_itr1->pt() * MeVtoGeV<<" [GeV], eta: "<<mu_itr1->eta()
                          <<", phi: "<<mu_itr1->phi()<<", author: "<<mu_itr1->author());
            if (!passMuonCuts(mu_itr1, m_mu1PtMin, m_mu1AbsEtaMax, m_applyQualityMu1)) {
                ATH_MSG_VERBOSE("Muon failed selection criteria");
                continue;
            }
            bool passOrTrig = passTrigger(mu_itr1, m_orTrigs);
            bool passAndTrig = !m_andTrigs.empty() && passTrigger(mu_itr1, m_andTrigs);
            for (const xAOD::Muon* mu_itr2 : *muons) {
                if (mu_itr2 == mu_itr1) continue;
                 ATH_MSG_VERBOSE("Check probe muon: "<<mu_itr2->pt() * MeVtoGeV<<" [GeV], eta: "<<mu_itr2->eta()
                            <<", phi: "<<mu_itr2->phi()<<", author: "<<mu_itr2->author());
                if (!passMuonCuts(mu_itr2, m_mu2PtMin, m_mu2AbsEtaMax, m_applyQualityMu2)) {
                    continue;
                }
                if (!muonPairCheck(mu_itr1, mu_itr2)) continue;
                bool passDiLepTrig = passOrTrig || passTrigger(mu_itr2, m_orTrigs) || (passAndTrig && passTrigger(mu_itr2, m_andTrigs));
                if (!passDiLepTrig) {
                    ATH_MSG_DEBUG("Trigger selection failed");
                    continue;
                }
                muo_decor(*mu_itr1) = true;
                muo_decor(*mu_itr2) = true;
                ATH_MSG_VERBOSE("T&P pair accepted");
                ++keepEvent;
            }          
        }
        if (!m_useTrackProbe) {
            return StatusCode::SUCCESS;
        }

        SG::ReadHandle<xAOD::TrackParticleContainer> tracks{m_trackSGKey, ctx};
        if (!tracks.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve " << m_trackSGKey.fullKey());
            return StatusCode::FAILURE;
        }
        ATH_MSG_VERBOSE("Event "<<ctx.eventID().event_number()<<" - Retrieved track Container from "<<m_trackSGKey.fullKey()<<" which contains "
                        <<tracks->size()<<" tracks.");
        TrackPassDecor trk_decor{makeHandle<bool>(ctx, m_trkKeepKey)};      
        for (const xAOD::Muon* mu_itr1 : *muons) {           
            if (!passMuonCuts(mu_itr1, m_mu1PtMin, m_mu1AbsEtaMax, m_applyQualityMu1)) {
                ATH_MSG_VERBOSE("Muon does not pass the trigger selection");
                continue;
            }
            if (!passTrigger(mu_itr1, m_orTrigs)) {
                ATH_MSG_VERBOSE("Muon does not trigger the event");
                continue;
            }
           
            for (const xAOD::TrackParticle* probe_trk : *tracks) { 
                if (probe_trk == mu_itr1->trackParticle(xAOD::Muon::TrackParticleType::InnerDetectorTrackParticle)) continue;
                if (!passKinematicCuts(probe_trk, m_mu2PtMin, m_mu2AbsEtaMax)) continue;
                if (!muonPairCheck(mu_itr1, probe_trk)) continue;
                trk_decor(*probe_trk) = true;
                ++keepEvent;
                /// Close by tracks are written as well to ensure that we can
                /// redo the isolation variables for our probes
                maskNearbyIDtracks(probe_trk, trk_decor);
                
            }
        }
        //// also mask tracks around truth muons
        for (const xAOD::TruthParticle* mu_itr2 : truth) {
            maskNearbyIDtracks(mu_itr2, trk_decor);
        }
        for (const xAOD::Muon* mu_itr : *muons) {
            passKinematicCuts(mu_itr, -1., -1.);
            ATH_MSG_VERBOSE("Pass decoration "<<muo_decor(*mu_itr));
        }
        return StatusCode::SUCCESS;
    }
    void DiMuonTaggingAlg::maskNearbyIDtracks(const xAOD::IParticle* ref_part, TrackPassDecor& decor) const {
        for (const xAOD::TrackParticle* trk : *decor) {
            if (xAOD::P4Helpers::deltaR2(ref_part, trk) < m_thinningConeSize2) decor(*trk) = true;
        }
    }
    bool DiMuonTaggingAlg::passKinematicCuts(const xAOD::IParticle* mu, const float ptMin, const float absEtaMax) const {
        ATH_MSG_VERBOSE("Particle with pt: "<<mu->pt() * MeVtoGeV<<" [GeV], eta: "<<mu->eta()
                          <<", phi: "<<mu->phi()<<", needs to be more energetic than "<<ptMin * MeVtoGeV<<" [GeV] and within "<<absEtaMax);
        return !(!mu || mu->pt() < ptMin || std::abs(mu->eta()) > absEtaMax);
    }
    bool DiMuonTaggingAlg::passMuonCuts(const xAOD::Muon* muon, const float ptMin, const float absEtaMax, const bool applyQuality) const{
        return passKinematicCuts(muon, ptMin, absEtaMax) && (!applyQuality  || m_muonSelTool->accept(*muon));
    }
    bool DiMuonTaggingAlg::passTrigger(const xAOD::Muon* muon, const std::vector<std::string>& trigList) const {
         return trigList.empty() || std::find_if(trigList.begin(), trigList.end(), [&](const std::string& trig_item){
            return m_matchingTool->match(*muon, trig_item, m_triggerMatchDeltaR);
         }) != trigList.end();
    }
    template <class probe_type> bool DiMuonTaggingAlg::muonPairCheck(const xAOD::Muon* mu1, const probe_type* mu2) const {
        if (m_requireOS != (mu1->charge() * mu2->charge() < 0)) {
            ATH_MSG_VERBOSE("Charge cut failed ");
            return false;
        }
        if (m_dPhiMin > 0 && std::abs(xAOD::P4Helpers::deltaPhi(mu1, mu2)) < m_dPhiMin) {
            ATH_MSG_VERBOSE("Delta phi cut "<<m_dPhiMin<<" was undercut "<<std::abs(xAOD::P4Helpers::deltaPhi(mu1, mu2)));
            return false;
        }
        const float mass2 = (mu1->p4() + mu2->p4()).M2();
        ATH_MSG_VERBOSE("Invariant mass "<<std::sqrt(mass2)*MeVtoGeV<<" need to be in the window ["
                      <<m_invariantMassLow*MeVtoGeV<<";"<<m_invariantMassHigh * MeVtoGeV<<"]");
        return !(mass2 < m_invariantMassLow2 || (m_invariantMassHigh > 0. && mass2 > m_invariantMassHigh2));
    }
}  // namespace DerivationFramework
