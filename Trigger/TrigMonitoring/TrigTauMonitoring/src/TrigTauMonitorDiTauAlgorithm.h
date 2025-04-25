/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGTAUMONITORING_TRIGTAUMONITORDITAUALGORITHM_H
#define TRIGTAUMONITORING_TRIGTAUMONITORDITAUALGORITHM_H

#include "TrigTauMonitorBaseAlgorithm.h"

class TrigTauMonitorDiTauAlgorithm : public TrigTauMonitorBaseAlgorithm {
public:
    TrigTauMonitorDiTauAlgorithm(const std::string& name, ISvcLocator* pSvcLocator);

private:
    // Enable total efficiency histograms
    // Note: Should only be used when reprocessing EB or MC data. Comparisons of total efficiencies between chains on normal data-taking 
    // conditions would be meaningless, since different L1/HLT items can have different prescales, and are not within a Coherent-Prescale-Set
    Gaudi::Property<bool> m_doTotalEfficiency{this, "DoTotalEfficiency", false, "Do total efficiency histograms"};

    // Require at least 1 offline Tau per event (will bias the variable distributions for background events)
    Gaudi::Property<bool> m_requireOfflineTaus{this, "RequireOfflineTaus", true, "Require at leat 1 offline tau per event"};
    Gaudi::Property<unsigned int> m_offline_tau_id{this, "OfflineTauID", TauID::RNN, "Offline TauID (1: RNN, 2: GNTau)"};

    virtual StatusCode processEvent(const EventContext& ctx) const override;

    void fillDiTauHLTEfficiencies(const EventContext& ctx,const std::string& trigger, const bool l1_accept_flag, const std::vector<const xAOD::TauJet*>& offline_tau_vec, const std::vector<const xAOD::TauJet*>& online_tau_vec) const;
    void fillDiTauVars(const std::string& trigger, const std::vector<const xAOD::TauJet*>& tau_vec) const;
};

#endif
