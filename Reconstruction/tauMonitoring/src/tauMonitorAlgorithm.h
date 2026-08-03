/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAUMONITORALGORITHM_H
#define TAUMONITORALGORITHM_H

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"

#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadDecorHandle.h"

#include "xAODTau/TauJetContainer.h"
#include "xAODTau/TauTrackContainer.h"


class tauMonitorAlgorithm : public AthMonitorAlgorithm {
public:
    tauMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator );
    virtual ~tauMonitorAlgorithm();
    virtual StatusCode initialize() override;
    virtual StatusCode fillHistograms( const EventContext& ctx ) const override;
private:
    std::vector<int> m_abGroups1;
    std::vector<std::vector<int>> m_abGroups2;
    std::map<std::string,int> m_cGroups1;
    std::map<std::string,std::map<std::string,int>> m_cGroups2;

    SG::ReadHandleKey<xAOD::TauJetContainer> m_TauContainerKey {this, "TauRecContainer", "TauJets"};
    SG::ReadHandleKey<xAOD::TauTrackContainer> m_TauTrackContainer{ this, "TauTrackContainer", "TauTracks"};
    SG::ReadDecorHandleKey<xAOD::TauJetContainer> m_offlineGNTauScoreDecorKey{this, "OfflineGNTauScoreDecorKey", "GNTauScore_v0prune", "Offline GNTau Score decoration key"};
    SG::ReadDecorHandleKey<xAOD::TauJetContainer> m_offlineGNTauTransScoreDecorKey{this, "OfflineGNTauTransScoreDecorKey", "GNTauScoreSigTrans_v0prune", "Offline GNTau Trans Score decoration key"};
    SG::ReadDecorHandleKey<xAOD::TauJetContainer> m_offlineGNTauLooseWPDecorKey{this, "OfflineGNTauLooseWPDecorKey", "GNTauL_v0prune", "Offline GNTau Loose WP decoration key"};
    SG::ReadDecorHandleKey<xAOD::TauJetContainer> m_offlineGNTauMediumWPDecorKey{this, "OfflineGNTauMediumWPDecorKey", "GNTauM_v0prune", "Offline GNTau Medium WP decoration key"};
    SG::ReadDecorHandleKey<xAOD::TauJetContainer> m_offlineGNTauTightWPDecorKey{this, "OfflineGNTauTightWPDecorKey", "GNTauT_v0prune", "Offline GNTau Tight WP decoration key"};
    SG::ReadDecorHandleKey<xAOD::TauJetContainer> m_passThinningDecorKey{this, "PassThinningDecorKey", "passThinning", "passThinning decoration key"};
    SG::ReadDecorHandleKey<xAOD::TauJetContainer> m_EMFracFixedDecorKey{this, "EMFracFixedDecorKey", "EMFracFixed", "EMFracFixed decoration key"};
    SG::ReadDecorHandleKey<xAOD::TauTrackContainer> m_d0SigTJVADecorKey{this, "d0SigTJVADecorKey", "d0SigTJVA", "d0SigTJVA decoration key"};
    SG::ReadDecorHandleKey<xAOD::TauTrackContainer> m_z0sinthetaSigTJVADecorKey{this, "z0sinthetaSigTJVADecorKey", "z0sinthetaSigTJVA", "z0sinthetaSigTJVA decoration key"};
    SG::ReadDecorHandleKey<xAOD::TauTrackContainer> m_rnn_chargedScoreDecorKey{this, "rnn_chargedScoreDecorKey",  "rnn_chargedScore", "rnn_chargedScore decoration key"};
    SG::ReadDecorHandleKey<xAOD::TauTrackContainer> m_rnn_isolationScoreDecorKey{this, "rnn_isolationScoreDecorKey", "rnn_isolationScore", "rnn_isolationScore decoration key"};
    SG::ReadDecorHandleKey<xAOD::TauTrackContainer> m_rnn_conversionScoreDecorKey{this, "rnn_conversionScoreDecorKey", "rnn_conversionScore", "rnn_conversionScore decoration key"};
    SG::ReadDecorHandleKey<xAOD::TauTrackContainer> m_z0sinthetaTJVADecorKey{this, "z0sinthetaTJVADecorKey", "z0sinthetaTJVA", "z0sinthetaTJVA decoration key"};
    SG::ReadDecorHandleKey<xAOD::TauTrackContainer> m_eProbabilityNNDecorKey{this, "eProbabilityNNDecorKey", "eProbabilityNN", "eProbabilityNN decoration key"};

    Gaudi::Property<float> m_etaMin {this, "etaMin", -1.};
    Gaudi::Property<float> m_etaMax {this, "etaMax", 3.0};

    Gaudi::Property<std::string> m_kinGroupName {this, "kinGroupName", "tauMonKinGroupBA"};

};
#endif
