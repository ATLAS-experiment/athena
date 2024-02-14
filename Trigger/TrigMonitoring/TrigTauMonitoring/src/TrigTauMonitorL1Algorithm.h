/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGTAUMONITORING_TRIGTAUMONITORL1ALGORITHM_H
#define TRIGTAUMONITORING_TRIGTAUMONITORL1ALGORITHM_H

#include "TrigTauMonitorBaseAlgorithm.h"

class TrigTauMonitorL1Algorithm : public TrigTauMonitorBaseAlgorithm {
public:
    TrigTauMonitorL1Algorithm(const std::string& name, ISvcLocator* pSvcLocator);

private:
    // Require at least 1 offline Tau per event (will bias the variable distributions for background events)
    Gaudi::Property<bool> m_requireOfflineTaus{this, "RequireOfflineTaus", true, "Require at leat 1 offline tau per event"};

    virtual StatusCode processEvent(const EventContext& ctx) const override;

    template <typename T = xAOD::eFexTauRoI>
    void fillL1Efficiencies(const EventContext& ctx, const std::vector<const xAOD::TauJet*>& offline_tau_vec, const std::string& nProng, const std::string& trigger, const std::vector<const T*>& rois) const
    {
        ATH_MSG_DEBUG("Fill L1 efficiencies: " << trigger);

        const TrigTauInfo& info = getTrigInfo(trigger);

        auto monGroup = getGroup(trigger+"_L1_Efficiency_"+nProng);

        auto tauPt = Monitored::Scalar<float>("tauPt", 0.0);
        auto tauEta = Monitored::Scalar<float>("tauEta", 0.0);
        auto tauPhi = Monitored::Scalar<float>("tauPhi", 0.0);
        auto averageMu = Monitored::Scalar<float>("averageMu", 0.0);
        auto L1_match = Monitored::Scalar<bool>("L1_pass", false);
        auto L1_match_highPt = Monitored::Scalar<bool>("L1_pass_highPt", false);

        averageMu = lbAverageInteractionsPerCrossing(ctx);

        for(const xAOD::TauJet* offline_tau : offline_tau_vec) {
            tauPt = offline_tau->pt()/Gaudi::Units::GeV;
            tauEta = offline_tau->eta();
            tauPhi = offline_tau->phi();

            bool is_highPt = tauPt > info.getL1TauThreshold() + 20.0;

            L1_match = false;
            for(const T* roi : rois) {
                L1_match = matchObjects(offline_tau, roi, 0.3);
                if(L1_match) break;
            }

            fill(monGroup, tauPt, tauEta, tauPhi, averageMu, L1_match);

            if(is_highPt) {
                L1_match_highPt = static_cast<bool>(L1_match);
                fill(monGroup, tauEta, tauPhi, L1_match_highPt);
            }
        }

        ATH_MSG_DEBUG("After fill L1 efficiencies: " << trigger);
    } 

    void fillL1eTauVars(const std::string& trigger, const std::vector<const xAOD::eFexTauRoI*>& rois) const;
    void fillL1jTauVars(const std::string& trigger, const std::vector<const xAOD::jFexTauRoI*>& rois) const;
    void fillL1cTauVars(const std::string& trigger, const std::vector<std::pair<const xAOD::eFexTauRoI*, const xAOD::jFexTauRoI*>>& rois) const;
    void fillL1LegacyVars(const std::string& trigger, const std::vector<const xAOD::EmTauRoI*>& rois) const;
};

#endif
