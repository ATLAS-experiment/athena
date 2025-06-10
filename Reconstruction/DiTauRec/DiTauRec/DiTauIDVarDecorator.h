/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#pragma once

// EDM include(s):
#include "xAODTau/TauxAODHelpers.h"

// Local include(s):
#include "DiTauToolBase.h"
#include "GaudiKernel/ToolHandle.h"
#include "AsgDataHandles/WriteDecorHandleKey.h"
#include "AsgDataHandles/WriteDecorHandle.h"


class DiTauIDVarDecorator : public DiTauToolBase
{
public:
  DiTauIDVarDecorator( const std::string& type, const std::string& name, const IInterface * parent);
  virtual ~DiTauIDVarDecorator();
  virtual StatusCode initialize() override;
  virtual StatusCode execute(DiTauCandidateData * data, const EventContext& ctx) const override;

private: 
  struct SubjetTrackingInfo{
    TLorentzVector subjet_p4;
    std::vector<const xAOD::TrackParticle*> vTracks;
    std::vector<const xAOD::TrackParticle*> vIsoTracks;
    std::vector<const xAOD::TrackParticle*> vCoreTracks;
    const xAOD::TrackParticle* leadTrack = nullptr;
  };
  struct DitauTrackingInfo{
    std::vector<const xAOD::TrackParticle*> vTracks;
    std::vector<const xAOD::TrackParticle*> vIsoTracks;
    int nSubjets = 0;
    std::vector<SubjetTrackingInfo> vSubjetInfo;
  };
private:
  float n_subjets       (const xAOD::DiTauJet& xDiTau, const DitauTrackingInfo& ditauInfo)              const;
  float ditau_pt        (const xAOD::DiTauJet& xDiTau, const DitauTrackingInfo& ditauInfo)              const;
  float f_core          (const xAOD::DiTauJet& xDiTau, const DitauTrackingInfo& ditauInfo, int iSubjet) const;
  float f_subjet        (const xAOD::DiTauJet& xDiTau, const DitauTrackingInfo& ditauInfo, int iSubjet) const;
  float f_subjets       (const xAOD::DiTauJet& xDiTau, const DitauTrackingInfo& ditauInfo)              const;
  float f_track         (const xAOD::DiTauJet& xDiTau, const DitauTrackingInfo& ditauInfo, int iSubjet) const;
  float R_max           (const xAOD::DiTauJet& xDiTau, const DitauTrackingInfo& ditauInfo, int iSubjet) const;
  int   n_track         (const xAOD::DiTauJet& xDiTau, const DitauTrackingInfo& ditauInfo)              const;
  int   n_tracks        (const xAOD::DiTauJet& xDiTau, const DitauTrackingInfo& ditauInfo, int iSubjet) const;
  int   n_isotrack      (const xAOD::DiTauJet& xDiTau, const DitauTrackingInfo& ditauInfo)              const;
  float R_track         (const xAOD::DiTauJet& xDiTau, const DitauTrackingInfo& ditauInfo)              const;
  float R_track_all     (const xAOD::DiTauJet& xDiTau, const DitauTrackingInfo& ditauInfo)              const;
  float R_track_core    (const xAOD::DiTauJet& xDiTau, const DitauTrackingInfo& ditauInfo)              const;
  float R_isotrack      (const xAOD::DiTauJet& xDiTau, const DitauTrackingInfo& ditauInfo)              const;
  float R_core          (const xAOD::DiTauJet& xDiTau, const DitauTrackingInfo& ditauInfo, int iSubjet) const;
  float R_tracks        (const xAOD::DiTauJet& xDiTau, const DitauTrackingInfo& ditauInfo, int iSubjet) const;
  float mass_track      (const xAOD::DiTauJet& xDiTau, const DitauTrackingInfo& ditauInfo)              const;
  float mass_track_core (const xAOD::DiTauJet& xDiTau, const DitauTrackingInfo& ditauInfo)              const;
  float mass_core       (const xAOD::DiTauJet& xDiTau, const DitauTrackingInfo& ditauInfo, int iSubjet) const;
  float mass_track_all  (const xAOD::DiTauJet& xDiTau, const DitauTrackingInfo& ditauInfo)              const;
  float mass_tracks     (const xAOD::DiTauJet& xDiTau, const DitauTrackingInfo& ditauInfo, int iSubjet) const;
  float E_frac          (const xAOD::DiTauJet& xDiTau, const DitauTrackingInfo& ditauInfo, int iSubjet) const;
  float R_subjets       (const xAOD::DiTauJet& xDiTau, const DitauTrackingInfo& ditauInfo, int iSubjet) const;
  float d0_leadtrack    (const xAOD::DiTauJet& xDiTau, const DitauTrackingInfo& ditauInfo, int iSubjet) const;
  float f_isotracks     (const xAOD::DiTauJet& xDiTau, const DitauTrackingInfo& ditauInfo)              const;
  
  float m_dDefault;
  StatusCode getTrackingInfo(xAOD::DiTauJet& xDiTau, DitauTrackingInfo& trackingInfo) const;

  // Decorators 
  SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> m_ditau_ptDecKey          { this, "ditauPtDecName",         "DiTauJets.ditau_pt",          "Name of the ditauPt Decorator"};  
  SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> m_f_core_leadDecKey       { this, "fCoreLeadDecName",       "DiTauJets.f_core_lead",       "Name of the fCoreLead Decorator"};    
  SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> m_f_core_sublDecKey       { this, "fCoreSublDecName",       "DiTauJets.f_core_subl",       "Name of the fCoreSubl Decorator"};    
  SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> m_f_subjet_leadDecKey     { this, "fSubjetLeadDecName",     "DiTauJets.f_subjet_lead",     "Name of the fSubjetLead Decorator"};      
  SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> m_f_subjet_sublDecKey     { this, "fSubjetSublDecName",     "DiTauJets.f_subjet_subl",     "Name of the fSubjetSubl Decorator"};      
  SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> m_f_subjetsDecKey         { this, "fSubjetsDecName",        "DiTauJets.f_subjets",         "Name of the fSubjets Decorator"};  
  SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> m_f_track_leadDecKey      { this, "fTrackLeadDecName",      "DiTauJets.f_track_lead",      "Name of the fTrackLead Decorator"};      
  SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> m_f_track_sublDecKey      { this, "fTrackSublDecName",      "DiTauJets.f_track_subl",      "Name of the fTrackSubl Decorator"};      
  SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> m_R_max_leadDecKey        { this, "RMaxLeadDecName",        "DiTauJets.R_max_lead",        "Name of the RMaxLead Decorator"};    
  SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> m_R_max_sublDecKey        { this, "RMaxSublDecName",        "DiTauJets.R_max_subl",        "Name of the RMaxSubl Decorator"};    
  SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> m_n_trackDecKey           { this, "nTrackDecName",          "DiTauJets.n_track",           "Name of the nTrack Decorator"};
  SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> m_n_tracks_leadDecKey     { this, "nTracksLeadDecName",     "DiTauJets.n_tracks_lead",     "Name of the nTracksLead Decorator"};      
  SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> m_n_tracks_sublDecKey     { this, "nTracksSublDecName",     "DiTauJets.n_tracks_subl",     "Name of the nTracksSubl Decorator"};      
  SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> m_n_isotrackDecKey        { this, "nIsotrackDecName",       "DiTauJets.n_isotrack",        "Name of the nIsotrack Decorator"};    
  SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> m_R_trackDecKey           { this, "RTrackDecName",          "DiTauJets.R_track",           "Name of the RTrack Decorator"};
  SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> m_R_track_coreDecKey      { this, "RTrackCoreDecName",      "DiTauJets.R_track_core",      "Name of the RTrackCore Decorator"};      
  SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> m_R_track_allDecKey       { this, "RTrackAllDecName",       "DiTauJets.R_track_all",       "Name of the RTrackAll Decorator"};    
  SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> m_R_isotrackDecKey        { this, "RIsotrackDecName",       "DiTauJets.R_isotrack",        "Name of the RIsotrack Decorator"};    
  SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> m_R_core_leadDecKey       { this, "RCoreLeadDecName",       "DiTauJets.R_core_lead",       "Name of the RCoreLead Decorator"};    
  SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> m_R_core_sublDecKey       { this, "RCoreSublDecName",       "DiTauJets.R_core_subl",       "Name of the RCoreSubl Decorator"};    
  SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> m_R_tracks_leadDecKey     { this, "RTracksLeadDecName",     "DiTauJets.R_tracks_lead",     "Name of the RTracksLead Decorator"};      
  SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> m_R_tracks_sublDecKey     { this, "RTracksSublDecName",     "DiTauJets.R_tracks_subl",     "Name of the RTracksSubl Decorator"};      
  SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> m_M_trackDecKey           { this, "MTrackDecName",          "DiTauJets.m_track",           "Name of the MTrack Decorator"};
  SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> m_M_track_coreDecKey      { this, "MTrackCoreDecName",      "DiTauJets.m_track_core",      "Name of the MTrackCore Decorator"};      
  SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> m_M_core_leadDecKey       { this, "MCoreLeadDecName",       "DiTauJets.m_core_lead",       "Name of the MCoreLead Decorator"};    
  SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> m_M_core_sublDecKey       { this, "MCoreSublDecName",       "DiTauJets.m_core_subl",       "Name of the MCoreSubl Decorator"};    
  SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> m_M_track_allDecKey       { this, "MTrackAllDecName",       "DiTauJets.m_track_all",       "Name of the MTrackAll Decorator"};    
  SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> m_M_tracks_leadDecKey     { this, "MTracksLeadDecName",     "DiTauJets.m_tracks_lead",     "Name of the MTracksLead Decorator"};      
  SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> m_M_tracks_sublDecKey     { this, "MTracksSublDecName",     "DiTauJets.m_tracks_subl",     "Name of the MTracksSubl Decorator"};      
  SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> m_E_frac_sublDecKey       { this, "EFracSublDecName",       "DiTauJets.E_frac_subl",       "Name of the EFracSubl Decorator"};    
  SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> m_E_frac_subsublDecKey    { this, "EFracSubsublDecName",    "DiTauJets.E_frac_subsubl",    "Name of the EFracSubsubl Decorator"};        
  SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> m_R_subjets_sublDecKey    { this, "RSubjetsSublDecName",    "DiTauJets.R_subjets_subl",    "Name of the RSubjetsSubl Decorator"};        
  SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> m_R_subjets_subsublDecKey { this, "RSubjetsSubsublDecName", "DiTauJets.R_subjets_subsubl", "Name of the RSubjetsSubsubl Decorator"};          
  SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> m_d0_leadtrack_leadDecKey { this, "d0LeadtrackLeadDecName", "DiTauJets.d0_leadtrack_lead", "Name of the d0LeadtrackLead Decorator"};          
  SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> m_d0_leadtrack_sublDecKey { this, "d0LeadtrackSublDecName", "DiTauJets.d0_leadtrack_subl", "Name of the d0LeadtrackSubl Decorator"};          
  SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> m_f_isotracksDecKey       { this, "fIsotracksDecName",      "DiTauJets.f_isotracks",       "Name of the fIsotracks Decorator"};    
}; // class DiTauIDVarDecorator
