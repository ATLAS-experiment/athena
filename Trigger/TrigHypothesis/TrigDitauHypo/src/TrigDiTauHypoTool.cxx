/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "AthenaMonitoringKernel/Monitored.h"
#include "TrigCompositeUtils/TrigCompositeUtils.h"
#include "xAODTau/TauJetContainer.h"
#include "GaudiKernel/SystemOfUnits.h"

#include "TrigDiTauHypoTool.h"


using namespace TrigCompositeUtils;

TrigDiTauHypoTool::TrigDiTauHypoTool(const std::string& type, const std::string& name, const IInterface* parent)
  : base_class(type, name, parent), 
    m_decisionId(HLT::Identifier::fromToolName(name))
{}


TrigDiTauHypoTool::~TrigDiTauHypoTool()
{}


StatusCode TrigDiTauHypoTool::initialize()
{
    ATH_MSG_DEBUG(name() << ": in initialize()");

    ATH_MSG_DEBUG("TrigDiTauHypoTool will cut on:");
    ATH_MSG_DEBUG(" - PtMin       : " << m_ditau_pt_threshold.value());
    ATH_MSG_DEBUG(" - ID Score    : " << m_ditau_omni_score.value());
    ATH_MSG_DEBUG(" - Max Lead Trk: " << m_ditau_lead_max_trk.value());
    ATH_MSG_DEBUG(" - Max Subl Trk: " << m_ditau_subl_max_trk.value());
    return StatusCode::SUCCESS;
}


bool TrigDiTauHypoTool::decide(const ITrigDiTauHypoTool::ToolInfo& input) const
{
    static const SG::ConstAccessor<     int > n_tracks_leadAcc("n_tracks_lead");
    static const SG::ConstAccessor<     int > n_tracks_sublAcc("n_tracks_subl");
    static const SG::ConstAccessor<   float > ditau_ptAcc     ("ditau_pt");
    static const SG::ConstAccessor<   float > omni_scoreAcc   ("omni_score");
    ATH_MSG_DEBUG(name() << ": in execute()");
    // Tau pass flag
    bool pass = false;
    for(const xAOD::DiTauJet* diTau : *(input.diTauContainer)) {
        if(ditau_ptAcc(*diTau) < m_ditau_pt_threshold.value()) continue;
        if(omni_scoreAcc(*diTau) < m_ditau_omni_score.value()) continue;
        if(n_tracks_leadAcc(*diTau) > m_ditau_lead_max_trk.value() || n_tracks_leadAcc(*diTau) < 0) continue;
        if(n_tracks_sublAcc(*diTau) > m_ditau_subl_max_trk.value() || n_tracks_sublAcc(*diTau) < 0) continue;
        pass = true;
    }
    ATH_MSG_DEBUG(" Pass hypo tool: " << pass);
    return pass;
}


StatusCode TrigDiTauHypoTool::decide(std::vector<ITrigDiTauHypoTool::ToolInfo>& input) const {
    for(auto& i : input) {
        if(passed(m_decisionId.numeric(), i.previousDecisionIDs)) {
            if(decide(i)) {
	            addDecisionID(m_decisionId, i.decision);
            }
        }
    }

    return StatusCode::SUCCESS;
}

