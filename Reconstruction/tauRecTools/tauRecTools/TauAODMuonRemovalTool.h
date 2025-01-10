/*
    Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 *  @brief the tool meant to run at AOD level, to remove the muon found inside the TauJet.
 */

#ifndef TAURECTOOLS_TAUAODMUONREMOVALTOOL_H
#define TAURECTOOLS_TAUAODMUONREMOVALTOOL_H

#include "tauRecTools/TauRecToolBase.h"
#include "xAODTau/TauJet.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "AsgDataHandles/ReadHandle.h"
#include "AsgDataHandles/ReadHandleKey.h"
#include "AsgTools/PropertyWrapper.h"

class TauAODMuonRemovalTool : public TauRecToolBase 
{
    public:
        ASG_TOOL_CLASS2( TauAODMuonRemovalTool, TauRecToolBase, ITauToolBase )
        TauAODMuonRemovalTool(const std::string& type);
        virtual StatusCode initialize() override;
        virtual StatusCode execute(xAOD::TauJet&) const override;
    private:
        const std::map<std::string, uint>           m_mapMuonIdWp        = {{"Tight", 0}, {"Medium", 1}, {"Loose", 2}, {"VeryLoose",3}};
        uint                                        m_muonWpUi           = 1;
        SG::ReadHandleKey<xAOD::MuonContainer>      m_muonInputContainer{this, "Key_MuonInputContainer", "Muons",     "input xAOD muons"};
        //properties
        Gaudi::Property<bool>        m_doMuonTrkRm       {this, "doMuonTrkRm",        false,                 "Whether to remove the muon tracks from the tau candidate"             };
        Gaudi::Property<bool>        m_doMuonClsRm       {this, "doMuonClsRm",        false,                 "Whether to remove the muon clusters from the tau candidate"           };
        Gaudi::Property<std::string> m_strMinMuonIdWp    {this, "muonIDWP",           "Medium",              "minimum muon identification WP, [VeryLoose, Loose, Medium, Tight]"    };
        Gaudi::Property<double>      m_lepRemovalConeSize{this, "lepRemovalConeSize", 0.6,                   "The maximum dR between the lepton and the tau"                        };
        //helpers
        std::vector<const xAOD::CaloCluster*>                                       getOriginalTopoClusters (const xAOD::CaloCluster   *cluster) const;
        std::vector<std::pair<const xAOD::TrackParticle*,     const xAOD::Muon*>>   getMuonAndTrk(const xAOD::TauJet& tau,  const xAOD::MuonContainer& muon_cont) const;
        std::vector<std::pair<const xAOD::CaloCluster*,       const xAOD::Muon*>>   getMuonAndCls(const xAOD::TauJet& tau,  const xAOD::MuonContainer& muon_cont) const;
        template<typename Tlep, typename Tlinks> std::vector<Tlep>                  removeTrks(Tlinks& tau_trk_links,   std::vector<std::pair<const xAOD::TrackParticle*, Tlep>>& removings) const;
        template<typename Tlep, typename Tlinks> std::vector<Tlep>                  removeClss(Tlinks& tau_cls_links,   std::vector<std::pair<const xAOD::CaloCluster*, Tlep>>& clusters_and_leps) const;
};

#endif// TAURECTOOLS_TAUAODMUONREMOVALTOOL_H
