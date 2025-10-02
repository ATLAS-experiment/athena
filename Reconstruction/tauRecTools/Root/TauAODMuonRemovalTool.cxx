/*
    Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "tauRecTools/TauAODMuonRemovalTool.h"

#define GeV 1000

TauAODMuonRemovalTool::TauAODMuonRemovalTool(const std::string& name):
    TauRecToolBase(name) {
}

StatusCode TauAODMuonRemovalTool::initialize() {
    ATH_CHECK(m_muonInputContainer.initialize());
    m_muonWpUi  = m_mapMuonIdWp.at(m_strMinMuonIdWp);
    return StatusCode::SUCCESS;
}

StatusCode TauAODMuonRemovalTool::execute(xAOD::TauJet& tau) const {
    // Read in muon container
    SG::ReadHandle<xAOD::MuonContainer> muon_input_handle(m_muonInputContainer);
    if (bool fail_muon = !muon_input_handle.isValid(); fail_muon) {
        ATH_MSG_ERROR( "Could not retrieve Muon container with key " + muon_input_handle.key() );
        return StatusCode::FAILURE;
    }
    auto muon_container = muon_input_handle.cptr();
    //Add the Aux element as empty vector
    const SG::Accessor<std::vector<ElementLink<xAOD::MuonContainer>>> acc_removed_muons("removedMuons");
    acc_removed_muons(tau).clear();
    //get the muon tracks and clusters
    auto muon_and_tracks   = decltype((getMuonAndTrk)(tau, *muon_container))();
    auto muon_and_clusters = decltype((getMuonAndCls)(tau, *muon_container))();
    if(m_doMuonTrkRm) muon_and_tracks   = getMuonAndTrk(tau, *muon_container);
    if(m_doMuonClsRm) muon_and_clusters = getMuonAndCls(tau, *muon_container);
    // if nothing found just give up here
    if(muon_and_tracks.empty() && muon_and_clusters.empty()) return StatusCode::SUCCESS;
    // remove the links from the tau
    auto tau_track_links = tau.allTauTrackLinksNonConst();
    auto tau_cluster_links = tau.clusterLinks();
    auto trk_removed_muons = removeTrks(tau_track_links,    muon_and_tracks);
    auto cls_removed_muons = removeClss(tau_cluster_links,  muon_and_clusters);
    tau.clearTauTrackLinks();
    tau.clearClusterLinks();
    tau.setClusterLinks(tau_cluster_links);
    tau.setAllTauTrackLinks(tau_track_links);
    //Merge the resulting vector and add them to sets
    auto removed_muons = std::move(trk_removed_muons);
    removed_muons.insert(removed_muons.end(), cls_removed_muons.begin(), cls_removed_muons.end());
    auto removed_muons_set = std::set(removed_muons.begin(), removed_muons.end());
    //set link to the removed lepton
    for (auto muon : removed_muons_set ){
        ElementLink<xAOD::MuonContainer> link;
        link.toContainedElement(*muon_container, muon);
        acc_removed_muons(tau).push_back(link);
    }
    //notify the runner alg that the tau was modified
    if (!acc_removed_muons(tau).empty())
    {
        const SG::Accessor<char> acc_modified("ModifiedInAOD");
        acc_modified(tau) = static_cast<char>(true);
    }
    return StatusCode::SUCCESS;
}

//helpers
std::vector<const xAOD::CaloCluster*> TauAODMuonRemovalTool::getOriginalTopoClusters(const xAOD::CaloCluster *cluster) const {
    static const SG::Accessor<std::vector<ElementLink<xAOD::CaloClusterContainer>>> acc_origClusterLinks("constituentClusterLinks");
    std::vector< const xAOD::CaloCluster* > orig_cls;
    if(acc_origClusterLinks.isAvailable(*cluster)) {
        auto links = acc_origClusterLinks(*cluster);
        for (const auto &link : links) {
            if (link.dataID() != "CaloCalTopoClusters")
                ATH_MSG_WARNING("the clusters in the lepton cannot be converted to CaloCalTopoClusters, the ID is " << link.dataID());
            if (link.isValid())
                orig_cls.push_back(*link);
        }
    }
    return orig_cls;
}

std::vector<std::pair<const xAOD::TrackParticle*, const xAOD::Muon*>> TauAODMuonRemovalTool::getMuonAndTrk(const xAOD::TauJet& tau, const xAOD::MuonContainer& muon_container) const {
    std::vector<std::pair<const xAOD::TrackParticle*, const xAOD::Muon*>> ret;
    std::for_each(muon_container.cbegin(), muon_container.cend(),
        [&](auto muon) -> void {
            if(tau.p4().DeltaR(muon->p4()) < m_lepRemovalConeSize && muon->quality() <= m_muonWpUi) {
                if(const auto & muon_ID_tracks_link = muon->inDetTrackParticleLink();  muon_ID_tracks_link.isValid())
                    ret.push_back(std::make_pair(std::move(*muon_ID_tracks_link), muon));
            }
        }
    );
    return ret;
}

std::vector<std::pair<const xAOD::CaloCluster*, const xAOD::Muon*>> TauAODMuonRemovalTool::getMuonAndCls(const xAOD::TauJet& tau, const xAOD::MuonContainer& muon_container) const {
    std::vector<std::pair<const xAOD::CaloCluster*, const xAOD::Muon*>> ret;
    std::for_each(muon_container.cbegin(), muon_container.cend(),
        [&](auto muon) -> void {
            if(tau.p4().DeltaR(muon->p4()) < m_lepRemovalConeSize && muon->quality() <= m_muonWpUi) {
                if(const auto & muon_cluster_link = muon->clusterLink();  muon_cluster_link.isValid()) {
                    auto muon_cluster = std::move(*muon_cluster_link);
                    auto muon_e = muon->e();
                    auto loss_e = muon->floatParameter(xAOD::Muon::ParamEnergyLoss);
                    auto cls_e = muon_cluster->e();
                    auto loss_diff = ((cls_e - loss_e) / (cls_e + loss_e));
                    if (muon_e > cls_e && loss_diff < 0.1 && loss_diff > -0.3) {
                        auto orig_muon_clusters = getOriginalTopoClusters(muon_cluster);
                        for (auto cluster : orig_muon_clusters)
                            ret.push_back(std::make_pair(cluster, muon));
                    }
                }
            }
        }
    );
    return ret;
}

template<typename Tlep, typename Tlinks> std::vector<Tlep> TauAODMuonRemovalTool::removeTrks(Tlinks& tau_trk_links, std::vector<std::pair<const xAOD::TrackParticle*, Tlep>>& tracks_and_leps) const {
    std::vector<Tlep> ret;
    tau_trk_links.erase(
        std::remove_if(tau_trk_links.begin(), tau_trk_links.end(),
            [&](auto tau_trk_link) -> bool {
                bool match = false;
                if(tau_trk_link.isValid()) {
                    auto tau_trk = (*tau_trk_link)->track();
                    auto where = std::find_if(tracks_and_leps.cbegin(), tracks_and_leps.cend(),
                        [&](auto track_and_lep){ return tau_trk == track_and_lep.first; });
                    if(where != tracks_and_leps.cend()) {
                        ATH_MSG_DEBUG("track with pt " << tau_trk->pt()/GeV << " GeV removed");
                        ret.push_back(where->second);
                        match = true;
                    }
                }
                return match;
            }
        ),
        tau_trk_links.end()
    );
    return ret;
}

template<typename Tlep, typename Tlinks> std::vector<Tlep> TauAODMuonRemovalTool::removeClss(Tlinks& tau_cls_links, std::vector<std::pair<const xAOD::CaloCluster*, Tlep>>& clusters_and_leps) const {
    std::vector<Tlep> ret;
    tau_cls_links.erase(
        std::remove_if(tau_cls_links.begin(), tau_cls_links.end(),
            [&](auto tau_cls_link) -> bool {
                bool match = false;
                if(tau_cls_link.isValid()) {
                    auto tau_cls = static_cast<const xAOD::CaloCluster*>(*tau_cls_link);
                    auto where = std::find_if(clusters_and_leps.cbegin(), clusters_and_leps.cend(),
                        [&](auto cluster_and_lep){ return tau_cls == cluster_and_lep.first; });
                    if(where != clusters_and_leps.cend()) {
                        ATH_MSG_DEBUG("cluster with pt " << tau_cls->pt()/GeV << " GeV removed");
                        ret.push_back(where->second);
                        match = true;
                    }
                }
                return match;
            }
        ),
        tau_cls_links.end()
    );
    return ret;
}
