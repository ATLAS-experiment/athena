/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGTAUMONITORING_TRIGTAUMONITORSINGLEALGORITHM_H
#define TRIGTAUMONITORING_TRIGTAUMONITORSINGLEALGORITHM_H

#include "Gaudi/Parsers/Factory.h"

#include "TrigTauMonitorBaseAlgorithm.h"

class TrigTauMonitorSingleAlgorithm : public TrigTauMonitorBaseAlgorithm {
public:
    TrigTauMonitorSingleAlgorithm(const std::string& name, ISvcLocator* pSvcLocator);
    virtual StatusCode initialize() override;

    using VarPropertyMap = std::map<std::string, std::map<std::string, std::pair<std::string, std::string>>>;
    using VarKeysMap = std::map<std::string, std::map<std::string, std::pair<SG::ReadDecorHandleKey<xAOD::TauJetContainer>, SG::ReadDecorHandleKey<xAOD::TauJetContainer>>>>;

private:
    // Enable total efficiency histograms
    // Note: Should only be used when reprocessing EB or MC data. Comparisons of total efficiencies between chains on normal data-taking 
    // conditions would be meaningless, since different L1/HLT items can have different prescales, and are not within a Coherent-Prescale-Set
    Gaudi::Property<bool> m_doTotalEfficiency{this, "DoTotalEfficiency", false, "Do total efficiency histograms"};

    // Require at least 1 offline Tau per event (will bias the variable distributions for background events)
    Gaudi::Property<bool> m_requireOfflineTaus{this, "RequireOfflineTaus", true, "Require at leat 1 offline tau per event"};
    Gaudi::Property<unsigned int> m_offline_tau_id{this, "OfflineTauID", TauID::RNN, "Offline TauID (1: RNN, 2: GNTau)"};
    
    // Do offline taus variable distributions
    Gaudi::Property<bool> m_doOfflineTausDistributions{this, "DoOfflineTausDistributions", true};

    // HLT TauID/HitZ monitoring
    Gaudi::Property<VarPropertyMap> m_monitoredHLTIdScores {this, "HLTTauIDScores", {}, "Pairs of the TauID score and signal-transformed scores for each HLT TauID algorithm to be monitored, for each Tau container suffix (type, e.g. MVA, LLP, etc...)"};
    Gaudi::Property<VarPropertyMap> m_monitoredHLTCaloHitsPreselIdScores {this, "HLTTauCaloHitsPreselIDScores", {}, "Pairs of the Calo+Hits preselection TauID score and signal-transformed scores for each HLT TauID algorithm to be monitored, for each Tau container suffix (type, e.g. MVA_CaloHitsBase, MVA_HitZ, etc...)"};
    Gaudi::Property<VarPropertyMap> m_monitoredHLTHitZVars {this, "HLTTauHitZVars", {}, "Pairs of the HitZ z0 and z0 sigma for each HLT HitZ algorithm to be monitored, for each Tau container suffix (type, e.g. MVA_HitZ, etc...)"};

    VarKeysMap m_monitoredVarPairsDecorHandleKeys;

    StatusCode createKeys(const VarPropertyMap& var_names_map);

    // Offline TauID score monitoring
    Gaudi::Property<std::map<std::string, std::pair<std::string, std::string>>> m_monitoredOfflineIdScores {this, "OfflineTauIDScores", {}, "Pairs of the TauID score and signal-transformed scores for each Offline TauID algorithm to be monitored"};

    virtual StatusCode processEvent(const EventContext& ctx) const override;

    void fillHLTEfficiencies(const EventContext& ctx, const std::string& trigger, const bool l1_accept_flag, const std::vector<const xAOD::TauJet*>& offline_tau_vec, const std::vector<const xAOD::TauJet*>& online_tau_vec, const std::string& nProng) const;
    void fillIDInputVars(const std::string& trigger, const std::vector<const xAOD::TauJet*>& tau_vec,const std::string& nProng, bool online) const;
    void fillIDTrack(const std::string& trigger, const std::vector<const xAOD::TauJet*>& tau_vec, bool online) const;
    void fillIDCluster(const std::string& trigger, const std::vector<const xAOD::TauJet*>& tau_vec, bool online) const;
    void fillBasicVars(const EventContext& ctx, const std::string& trigger, const std::vector<const xAOD::TauJet*>& tau_vec, const std::string& nProng, bool online) const;
    void fillIDScores(const EventContext& ctx, const std::string& trigger, const std::vector<const xAOD::TauJet*>& tau_vec, const std::string& nProng, bool online) const;
    void fillHitZVars(const EventContext& ctx, const std::string& trigger, const std::vector<const xAOD::TauJet*>& tau_vec, const std::string& nProng) const;

    void fillVarPairs(const EventContext& ctx, const ToolHandle<GenericMonitoringTool>& mon_group, const VarPropertyMap::mapped_type& vars, const std::string& category, const std::string& match_var_name, const std::string& mon_var_1_name, const std::string& mon_var_2_name, const std::vector<const xAOD::TauJet*>& tau_vec) const;

    std::vector<TLorentzVector> getRoIsVector(const EventContext& ctx, const std::string& trigger) const;
};

#endif
