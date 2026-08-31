/*
# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGMUONHYPO_TRIGMUONEFIDTPHYPOALG_H
#define TRIGMUONHYPO_TRIGMUONEFIDTPHYPOALG_H

#include "TrigMuonEFIdtpHypoTool.h"
#include "DecisionHandling/HypoBase.h"


// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

class TrigMuonEFIdtpHypoAlg : public ::HypoBase
{
public:
   
   TrigMuonEFIdtpHypoAlg( const std::string& name, ISvcLocator* pSvcLocator );   
   
   virtual StatusCode  initialize() override;
   virtual StatusCode  execute( const EventContext& context ) const override;

private:

   ToolHandleArray<TrigMuonEFIdtpHypoTool> m_hypoTools {this, "HypoTools", {}, "Tools to perform selection"}; 
   
   SG::ReadHandleKey<xAOD::TrackParticleContainer> m_PTTracksKey {this, "PTTracks",  "HLT_IDTrack_Muon_IDTrig", ""};
   SG::ReadHandleKey<xAOD::TrackParticleContainer> m_FTFTracksKey{this, "FTFTracks", "HLT_IDTrack_Muon_FTF", ""};

   Gaudi::Property<bool> m_mapToPrevDec{ this, "MapToPreviousDecisions", false, "Map to decisions from previous decisions (needed if IM has mergeUsingFeature=True)"};
   BooleanProperty m_isPhIIMuon { this, "IsPhIIMuon", false, "Whether it's phase-2 muoon software" };
};

#endif
