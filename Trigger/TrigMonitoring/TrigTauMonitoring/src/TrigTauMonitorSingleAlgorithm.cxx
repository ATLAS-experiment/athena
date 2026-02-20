/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigTauMonitorSingleAlgorithm.h"
#include "TrigDecisionTool/TrigDecisionTool.h"

#include "StoreGate/ReadDecorHandle.h"


TrigTauMonitorSingleAlgorithm::TrigTauMonitorSingleAlgorithm(const std::string& name, ISvcLocator* pSvcLocator)
    : TrigTauMonitorBaseAlgorithm(name, pSvcLocator)
{}


StatusCode TrigTauMonitorSingleAlgorithm::initialize()
{
    ATH_CHECK( TrigTauMonitorBaseAlgorithm::initialize() );

    // Create the TauID score decor handle keys for the Monitoring...
    for(const auto& [seq_name, m] : m_monitoredHLTIdScores) {
        const std::string online_container_name = getOnlineContainerKey(seq_name).key();
        for(const auto& [key, p] : m) {
            if(p.first.empty() || p.second.empty()) {
                ATH_MSG_WARNING("Invalid HLT TauID score variable names; skipping this entry for the monitoring!");
                continue;
            }

            m_monitoredHLTIdDecorHandleKeys[seq_name].emplace(
                key, 
                std::make_pair(
                    SG::ReadDecorHandleKey<xAOD::TauJetContainer>(online_container_name + "." + p.first),
                    SG::ReadDecorHandleKey<xAOD::TauJetContainer>(online_container_name + "." + p.second)
                )
            );
            ATH_CHECK(m_monitoredHLTIdDecorHandleKeys.at(seq_name).at(key).first.initialize());
            ATH_CHECK(m_monitoredHLTIdDecorHandleKeys.at(seq_name).at(key).second.initialize());
        }
    }

    for(const auto& [key, p] : m_monitoredOfflineIdScores) {
        if(p.first.empty() || p.second.empty()) {
            ATH_MSG_WARNING("Invalid Offline TauID score variable names; skipping this entry for the monitoring!");
            continue;
        }

        m_monitoredOfflineIdDecorHandleKeys.emplace(
            key, 
            std::make_pair(
                SG::ReadDecorHandleKey<xAOD::TauJetContainer>(m_offlineTauJetKey.key() + "." + p.first),
                SG::ReadDecorHandleKey<xAOD::TauJetContainer>(m_offlineTauJetKey.key() + "." + p.second)
            )
        );
        ATH_CHECK(m_monitoredOfflineIdDecorHandleKeys.at(key).first.initialize());
        ATH_CHECK(m_monitoredOfflineIdDecorHandleKeys.at(key).second.initialize());
    }

    return StatusCode::SUCCESS;
}


StatusCode TrigTauMonitorSingleAlgorithm::processEvent(const EventContext& ctx) const
{
    constexpr float threshold_offset = 10.0;

    // Offline taus
    auto offline_taus_all = getOfflineTausAll(ctx, 0.0);
    if(m_requireOfflineTaus && offline_taus_all.empty()) return StatusCode::SUCCESS;

    for(const std::string& trigger : m_triggers) {
        const TrigTauInfo& info = getTrigInfo(trigger);

        if(!info.isHLTSingleTau()) {
            ATH_MSG_WARNING("Chain \"" << trigger << "\" is not a single tau trigger. Skipping...");
            continue;
        }

        const auto passBits = m_trigDecTool->isPassedBits(trigger);
        const bool l1_accept_flag = passBits & TrigDefs::L1_isPassedAfterVeto;
        const bool hlt_not_prescaled_flag = (passBits & TrigDefs::EF_prescaled) == 0;

        // Offline tau requirement check
        const std::vector<const xAOD::TauJet*> offline_taus_with_id = classifyTausAll(offline_taus_all, 0, static_cast<TauID>(m_offline_tau_id.value()));
        if(m_requireOfflineTaus && offline_taus_with_id.empty()) continue;

        // Filter offline taus
        auto offline_taus = classifyOfflineTaus(offline_taus_with_id, info.getHLTTauThreshold() - threshold_offset);
        std::vector<const xAOD::TauJet*> offline_taus_1p = offline_taus.first;
        std::vector<const xAOD::TauJet*> offline_taus_3p = offline_taus.second;

        // Online taus
        std::vector<const xAOD::TauJet*> hlt_taus_all = getOnlineTausAll(trigger, true);
        auto hlt_taus = classifyOnlineTaus(hlt_taus_all);
        std::vector<const xAOD::TauJet*> hlt_taus_0p = std::get<0>(hlt_taus);
        std::vector<const xAOD::TauJet*> hlt_taus_1p = std::get<1>(hlt_taus);
        std::vector<const xAOD::TauJet*> hlt_taus_mp = std::get<2>(hlt_taus);

        if(m_do_variable_plots) {
            // Offline variables:
            if(m_doOfflineTausDistributions && !offline_taus_1p.empty()) {
                fillBasicVars(ctx, trigger, offline_taus_1p, "1P", false);
                fillIDScores(ctx, trigger, offline_taus_1p, "1P", false);
                fillIDInputVars(trigger, offline_taus_1p, "1P", false);
                fillIDTrack(trigger, offline_taus_1p, false);
                fillIDCluster(trigger, offline_taus_1p, false);
            }
            if(m_doOfflineTausDistributions && !offline_taus_3p.empty()) {
                fillBasicVars(ctx, trigger, offline_taus_3p, "3P", false);
                fillIDScores(ctx, trigger, offline_taus_3p, "3P", false);
                fillIDInputVars(trigger, offline_taus_3p, "3P", false);
                fillIDTrack(trigger, offline_taus_3p, false);
                fillIDCluster(trigger, offline_taus_3p, false);
            }

            // Fill information for online 0 prong taus
            if(!hlt_taus_0p.empty()) {
                fillBasicVars(ctx, trigger, hlt_taus_0p, "0P", true);
                fillIDScores(ctx, trigger, hlt_taus_0p, "0P", true);
                fillIDInputVars(trigger, hlt_taus_0p, "0P", true);
                fillIDTrack(trigger, hlt_taus_0p, true);
                fillIDCluster(trigger, hlt_taus_0p, true);
            }

            // Fill information for online 1 prong taus
            if(!hlt_taus_1p.empty()) {
                fillBasicVars(ctx, trigger, hlt_taus_1p, "1P", true);
                fillIDScores(ctx, trigger, hlt_taus_1p, "1P", true);
                fillIDInputVars(trigger, hlt_taus_1p, "1P", true);
                fillIDTrack(trigger, hlt_taus_1p, true);
                fillIDCluster(trigger, hlt_taus_1p, true);
            }

            // Fill information for online multiprong prong taus 
            if(!hlt_taus_mp.empty()) {
                fillBasicVars(ctx, trigger, hlt_taus_mp, "MP", true);
                fillIDScores(ctx, trigger, hlt_taus_mp, "MP", true);
                fillIDInputVars(trigger, hlt_taus_mp, "MP", true);
                fillIDTrack(trigger, hlt_taus_mp, true);
                fillIDCluster(trigger, hlt_taus_mp, true);
            }
	    }

        if(m_do_efficiency_plots && hlt_not_prescaled_flag) {
            fillHLTEfficiencies(ctx, trigger, l1_accept_flag, offline_taus_1p, hlt_taus_all, "1P");
            fillHLTEfficiencies(ctx, trigger, l1_accept_flag, offline_taus_3p, hlt_taus_all, "3P");
        }
    }

    return StatusCode::SUCCESS;
}


void TrigTauMonitorSingleAlgorithm::fillHLTEfficiencies(const EventContext& ctx, const std::string& trigger, const bool l1_accept_flag, const std::vector<const xAOD::TauJet*>& offline_tau_vec, const std::vector<const xAOD::TauJet*>& online_tau_vec, const std::string& nProng) const
{
    ATH_MSG_DEBUG("Fill HLT " << nProng << " efficiencies: " << trigger);

    const TrigTauInfo& info = getTrigInfo(trigger);

    // Efficiency for single leg tau triggers:
    // denominator = offline tau + matching with L1 object with dR(offline tau,L1 item) < 0.3
    // numerator = denominator + hlt fires + matching with HLT tau with dR(offline tau, HLT tau) < 0.2

    auto monGroup = getGroup(trigger+"_HLT_Efficiency_"+nProng);

    auto tauPt = Monitored::Scalar<float>("tauPt", 0.0);
    auto tauEta = Monitored::Scalar<float>("tauEta", 0.0);
    auto tauPhi = Monitored::Scalar<float>("tauPhi", 0.0);
    auto averageMu = Monitored::Scalar<float>("averageMu", 0.0); 
    auto HLT_match = Monitored::Scalar<bool>("HLT_pass", false);
    auto HLT_match_highPt = Monitored::Scalar<bool>("HLT_pass_highPt", false);
    auto Total_match = Monitored::Scalar<bool>("Total_pass", false);
    auto Total_match_highPt = Monitored::Scalar<bool>("Total_pass_highPt", false);

    bool hlt_fires = m_trigDecTool->isPassed(trigger, TrigDefs::Physics | TrigDefs::allowResurrectedDecision);

    std::vector<TLorentzVector> rois = getRoIsVector(ctx, trigger);
    for(const auto *offline_tau : offline_tau_vec) {
        bool L1_match = false;
    
        // Check the matching offline tau with L1 item -> depending on the L1 type (phase-1 eTAU, jTAU, cTAU)
        // All L1 RoIs have a core size of 3x3 TTs -> 0.3 x 0.3
        for(const TLorentzVector& roi : rois) {
            L1_match = offline_tau->p4().DeltaR(roi) <= 0.3;
            if(L1_match) break;
        }

        tauPt = offline_tau->pt()/Gaudi::Units::GeV;
        tauEta = offline_tau->eta();
        tauPhi = offline_tau->phi();
        averageMu = lbAverageInteractionsPerCrossing(ctx);

        bool is_highPt = tauPt > info.getHLTTauThreshold() + 20.0;

        // HLT matching: dR matching + HLT fires
        HLT_match = matchObjects(offline_tau, online_tau_vec, 0.2) && hlt_fires;

        // Total efficiency (without L1 matching)
        if(m_doTotalEfficiency) {
            Total_match = static_cast<bool>(HLT_match);
            fill(monGroup, tauPt, tauEta, tauPhi, Total_match);

            if(is_highPt) {
                Total_match_highPt = static_cast<bool>(HLT_match);
                fill(monGroup, tauEta, tauPhi, Total_match_highPt);
            }
        }

        if(!L1_match || !l1_accept_flag) continue; // Skip this offline tau since not matched with L1 item   

        fill(monGroup, tauPt, tauEta, tauPhi, averageMu, HLT_match);

        if(is_highPt) {
            HLT_match_highPt = static_cast<bool>(HLT_match);
            fill(monGroup, tauEta, tauPhi, HLT_match_highPt);
        }
    }

    ATH_MSG_DEBUG("After fill HLT efficiencies: " << trigger);
}


void TrigTauMonitorSingleAlgorithm::fillIDInputVars(const std::string& trigger, const std::vector<const xAOD::TauJet*>& tau_vec,const std::string& nProng, bool online) const
{
    ATH_MSG_DEBUG("Fill ID input variables: " << trigger);

    auto monGroup = getGroup(trigger+"_ID_"+(online ? "HLT" : "Offline")+"_InputScalar_"+nProng);  

    auto centFrac           = Monitored::Collection("centFrac", tau_vec, [](const xAOD::TauJet* tau){
                                                        float detail = -999;
                                                        if(tau->detail(xAOD::TauJetParameters::centFrac, detail)) detail = std::min(detail, 1.0f);
                                                        return detail;
                                                    });
    auto etOverPtLeadTrk    = Monitored::Collection("etOverPtLeadTrk", tau_vec, [](const xAOD::TauJet* tau){
                                                        float detail = -999;
                                                        if(tau->detail(xAOD::TauJetParameters::etOverPtLeadTrk, detail)) detail = std::log10(std::max(detail, 0.1f));
                                                        return detail;
                                                    });
    auto dRmax              = Monitored::Collection("dRmax", tau_vec, [](const xAOD::TauJet* tau){
                                                        float detail = -999;
                                                        tau->detail(xAOD::TauJetParameters::dRmax, detail);
                                                        return detail;
                                                    });
    auto absipSigLeadTrk    = Monitored::Collection("absipSigLeadTrk", tau_vec, [](const xAOD::TauJet* tau){
                                                        float detail = (tau->nTracks()>0) ? std::abs(tau->track(0)->d0SigTJVA()) : 0;
                                                        detail = std::min(std::abs(detail), 30.0f);
                                                        return detail;
                                                    });
    auto sumPtTrkFrac       = Monitored::Collection("sumPtTrkFrac", tau_vec, [](const xAOD::TauJet* tau){
                                                        float detail = -999;
                                                        tau->detail(xAOD::TauJetParameters::SumPtTrkFrac, detail);
                                                        return detail;
                                                    });
    auto emPOverTrkSysP     = Monitored::Collection("emPOverTrkSysP", tau_vec, [](const xAOD::TauJet* tau){
                                                        float detail = -999;
                                                        if(tau->detail(xAOD::TauJetParameters::EMPOverTrkSysP, detail)) detail = std::log10(std::max(detail, 1e-3f));
                                                        return detail;
                                                    });
    auto ptRatioEflowApprox = Monitored::Collection("ptRatioEflowApprox", tau_vec, [](const xAOD::TauJet* tau){
                                                        float detail = -999;
                                                        if(tau->detail(xAOD::TauJetParameters::ptRatioEflowApprox, detail)) detail = std::min(detail, 4.0f);
                                                        return detail;
                                                    });
    auto mEflowApprox       = Monitored::Collection("mEflowApprox", tau_vec, [](const xAOD::TauJet* tau){
                                                        float detail = -999;
                                                        if(tau->detail(xAOD::TauJetParameters::mEflowApprox, detail)) detail = std::log10(std::max(detail, 140.0f));
                                                        return detail;
                                                    });
    auto ptDetectorAxis     = Monitored::Collection("ptDetectorAxis", tau_vec, [](const xAOD::TauJet* tau){
                                                        float detail = -999;
                                                        if( tau->ptDetectorAxis() > 0) detail = std::log10(std::min(tau->ptDetectorAxis()/Gaudi::Units::GeV, 100.0));
                                                        return detail;
                                                    });
    auto massTrkSys         = Monitored::Collection("massTrkSys", tau_vec, [&nProng](const xAOD::TauJet* tau){
                                                        float detail = -999;
                                                        if( tau->detail(xAOD::TauJetParameters::massTrkSys, detail) && (nProng.find("MP") != std::string::npos || nProng.find("3P") != std::string::npos)) {
                                                        detail = std::log10(std::max(detail, 140.0f));
                                                        }
                                                    return detail;});
    auto trFlightPathSig    = Monitored::Collection("trFlightPathSig", tau_vec, [&nProng](const xAOD::TauJet* tau){
                                                        float detail = -999;
                                                        if(nProng.find("MP") != std::string::npos || nProng.find("3P") != std::string::npos) tau->detail(xAOD::TauJetParameters::trFlightPathSig, detail);
                                                        return detail;
                                                    });
    
    fill(monGroup, centFrac, etOverPtLeadTrk, dRmax, absipSigLeadTrk, sumPtTrkFrac, emPOverTrkSysP, ptRatioEflowApprox, mEflowApprox, ptDetectorAxis, massTrkSys, trFlightPathSig);     

    ATH_MSG_DEBUG("After fill ID input variables: " << trigger);
}


void TrigTauMonitorSingleAlgorithm::fillIDTrack(const std::string& trigger, const std::vector<const xAOD::TauJet*>& tau_vec, bool online) const
{
    ATH_MSG_DEBUG("Fill ID input Track: " << trigger);

    auto monGroup = getGroup(trigger+"_ID_"+(online ? "HLT" : "Offline")+"_InputTrack");  

    auto track_pt_jetseed_log = Monitored::Collection("track_pt_jetseed_log", tau_vec, [](const xAOD::TauJet* tau){ return std::log10(tau->ptJetSeed()); });
    fill(monGroup, track_pt_jetseed_log);

    for(const auto *tau : tau_vec) {
        // Don't call ->allTracks() unless the element links are valid
        static const SG::ConstAccessor< std::vector<ElementLink<xAOD::TauTrackContainer>> > tauTrackAcc("tauTrackLinks");
        bool linksValid = true;
        for(const ElementLink<xAOD::TauTrackContainer>& trackEL : tauTrackAcc(*tau)) {
            if(!trackEL.isValid()) {
                linksValid = false;
                break;
            }
        }
        if(!linksValid) {
            ATH_MSG_WARNING("Invalid track element links from TauJet in " << trigger);
            continue;
        }

        auto tracks = tau->allTracks();
        std::sort(tracks.begin(), tracks.end(), [](const xAOD::TauTrack* lhs, const xAOD::TauTrack* rhs){ return lhs->pt() > rhs->pt(); });
                                
        auto n_track = Monitored::Scalar<int>("n_track", tracks.size());

        auto track_pt_log = Monitored::Collection("track_pt_log", tracks, [](const xAOD::TauTrack *track){ return std::log10(track->pt()); }); 
        auto track_eta = Monitored::Collection("track_eta", tracks, [](const xAOD::TauTrack *track){ return track->eta(); });
        auto track_phi = Monitored::Collection("track_phi", tracks, [](const xAOD::TauTrack *track){ return track->phi(); }); 

        auto track_dEta = Monitored::Collection("track_dEta", tracks, [&tau](const xAOD::TauTrack *track){ return track->eta() - tau->eta(); });
        auto track_dPhi = Monitored::Collection("track_dPhi", tracks, [&tau](const xAOD::TauTrack *track){ return track->p4().DeltaPhi(tau->p4()); });

        auto track_z0sinthetaTJVA_abs_log = Monitored::Collection("track_z0sinthetaTJVA_abs_log", tracks, [](const xAOD::TauTrack *track){return track->z0sinthetaTJVA(); }); 
        auto track_d0_abs_log = Monitored::Collection("track_d0_abs_log", tracks, [](const xAOD::TauTrack *track){ return std::log10(std::abs(track->track()->d0()) + 1e-6); }); 

        auto track_nIBLHitsAndExp = Monitored::Collection("track_nIBLHitsAndExp", tracks, [](const xAOD::TauTrack *track){
                                                            uint8_t inner_pixel_hits, inner_pixel_exp;
                                                            const auto success1_innerPixel_hits = track->track()->summaryValue(inner_pixel_hits, xAOD::numberOfInnermostPixelLayerHits);
                                                            const auto success2_innerPixel_exp = track->track()->summaryValue(inner_pixel_exp, xAOD::expectInnermostPixelLayerHit);
                                                            float detail = -999;
                                                            if(success1_innerPixel_hits && success2_innerPixel_exp) { detail = inner_pixel_exp ? inner_pixel_hits : 1.; };
                                                            return detail;
                                                        });
        auto track_nPixelHitsPlusDeadSensors = Monitored::Collection("track_nPixelHitsPlusDeadSensors", tracks, [](const xAOD::TauTrack *track){
                                                                    uint8_t pixel_hits, pixel_dead;
                                                                    const auto success1_pixel_hits = track->track()->summaryValue(pixel_hits, xAOD::numberOfPixelHits);
                                                                    const auto success2_pixel_dead = track->track()->summaryValue(pixel_dead, xAOD::numberOfPixelDeadSensors);
                                                                    float detail = -999;
                                                                    if(success1_pixel_hits && success2_pixel_dead) { detail = pixel_hits + pixel_dead; };
                                                                    return detail;
                                                                    });
        auto track_nSCTHitsPlusDeadSensors = Monitored::Collection("track_nSCTHitsPlusDeadSensors", tracks, [](const xAOD::TauTrack *track){
                                                                    uint8_t sct_hits, sct_dead;
                                                                    const auto success1_sct_hits = track->track()->summaryValue(sct_hits, xAOD::numberOfSCTHits);
                                                                    const auto success2_sct_dead = track->track()->summaryValue(sct_dead, xAOD::numberOfSCTDeadSensors);
                                                                    float detail = -999;
                                                                    if(success1_sct_hits && success2_sct_dead) { detail = sct_hits + sct_dead; };
                                                                    return detail;
                                                                });
                                                    
        fill(monGroup, n_track, track_pt_log, track_eta, track_phi, track_dEta, track_dPhi, track_z0sinthetaTJVA_abs_log, track_d0_abs_log, track_nIBLHitsAndExp, track_nPixelHitsPlusDeadSensors, track_nSCTHitsPlusDeadSensors);
    }

    ATH_MSG_DEBUG("After fill ID input Track: " << trigger);
}


void TrigTauMonitorSingleAlgorithm::fillIDCluster(const std::string& trigger, const std::vector<const xAOD::TauJet*>& tau_vec, bool online) const
{
    ATH_MSG_DEBUG("Fill ID input Cluster: " << trigger << " for online/offline " << online);
    
    auto monGroup = getGroup(trigger+"_ID_"+(online ? "HLT" : "Offline")+"_InputCluster");  
    
    for(const auto *tau : tau_vec){
        auto cluster_pt_jetseed_log = Monitored::Collection("cluster_pt_jetseed_log", tau_vec, [](const xAOD::TauJet* tau){ return std::log10(tau->ptJetSeed()); });

        std::vector<const xAOD::CaloCluster*> clusters;
        for(const xAOD::IParticle* particle : tau->clusters()) {
            const xAOD::CaloCluster* cluster = static_cast<const xAOD::CaloCluster*>(particle);
            clusters.push_back(cluster); 
        } 
        std::sort(clusters.begin(), clusters.end(), [](const xAOD::CaloCluster *lhs, const xAOD::CaloCluster *rhs){ return lhs->et() > rhs->et(); });

        auto n_cluster = Monitored::Scalar<int>("n_cluster", 0);
        n_cluster = clusters.size();

        auto cluster_et_log = Monitored::Collection("cluster_et_log",clusters, [](const xAOD::CaloCluster *cluster){ return std::log10( cluster->et()); });
        auto cluster_eta = Monitored::Collection("cluster_eta", clusters, [](const xAOD::CaloCluster *cluster){ return cluster->eta(); });
        auto cluster_phi = Monitored::Collection("cluster_phi", clusters, [](const xAOD::CaloCluster *cluster){ return cluster->phi(); });
        auto cluster_dEta = Monitored::Collection("cluster_dEta", clusters, [&tau](const xAOD::CaloCluster *cluster){ return cluster->eta() - tau->eta(); });
        auto cluster_dPhi = Monitored::Collection("cluster_dPhi", clusters, [&tau](const xAOD::CaloCluster *cluster){ return cluster->p4().DeltaPhi(tau->p4()); }); 
        auto cluster_SECOND_R_log10 = Monitored::Collection("cluster_SECOND_R_log10", clusters, [](const xAOD::CaloCluster *cluster){
                                                            double detail = -999;
                                                            const auto success_SECOND_R = cluster->retrieveMoment(xAOD::CaloCluster::MomentType::SECOND_R,detail);
                                                            if(success_SECOND_R) detail = std::log10(detail + 0.1);
                                                            return detail;
                                                            });

        auto cluster_SECOND_LAMBDA_log10 = Monitored::Collection("cluster_SECOND_LAMBDA_log10", clusters, [](const xAOD::CaloCluster *cluster){
                                                                double detail = -999;
                                                                const auto success_SECOND_LAMBDA = cluster->retrieveMoment(xAOD::CaloCluster::MomentType::SECOND_LAMBDA, detail);
                                                                if(success_SECOND_LAMBDA) detail = std::log10(detail + 0.1); 
                                                                return detail;
                                                                });

        auto cluster_CENTER_LAMBDA_log10 = Monitored::Collection("cluster_CENTER_LAMBDA_log10", clusters, [](const xAOD::CaloCluster *cluster){
                                                                double detail = -999;
                                                                const auto success_CENTER_LAMBDA = cluster->retrieveMoment(xAOD::CaloCluster::MomentType::CENTER_LAMBDA, detail);
                                                                if(success_CENTER_LAMBDA) detail = std::log10(detail + 1e-6); 
                                                                return detail;
                                                                });

        fill(monGroup, n_cluster, cluster_pt_jetseed_log, cluster_et_log, cluster_eta, cluster_phi, cluster_dEta, cluster_dPhi, cluster_SECOND_R_log10, cluster_SECOND_LAMBDA_log10, cluster_CENTER_LAMBDA_log10);
    }

    ATH_MSG_DEBUG("After fill ID input Cluster: " << trigger);
}


void TrigTauMonitorSingleAlgorithm::fillBasicVars(const EventContext& ctx, const std::string& trigger, const std::vector<const xAOD::TauJet*>& tau_vec, const std::string& nProng, bool online) const
{
    ATH_MSG_DEBUG("Fill Basic Variables: " << trigger); 

    auto monGroup = getGroup(trigger+"_"+(online ? "HLT" : "Offline")+"_basicVars_"+nProng);  

    auto Pt = Monitored::Collection("Pt", tau_vec, [](const xAOD::TauJet* tau){ return tau->pt()/Gaudi::Units::GeV; });
    auto Eta = Monitored::Collection("Eta", tau_vec, [](const xAOD::TauJet* tau){ return tau->eta(); });                                                     
    auto Phi = Monitored::Collection("Phi", tau_vec, [](const xAOD::TauJet* tau){ return tau->phi(); });

    auto nTrack = Monitored::Collection("nTrack", tau_vec, [](const xAOD::TauJet* tau){
                                            int nTrack = -1;
                                            tau->detail(xAOD::TauJetParameters::nChargedTracks, nTrack);
                                            return nTrack;
                                            });
    auto nIsoTrack = Monitored::Collection("nIsoTrack", tau_vec, [](const xAOD::TauJet* tau){ return tau->nTracksIsolation(); });

    auto averageMu = Monitored::Scalar<float>("averageMu", 0.0);
    averageMu = lbAverageInteractionsPerCrossing(ctx);
    
    auto TauVertexX = Monitored::Collection("TauVertexX", tau_vec, [](const xAOD::TauJet* tau){
                                                double vtx = -999;
                                                if(tau->vertex() != nullptr) vtx = tau->vertex()->x();
                                                return vtx;
                                            });
    auto TauVertexY = Monitored::Collection("TauVertexY", tau_vec, [](const xAOD::TauJet* tau){
                                                double vty = -999;
                                                if(tau->vertex() != nullptr) vty = tau->vertex()->y();
                                                return vty;
                                            });
    auto TauVertexZ = Monitored::Collection("TauVertexZ", tau_vec, [](const xAOD::TauJet* tau){
                                                double vtz = -999;
                                                if(tau->vertex() != nullptr) vtz = tau->vertex()->z();
                                                return vtz;
                                            });

    fill(monGroup, Pt, Eta, Phi, nTrack, nIsoTrack, averageMu, TauVertexX, TauVertexY, TauVertexZ);

    ATH_MSG_DEBUG("After fill Basic variables: " << trigger);
}


void TrigTauMonitorSingleAlgorithm::fillIDScores(const EventContext& ctx, const std::string& trigger, const std::vector<const xAOD::TauJet*>& tau_vec, const std::string& nProng, bool online) const
{
    ATH_MSG_DEBUG("Fill TauID Scores: " << trigger); 

    const TrigTauInfo& info = getTrigInfo(trigger);
    const std::string tau_id = info.getHLTTauID();
    bool store_all = tau_id == "idperf" || tau_id == "perf";

    if(online) {
        if(m_monitoredHLTIdDecorHandleKeys.find(info.getHLTTauType()) == m_monitoredHLTIdDecorHandleKeys.end()) return;
    } else {
        if(m_monitoredOfflineIdDecorHandleKeys.size() == 0) return;
    }

    
    const auto& decor_handle_keys = online ? m_monitoredHLTIdDecorHandleKeys.at(info.getHLTTauType()) : m_monitoredOfflineIdDecorHandleKeys;

    // This has to be down here, because otherwise we crash on unmonitored chains
    auto monGroup = getGroup(trigger+"_"+(online ? "HLT" : "Offline")+"_IDScores_"+nProng);

    for(const auto& [key, p] : decor_handle_keys) {
        // We will either store the TauID scores indicated in the chain name, or all of them in case of (id)perf chains
        if(online && !store_all && tau_id != key) continue;
           
        SG::ReadDecorHandle<xAOD::TauJetContainer, float> score_handle(p.first, ctx);
        SG::ReadDecorHandle<xAOD::TauJetContainer, float> score_sig_trans_handle(p.second, ctx);

        // Skip if both the Score and ScoreSigTrans aren't available
        if(!score_handle.isAvailable() || !score_sig_trans_handle.isAvailable()) continue;

        std::vector<float> score, score_sig_trans;
        for(const xAOD::TauJet* tau : tau_vec) {
            score.push_back(score_handle(*tau));
            score_sig_trans.push_back(score_sig_trans_handle(*tau));
        }

        auto IDScore = Monitored::Collection(key + "_TauIDScore", score);
        auto IDScoreSigTrans = Monitored::Collection(key + "_TauIDScoreSigTrans", score_sig_trans);
        fill(monGroup, IDScore, IDScoreSigTrans);
    }

    ATH_MSG_DEBUG("After fill TauID Scores: " << trigger);
}


std::vector<TLorentzVector> TrigTauMonitorSingleAlgorithm::getRoIsVector(const EventContext& ctx, const std::string& trigger) const
{
    std::vector<TLorentzVector> ret;

    const TrigTauInfo& info = getTrigInfo(trigger);

    TLorentzVector v;
    if(info.getL1TauType() == "eTAU") {
        for(const xAOD::eFexTauRoI* roi : getL1eTAUs(ctx, info.getL1TauItem())) {
            v.SetPtEtaPhiM(roi->et(), roi->eta(), roi->phi(), 0);
            ret.push_back(v);
        }
    } else if(info.getL1TauType() == "jTAU") {
        for(const xAOD::jFexTauRoI* roi : getL1jTAUs(ctx, info.getL1TauItem())) {
            v.SetPtEtaPhiM(roi->et(), roi->eta(), roi->phi(), 0);
            ret.push_back(v);
        }
    } else if(info.getL1TauType() == "cTAU") {
        for(const auto& [eTau_roi, jTau_roi] : getL1cTAUs(ctx, info.getL1TauItem())) {
            v.SetPtEtaPhiM(eTau_roi->et(), eTau_roi->eta(), eTau_roi->phi(), 0);
            ret.push_back(v);
        }
    } 

    return ret;
}
