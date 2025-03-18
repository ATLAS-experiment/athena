/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGTAUMONITORING_TRIGTAUMONITORSINGLEALGORITHM_H
#define TRIGTAUMONITORING_TRIGTAUMONITORSINGLEALGORITHM_H

#include "Gaudi/Parsers/Factory.h"

#include "TrigTauMonitorBaseAlgorithm.h"

class TrigTauMonitorSingleAlgorithm : public TrigTauMonitorBaseAlgorithm {
public:
    TrigTauMonitorSingleAlgorithm(const std::string& name, ISvcLocator* pSvcLocator);
    virtual StatusCode initialize() override;

private:
    // Enable total efficiency histograms
    // Note: Should only be used when reprocessing EB or MC data. Comparisons of total efficiencies between chains on normal data-taking 
    // conditions would be meaningless, since different L1/HLT items can have different prescales, and are not within a Coherent-Prescale-Set
    Gaudi::Property<bool> m_doTotalEfficiency{this, "DoTotalEfficiency", false, "Do total efficiency histograms"};

    // Require at least 1 offline Tau per event (will bias the variable distributions for background events)
    Gaudi::Property<bool> m_requireOfflineTaus{this, "RequireOfflineTaus", true, "Require at leat 1 offline tau per event"};
    
    // Do offline taus variable distributions
    Gaudi::Property<bool> m_doOfflineTausDistributions{this, "DoOfflineTausDistributions", true};

    // HLT TauID score monitoring
    Gaudi::Property<std::map<std::string, std::map<std::string, std::pair<std::string, std::string>>>> m_monitoredHLTIdScores {this, "HLTTauIDScores", {}, "Pairs of the TauID score and signal-transformed scores for each HLT TauID algorithm to be monitored, for each reconstruction sequence (type, e.g. tracktwoMVA, tracktwoLLP, etc...)"};
    std::map<std::string, std::map<std::string, std::pair<SG::ConstAccessor<float>, SG::ConstAccessor<float>>>> m_monitoredHLTIdAccessors;

    // Offline TauID score monitoring
    Gaudi::Property<std::map<std::string, std::pair<std::string, std::string>>> m_monitoredOfflineIdScores {this, "OfflineTauIDScores", {}, "Pairs of the TauID score and signal-transformed scores for each Offline TauID algorithm to be monitored"};
    std::map<std::string, std::pair<SG::ConstAccessor<float>, SG::ConstAccessor<float>>> m_monitoredOfflineIdAccessors;

    virtual StatusCode processEvent(const EventContext& ctx) const override;

    void fillHLTEfficiencies(const EventContext& ctx,const std::string& trigger, const bool l1_accept_flag, const std::vector<const xAOD::TauJet*>& offline_tau_vec, const std::vector<const xAOD::TauJet*>& online_tau_vec, const std::string& nProng) const;
    void fillIDInputVars(const std::string& trigger, const std::vector<const xAOD::TauJet*>& tau_vec,const std::string& nProng, bool online) const;
    void fillIDTrack(const std::string& trigger, const std::vector<const xAOD::TauJet*>& tau_vec, bool online) const;
    void fillIDCluster(const std::string& trigger, const std::vector<const xAOD::TauJet*>& tau_vec, bool online) const;
    void fillBasicVars(const EventContext& ctx, const std::string& trigger, const std::vector<const xAOD::TauJet*>& tau_vec, const std::string& nProng, bool online) const;
    void fillIDScores(const std::string& trigger, const std::vector<const xAOD::TauJet*>& tau_vec, const std::string& nProng, bool online) const;

    std::vector<TLorentzVector> getRoIsVector(const EventContext& ctx, const std::string& trigger) const;
};

#endif
