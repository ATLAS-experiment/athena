/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGTAUMONITORING_TRIGTAUMONITORTRUTHALGORITHM_H
#define TRIGTAUMONITORING_TRIGTAUMONITORTRUTHALGORITHM_H

#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthVertex.h"

#include "TrigTauMonitorBaseAlgorithm.h"

class TrigTauMonitorTruthAlgorithm : public TrigTauMonitorBaseAlgorithm {
public:
    TrigTauMonitorTruthAlgorithm(const std::string& name, ISvcLocator* pSvcLocator);
    virtual StatusCode initialize() override;

private:
    // Get truth 1P and 3P Taus that pass the quality selection cuts
    std::pair<std::vector<std::shared_ptr<xAOD::TruthParticle>>, std::vector<std::shared_ptr<xAOD::TruthParticle>>> getTruthTaus(const EventContext& ctx, const float threshold = 20.0) const;

    // Process truth tau object
    StatusCode examineTruthTau(const std::shared_ptr<xAOD::TruthParticle> xTruthParticle) const;

    virtual StatusCode processEvent(const EventContext& ctx) const override;

    void fillTruthEfficiency(const std::vector<const xAOD::TauJet*>& online_tau_vec_all, const std::vector<std::shared_ptr<xAOD::TruthParticle>>& true_taus, const std::string& trigger, const std::string& nProng) const;
    void fillTruthVars(const std::vector<const xAOD::TauJet*>& tau_vec, const std::vector<std::shared_ptr<xAOD::TruthParticle>>& true_taus, const std::string& trigger, const std::string& nProng) const; 


    // StorageGate keys
    SG::ReadHandleKey< xAOD::TruthParticleContainer> m_truthParticleKey{this, "truthParticleKey", "TruthParticles", "TruthParticleContainer key"};

};

#endif
