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
    SG::ReadDecorHandleKey<xAOD::TauJetContainer> m_offlineGNTauScoreDecorKey{this, "OfflineGNTauScoreDecorKey", "GNTauScore_v0prune", "Offline GNTau Score decoration key"};
    SG::ReadDecorHandleKey<xAOD::TauJetContainer> m_offlineGNTauTransScoreDecorKey{this, "OfflineGNTauTransScoreDecorKey", "GNTauScoreSigTrans_v0prune", "Offline GNTau Trans Score decoration key"};
    SG::ReadDecorHandleKey<xAOD::TauJetContainer> m_offlineGNTauLooseWPDecorKey{this, "OfflineGNTauLooseWPDecorKey", "GNTauL_v0prune", "Offline GNTau Loose WP decoration key"};
    SG::ReadDecorHandleKey<xAOD::TauJetContainer> m_offlineGNTauMediumWPDecorKey{this, "OfflineGNTauMediumWPDecorKey", "GNTauM_v0prune", "Offline GNTau Medium WP decoration key"};
    SG::ReadDecorHandleKey<xAOD::TauJetContainer> m_offlineGNTauTightWPDecorKey{this, "OfflineGNTauTightWPDecorKey", "GNTauT_v0prune", "Offline GNTau Tight WP decoration key"};

    Gaudi::Property<float> m_etaMin {this, "etaMin", -1.};
    Gaudi::Property<float> m_etaMax {this, "etaMax", 3.0};

    Gaudi::Property<std::string> m_kinGroupName {this, "kinGroupName", "tauMonKinGroupBA"};

};
#endif
