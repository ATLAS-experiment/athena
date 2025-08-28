/*
 Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

// Local include(s):
#include "MuonScaleFactorTestAlg.h"
#include "StoreGate/ReadHandle.h"
#include "MuonTesterTree/EventInfoBranch.h"
#include "PATInterfaces/SystematicsTool.h"
#include <cmath>

namespace{
    constexpr double MeVtoGeV = 1.e-3;
}

namespace CP {
    MuonScaleFactorTestAlg::MuonScaleFactorTestAlg(const std::string& name, ISvcLocator* svcLoc) :
        AthHistogramAlgorithm(name, svcLoc) {
        // force strict checking of return codes
        CP::CorrectionCode::enableFailure();
    }

    StatusCode MuonScaleFactorTestAlg::initialize() {
        ATH_CHECK(m_eventInfo.initialize());
        ATH_CHECK(m_sgKey.initialize());
        ATH_CHECK(m_prw_Tool.retrieve());
        ATH_CHECK(m_sel_tool.retrieve());

        ATH_MSG_DEBUG("PileupReweightingTool  = " << m_prw_Tool);
        ATH_CHECK(m_effiTools.retrieve());
        ATH_CHECK(m_comparisonTools.retrieve());
        for (auto& tool : m_effiTools) {
            auto sfBranch = std::make_shared<TestMuonSF::MuonSFBranches>(m_tree,tool,
                                                                         m_comparisonTools.empty() ? "" : m_defaultRelease.value());
            
            m_sfBranches.push_back(sfBranch);
            m_tree.addBranch(sfBranch);

            auto replicaBranch = std::make_shared<TestMuonSF::MuonReplicaBranches>(m_tree, tool,
                                                                                   m_comparisonTools.empty() ? "" : m_defaultRelease.value());

            m_sfBranches.push_back(replicaBranch);
            m_tree.addBranch(replicaBranch);        
        }
        for (auto& tool : m_comparisonTools) {
            auto sfBranch = std::make_shared<TestMuonSF::MuonSFBranches>(m_tree, tool, m_validRelease);
            m_sfBranches.push_back(sfBranch);
            m_tree.addBranch(sfBranch);

            auto replicaBranch = std::make_shared<TestMuonSF::MuonReplicaBranches>(m_tree,tool, m_validRelease);
            m_sfBranches.push_back(replicaBranch);
            m_tree.addBranch(replicaBranch);        
        }
        m_tree.addBranch(std::make_shared<MuonVal::EventInfoBranch>(m_tree, 0));
        ATH_CHECK(m_tree.init(this));
        return StatusCode::SUCCESS;
    }
    StatusCode MuonScaleFactorTestAlg::finalize() {
        ATH_CHECK(m_tree.write());
        return StatusCode::SUCCESS;
    }

    StatusCode MuonScaleFactorTestAlg::execute() {
        const EventContext& ctx{Gaudi::Hive::currentContext()};        
        // Retrieve the muons:
        SG::ReadHandle<xAOD::MuonContainer> muons{m_sgKey, ctx};       
        // Retrieve the EventInfo:
        SG::ReadHandle<xAOD::EventInfo> ei{m_eventInfo, ctx};
       
        ATH_MSG_DEBUG("Start to run over event "<<ei->eventNumber()<<" in run" <<ei->runNumber());
       
        //Apply the prwTool first before calling the efficiency correction methods
        ATH_CHECK(m_prw_Tool->apply(*ei));       
        
        for (const xAOD::Muon* mu : *muons) {
            if (mu->pt() < m_pt_cut || (m_eta_cut > 0 && std::abs(mu->eta()) >= m_eta_cut)) continue;
            // reject all loose muons
            if (m_sel_tool->getQuality(*mu) > m_muon_quality) continue;          
            
            m_muonPt = mu->pt() * MeVtoGeV;
            m_muonEta = mu->eta();
            m_muonPhi = mu->phi();
            m_muonQ = mu->charge();
            for (auto& br : m_sfBranches) {
                br->setMuon(*mu);
            }
            ATH_CHECK(m_tree.fill(ctx));          
        }
        ATH_MSG_DEBUG("Done with processing the event");
        // Return gracefully:
        return StatusCode::SUCCESS;
    }

} // namespace CP
