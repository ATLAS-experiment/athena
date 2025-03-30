/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#pragma once

// EDM include(s):
#include "xAODTau/TauxAODHelpers.h"

// Local include(s):
#include "DiTauToolBase.h"
#include "GaudiKernel/ToolHandle.h"


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
}; // class DiTauIDVarDecorator
