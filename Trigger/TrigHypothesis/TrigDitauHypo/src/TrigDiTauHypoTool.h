/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#pragma once

#include "TrigCompositeUtils/HLTIdentifier.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "TrigCompositeUtils/TrigCompositeUtils.h"
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"

#include "xAODTau/DiTauJetContainer.h"
#include "ITrigDiTauHypoTool.h"

class TrigDiTauHypoTool: public extends<AthAlgTool, ITrigDiTauHypoTool>  {
    public:
        TrigDiTauHypoTool(const std::string& type, const std::string& name, const IInterface* parent);
        virtual ~TrigDiTauHypoTool();
        virtual StatusCode initialize() override;
        virtual StatusCode decide(std::vector<ITrigDiTauHypoTool::ToolInfo>& input) const override;
        virtual bool decide(const ITrigDiTauHypoTool::ToolInfo& i) const override;
    private:
        HLT::Identifier m_decisionId;
        Gaudi::Property<float> m_ditau_pt_threshold {this, "ditau_pt_threshold", 200, "ditau pT threshold [GeV]" };
        Gaudi::Property<float> m_ditau_omni_score   {this, "ditau_id_score",     0.0, "ditau omni id score" };
        Gaudi::Property<int>   m_ditau_lead_max_trk {this, "ditau_lead_max_trk", 3,   "ditau lead track multiplicity" };
        Gaudi::Property<int>   m_ditau_subl_max_trk {this, "ditau_subl_max_trk", 3,   "ditau sublead track multiplicity" };
};

