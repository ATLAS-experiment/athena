/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGTAUMONITORING_TRIGTAUMONITORBOOSTEDDITAUALGORITHM_H
#define TRIGTAUMONITORING_TRIGTAUMONITORBOOSTEDDITAUALGORITHM_H

#include "xAODTau/DiTauJetContainer.h"

#include "TrigTauMonitorBaseAlgorithm.h"

class TrigTauMonitorBoostedDiTauAlgorithm : public TrigTauMonitorBaseAlgorithm {
public:
    TrigTauMonitorBoostedDiTauAlgorithm(const std::string& name, ISvcLocator* pSvcLocator);
    virtual StatusCode initialize() override;

private:
    virtual StatusCode processEvent(const EventContext& ctx) const override;
    std::vector<const xAOD::DiTauJet*> getOnlineBoostedDiTausAll(const std::string& trigger) const;

    void fillBoostedDiTauVars(const std::string& trigger, const std::vector<const xAOD::DiTauJet*>& tau_vec) const;
    const SG::ReadHandleKey<xAOD::DiTauJetContainer>& getOnlineBoostedDiTauContainerKey(const std::string& trigger) const;

    SG::ReadHandleKey<xAOD::DiTauJetContainer> m_hltBoostedDiTauJetKey{this, "HLTBoostedDiTauJetKey", "HLT_DiTauJets" , "HLT boosted ditau container key"};
};

#endif
