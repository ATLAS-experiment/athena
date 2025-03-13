/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGTAUMONITORING_TRIGTAUMONITORBASEALGORITHM_H
#define TRIGTAUMONITORING_TRIGTAUMONITORBASEALGORITHM_H

#include "xAODTau/TauJetContainer.h"

#include "xAODTrigger/eFexTauRoIContainer.h"
#include "xAODTrigger/jFexTauRoIContainer.h"

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"

#include "StoreGate/ReadDecorHandleKey.h"

#include "Gaudi/Parsers/Factory.h" // Needed to declare less common Property types
#include "GaudiKernel/SystemOfUnits.h"
#include "CxxUtils/phihelper.h"

#include "TrigTauMonitoring/TrigTauInfo.h"

class TrigTauMonitorBaseAlgorithm : public AthMonitorAlgorithm {
public:
    TrigTauMonitorBaseAlgorithm(const std::string& name, ISvcLocator* pSvcLocator);
    virtual StatusCode initialize() override;
    virtual StatusCode fillHistograms(const EventContext& ctx) const override;

protected:
    // List of triggers from menu (before duplicate filtering)
    Gaudi::Property<std::vector<std::string>> m_triggers{this, "TriggerList", {}};

    // Phase1 L1 tau threshold information
    Gaudi::Property<std::map<std::string, float>> m_L1_Phase1_thresholds{this, "L1Phase1Thresholds", {}};
    Gaudi::Property<std::map<std::string, uint64_t>> m_L1_Phase1_threshold_patterns{this, "L1Phase1ThresholdPatterns", {}};
    Gaudi::Property<bool> m_L1_select_by_et_only{this, "SelectL1ByETOnly", false};

    // Toggle efficiency and variable plots
    Gaudi::Property<bool> m_do_efficiency_plots{this, "DoEfficiencyPlots", true};
    Gaudi::Property<bool> m_do_variable_plots{this, "DoVariablePlots", true};

    // TrigTauInfo objects, containing all information from each trigger
    inline std::map<std::string, TrigTauInfo>& getTrigInfoMap() { return m_trigInfo; }
    inline const TrigTauInfo& getTrigInfo(const std::string& trigger) const { return m_trigInfo.at(trigger); }

    // Get online 0P, 1P and MP TauJets
    std::vector<const xAOD::TauJet*> getOnlineTausAll(const std::string& trigger, bool include_0P=true) const;
    std::tuple<std::vector<const xAOD::TauJet*>, std::vector<const xAOD::TauJet*>, std::vector<const xAOD::TauJet*>> getOnlineTaus(const std::string& trigger) const;

    // Get offline 1P and 3P TauJet objects that pass the quality selection cuts
    std::vector<const xAOD::TauJet*> getOfflineTausAll(const EventContext& ctx, const float threshold = 20.0) const;
    std::pair<std::vector<const xAOD::TauJet*>, std::vector<const xAOD::TauJet*>> getOfflineTaus(const EventContext& ctx, const float threshold = 20.0) const;

    // Get L1 RoIs
    std::vector<const xAOD::eFexTauRoI*> getL1eTAUs(const EventContext& ctx, const std::string& l1_item) const;
    std::vector<const xAOD::jFexTauRoI*> getL1jTAUs(const EventContext& ctx, const std::string& l1_item) const;
    std::vector<std::pair<const xAOD::eFexTauRoI*, const xAOD::jFexTauRoI*>> getL1cTAUs(const EventContext& ctx, const std::string& l1_item) const;

    // Process event, after bad event cleaning
    virtual StatusCode processEvent(const EventContext& ctx) const = 0;

    // Helper functions
    inline double dR(const double eta1, const double phi1, const double eta2, const double phi2) const
    {
        double deta = std::fabs(eta1 - eta2);
        double dphi = std::fabs(CxxUtils::wrapToPi(phi1-phi2));
        return std::sqrt(deta*deta + dphi*dphi);
    }

    template <typename T1 = xAOD::IParticle, typename T2 = xAOD::IParticle>
    inline bool matchObjects(const T1* tau, const std::vector<const T2*>& tau_vec, float threshold) const
    {
        for(auto tau_2 : tau_vec) {
            if(tau->p4().DeltaR(tau_2->p4()) < threshold) return true;
        }
        return false;
    }

    inline bool matchObjects(const TLorentzVector& tau, const std::vector<TLorentzVector>& tau_vec, float threshold) const
    {
        for(auto& tau_2 : tau_vec) {
            if(tau.DeltaR(tau_2) < threshold) return true;
        }
        return false;
    }

    template <typename T1 = xAOD::IParticle, typename T2 = xAOD::eFexTauRoI>
    inline bool matchObjects(const T1* tau_1, const T2* tau_2, float threshold) const
    {
        return dR(tau_1->eta(), tau_1->phi(), tau_2->eta(), tau_2->phi()) < threshold;
    }

    template <typename T1 = xAOD::IParticle, typename T2 = xAOD::IParticle>
    inline bool matchTruthObjects(const T1* true_tau, const std::vector<const T2*>& tau_vec, float threshold) const
    {
      static const SG::ConstAccessor<double> acc_ptvis("pt_vis");
      static const SG::ConstAccessor<double> acc_etavis("eta_vis");
      static const SG::ConstAccessor<double> acc_phivis("phi_vis");
      static const SG::ConstAccessor<double> acc_mvis("mvis");
      TLorentzVector true_tau_p4;
      true_tau_p4.SetPtEtaPhiM(acc_ptvis(*true_tau), acc_etavis(*true_tau), acc_phivis(*true_tau), acc_mvis(*true_tau));

      for(auto tau : tau_vec) {
	if(true_tau_p4.DeltaR(tau->p4()) < threshold) return true;
      }
      return false;
    }

    std::vector<const xAOD::TauJet*> classifyTausAll(const std::vector<const xAOD::TauJet*>& taus, const float threshold = 0.0) const;
    std::pair<std::vector<const xAOD::TauJet*>, std::vector<const xAOD::TauJet*>> classifyOfflineTaus(const std::vector<const xAOD::TauJet*>& taus, const float threshold = 0.0) const;
    std::tuple<std::vector<const xAOD::TauJet*>, std::vector<const xAOD::TauJet*>, std::vector<const xAOD::TauJet*>> classifyOnlineTaus(const std::vector<const xAOD::TauJet*>& taus, const float threshold = 0.0) const;

private:
    std::map<std::string, TrigTauInfo> m_trigInfo; // TauTrigInfo cache


    // StorageGate keys
    SG::ReadDecorHandleKey<xAOD::EventInfo> m_eventInfoDecorKey{this, "LArStatusFlag", "EventInfo.larFlags", "Key for EventInfo object"}; // To get data-dependencies right

    SG::ReadHandleKey<xAOD::TauJetContainer> m_offlineTauJetKey{this, "OfflineTauJetKey", "TauJets", "Offline taujet container key"};

    SG::ReadHandleKey<xAOD::eFexTauRoIContainer>  m_phase1l1eTauRoIKey{this, "Phase1L1eTauRoIKey", "L1_eTauRoI", "eTau Phase1 L1 RoI key"};
    SG::ReadDecorHandleKey<xAOD::eFexTauRoIContainer> m_phase1l1eTauRoIThresholdPatternsKey{this, "Phase1L1eTauRoIThresholdPatternsKey", "L1_eTauRoI.thresholdPatterns", "Decoration for the threshold patterns for the eTau RoIs"};
    SG::ReadHandleKey<xAOD::jFexTauRoIContainer>  m_phase1l1jTauRoIKey{this, "Phase1L1jTauRoIKey", "L1_jFexTauRoI", "jTau Phase1 L1 RoI key"};
    SG::ReadDecorHandleKey<xAOD::jFexTauRoIContainer> m_phase1l1jTauRoIThresholdPatternsKey{this, "Phase1L1jTauRoIThresholdPatternsKey", "L1_jFexTauRoI.thresholdPatterns", "Decoration for the threshold patterns for the jTau RoIs"};
    SG::ReadHandleKey<xAOD::eFexTauRoIContainer>  m_phase1l1cTauRoIKey{this, "Phase1L1cTauRoIKey", "L1_cTauRoI", "cTau Phase1 L1 RoI key"};
    SG::ReadDecorHandleKey<xAOD::eFexTauRoIContainer> m_phase1l1cTauRoIDecorKey{this, "Phase1L1cTauRoIjTauRoILinkKey", "L1_cTauRoI.jTauLink", "Decoration for the link from eTau to the matching jTau"};
    SG::ReadDecorHandleKey<xAOD::eFexTauRoIContainer> m_phase1l1cTauRoIThresholdPatternsKey{this, "Phase1L1cTauRoIThresholdPatternsKey", "L1_cTauRoI.thresholdPatterns", "Decoration for the threshold patterns for the cTau RoIs"};

    SG::ReadHandleKey<xAOD::TauJetContainer> m_hltTauJetKey{this, "HLTTauJetKey", "HLT_TrigTauRecMerged_MVA", "HLT tracktwoMVA taujet container key"};
    SG::ReadHandleKey<xAOD::TauJetContainer> m_hltTauJetLLPKey{this, "HLTTauJetLLPKey", "HLT_TrigTauRecMerged_LLP", "HLT tracktwoLLP taujet container key"};
    SG::ReadHandleKey<xAOD::TauJetContainer> m_hltTauJetLRTKey{this, "HLTTauJetLRTKey", "HLT_TrigTauRecMerged_LRT", "HLT trackLRT taujet container key"};
    SG::ReadHandleKey<xAOD::TauJetContainer> m_hltTauJetCaloMVAOnlyKey{this, "HLTTauJetCaloMVAOnlyKey", "HLT_TrigTauRecMerged_CaloMVAOnly", "HLT ptonly taujet container key"};

    const SG::ReadHandleKey<xAOD::TauJetContainer>& getOnlineContainerKey(const std::string& trigger) const;

};

#endif
