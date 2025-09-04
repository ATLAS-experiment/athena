/*
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGMUONHYPO_TRIGMUONEFIDTPINVMASSHYPOTOOL_H 
#define TRIGMUONHYPO_TRIGMUONEFIDTPINVMASSHYPOTOOL_H

#include <string>
#include "AthenaBaseComps/AthAlgTool.h" 
#include "TrigCompositeUtils/HLTIdentifier.h"
#include "TrigCompositeUtils/TrigCompositeUtils.h" 
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"
#include "DecisionHandling/ComboHypoToolBase.h"

#include "MuonAnalysisInterfaces/IMuonSelectionTool.h"

class StoreGateSvc;

class TrigMuonEFIdtpInvMassHypoTool: public ::ComboHypoToolBase
{
public:
   TrigMuonEFIdtpInvMassHypoTool(const std::string& type, const std::string & name, const IInterface* parent);
   ~TrigMuonEFIdtpInvMassHypoTool() = default;
 
   virtual StatusCode initialize() override;    

private:

   bool executeAlg(const std::vector<Combo::LegDecision>& combinaiton, const EventContext& ctx) const override;

   StatusCode doTPIdperf(const xAOD::TrackParticle* metrack, const std::vector<const xAOD::TrackParticle*>& tracks_pt, const std::vector<const xAOD::TrackParticle*>& tracks_ftf) const;

   float qOverPMatching(const xAOD::TrackParticle* metrack, const xAOD::TrackParticle* idtrack) const;
   bool  passedCBQualityCuts(const xAOD::Muon* muon) const;
   bool  passedSAQualityCuts(const xAOD::Muon* muon) const;

   // Properties:
   Gaudi::Property< double> m_invMassLow {
      this, "InvMassLow", -1.0, "Low threshold for invariant mass cut" };
   Gaudi::Property< double> m_invMassHigh {
      this, "InvMassHigh", -1.0, "High threshold for invariant mass cut" };
   Gaudi::Property< bool > m_acceptAll {
      this, "AcceptAll", false, "Ignore selection" };
   Gaudi::Property< bool > m_selOS {
      this, "SelectOppositeSign", false, "Select only oppositly charged pairs" };
   Gaudi::Property< bool > m_muonqualityCut {
      this, "MuonQualityCut", false, "Ignore selection" };

   // Other members:   
   ToolHandle< GenericMonitoringTool > m_monTool    {this, "MonTool", "", "Monitoring tool" };
   ToolHandle<CP::IMuonSelectionTool>  m_muonSelTool{this, "MuonSelectionTool", "CP::MuonSelectionTool/MuonSelectionTool", "Tool for muon quality selection"};
};

#endif
