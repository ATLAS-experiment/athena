/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGTAUMONITORING_TRIGTAUMONITORTANDPALGORITHM_H
#define TRIGTAUMONITORING_TRIGTAUMONITORTANDPALGORITHM_H

#include "TrigTauMonitorBaseAlgorithm.h"

class TrigTauMonitorTandPAlgorithm : public TrigTauMonitorBaseAlgorithm {
public:
    TrigTauMonitorTandPAlgorithm(const std::string& name, ISvcLocator* pSvcLocator);
    virtual StatusCode initialize() override;

private:
    // Require at least 1 offline Tau per event (will bias the variable distributions for background events)
    Gaudi::Property<bool> m_requireOfflineTaus{this, "RequireOfflineTaus", true, "Require at leat 1 offline tau per event"};
    Gaudi::Property<unsigned int> m_offline_tau_id{this, "OfflineTauID", TauID::RNN, "Offline TauID (1: RNN, 2: GNTau)"};

    // Get offline electrons that pass the quality selection cuts
    std::vector<const xAOD::Electron*> getOfflineElectrons(const EventContext& ctx, const float threshold = 0.0) const;

    // Get offline muons that pass the quality selection cuts
    std::vector<const xAOD::Muon*> getOfflineMuons(const EventContext& ctx, const float threshold = 0.0) const;

    // Get online electrons
    std::vector<const xAOD::Electron*> getOnlineElectrons(const std::string& trigger) const;

    // Get online muons
    std::vector<const xAOD::Muon*> getOnlineMuons(const std::string& trigger) const;

    virtual StatusCode processEvent(const EventContext& ctx) const override;

    void fillTAndPHLTEfficiencies(const EventContext& ctx, const std::string& trigger, const std::vector<const xAOD::IParticle*>& offline_lep_vec, const std::vector<const xAOD::IParticle*>& online_lep_vec, const std::vector<const xAOD::TauJet*>& offline_tau_vec, const std::vector<const xAOD::TauJet*>& online_tau_vec) const;
    void fillTagAndProbeVars(const std::string& trigger, const std::vector<const xAOD::TauJet*>& tau_vec, const std::vector<const xAOD::IParticle*>& lep_vec) const;

    // StorageGate keys
    SG::ReadHandleKey<xAOD::ElectronContainer> m_offlineElectronKey{this, "offlineElectronKey", "Electrons", "Offline Electron key for tau-e chains"};
    SG::ReadHandleKey<xAOD::MuonContainer> m_offlineMuonKey{this, "offlineMuonKey", "Muons", "Offline Muon key for tau-mu chains"};

    SG::ReadHandleKey<xAOD::ElectronContainer> m_hltElectronKey{this, "hltElectronKey", "HLT_egamma_Electrons_GSF", "HLT Electrons key for tau-e chains"};
    SG::ReadHandleKey<xAOD::MuonContainer> m_hltMuonKey{this, "hltMuonKey", "HLT_MuonsIso", "HLT Muons key for tau-mu chains"};
};

#endif
