/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "DiTauRec/DiTauIDVarDecorator.h"

// Core include(s):
#include "AthLinks/ElementLink.h"
#include "AthContainers/Accessor.h"
#include "AthContainers/ConstAccessor.h"

// EDM include(s):
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/TrackParticle.h"

#include "xAODTau/DiTauJet.h"

DiTauIDVarDecorator::DiTauIDVarDecorator( const std::string& type, const std::string& name, const IInterface * parent) :
  DiTauToolBase(type, name, parent),
  m_dDefault(-1234)
{
  declareInterface<DiTauToolBase > (this);
  declareProperty( "DefaultValue", m_dDefault = -1234);
}


DiTauIDVarDecorator::~DiTauIDVarDecorator() = default;


StatusCode DiTauIDVarDecorator::initialize() {
  ATH_MSG_INFO( "Initializing DiTauIDVarDecorator" );
  
  return StatusCode::SUCCESS;
}


StatusCode DiTauIDVarDecorator::execute(DiTauCandidateData * data, const EventContext& /*ctx*/) const {
    xAOD::DiTauJet* xDiTauPtr = data->xAODDiTau;
    xAOD::DiTauJet& xDiTau = *xDiTauPtr;
    ATH_MSG_DEBUG("Calculate DiTau ID variables");
    
    static const SG::Accessor< int > n_subjetsDec("n_subjets");
    DitauTrackingInfo ditauTrackingInfo;
    ATH_CHECK( getTrackingInfo(xDiTau, ditauTrackingInfo) );
    n_subjetsDec(xDiTau) = ditauTrackingInfo.nSubjets;
    
    static const SG::Accessor< float > ditau_ptDec          ("ditau_pt");          ditau_ptDec(xDiTau) = m_dDefault;        
    static const SG::Accessor< float > f_core_leadDec       ("f_core_lead");       f_core_leadDec(xDiTau) = m_dDefault;             
    static const SG::Accessor< float > f_core_sublDec       ("f_core_subl");       f_core_sublDec(xDiTau) = m_dDefault;             
    static const SG::Accessor< float > f_subjet_leadDec     ("f_subjet_lead");     f_subjet_leadDec(xDiTau) = m_dDefault;             
    static const SG::Accessor< float > f_subjet_sublDec     ("f_subjet_subl");     f_subjet_sublDec(xDiTau) = m_dDefault;             
    static const SG::Accessor< float > f_subjetsDec         ("f_subjets");         f_subjetsDec(xDiTau) = m_dDefault;         
    static const SG::Accessor< float > f_track_leadDec      ("f_track_lead");      f_track_leadDec(xDiTau) = m_dDefault;             
    static const SG::Accessor< float > f_track_sublDec      ("f_track_subl");      f_track_sublDec(xDiTau) = m_dDefault;             
    static const SG::Accessor< float > R_max_leadDec        ("R_max_lead");        R_max_leadDec(xDiTau) = m_dDefault;             
    static const SG::Accessor< float > R_max_sublDec        ("R_max_subl");        R_max_sublDec(xDiTau) = m_dDefault;             
    static const SG::Accessor< int >   n_trackDec           ("n_track");           n_trackDec(xDiTau) = m_dDefault;        
    static const SG::Accessor< int >   n_tracks_leadDec     ("n_tracks_lead");     n_tracks_leadDec(xDiTau) = m_dDefault;             
    static const SG::Accessor< int >   n_tracks_sublDec     ("n_tracks_subl");     n_tracks_sublDec(xDiTau) = m_dDefault;             
    static const SG::Accessor< int >   n_isotrackDec        ("n_isotrack");        n_isotrackDec(xDiTau) = m_dDefault;             
    static const SG::Accessor< float > R_trackDec           ("R_track");           R_trackDec(xDiTau) = m_dDefault;        
    static const SG::Accessor< float > R_track_coreDec      ("R_track_core");      R_track_coreDec(xDiTau) = m_dDefault;             
    static const SG::Accessor< float > R_track_allDec       ("R_track_all");       R_track_allDec(xDiTau) = m_dDefault;             
    static const SG::Accessor< float > R_isotrackDec        ("R_isotrack");        R_isotrackDec(xDiTau) = m_dDefault;             
    static const SG::Accessor< float > R_core_leadDec       ("R_core_lead");       R_core_leadDec(xDiTau) = m_dDefault;             
    static const SG::Accessor< float > R_core_sublDec       ("R_core_subl");       R_core_sublDec(xDiTau) = m_dDefault;             
    static const SG::Accessor< float > R_tracks_leadDec     ("R_tracks_lead");     R_tracks_leadDec(xDiTau) = m_dDefault;             
    static const SG::Accessor< float > R_tracks_sublDec     ("R_tracks_subl");     R_tracks_sublDec(xDiTau) = m_dDefault;             
    static const SG::Accessor< float > M_trackDec           ("m_track");           M_trackDec(xDiTau) = m_dDefault;        
    static const SG::Accessor< float > M_track_coreDec      ("m_track_core");      M_track_coreDec(xDiTau) = m_dDefault;             
    static const SG::Accessor< float > M_core_leadDec       ("m_core_lead");       M_core_leadDec(xDiTau) = m_dDefault;             
    static const SG::Accessor< float > M_core_sublDec       ("m_core_subl");       M_core_sublDec(xDiTau) = m_dDefault;             
    static const SG::Accessor< float > M_track_allDec       ("m_track_all");       M_track_allDec(xDiTau) = m_dDefault;             
    static const SG::Accessor< float > M_tracks_leadDec     ("m_tracks_lead");     M_tracks_leadDec(xDiTau) = m_dDefault;             
    static const SG::Accessor< float > M_tracks_sublDec     ("m_tracks_subl");     M_tracks_sublDec(xDiTau) = m_dDefault;             
    static const SG::Accessor< float > E_frac_sublDec       ("E_frac_subl");       E_frac_sublDec(xDiTau) = m_dDefault;             
    static const SG::Accessor< float > E_frac_subsublDec    ("E_frac_subsubl");    E_frac_subsublDec(xDiTau) = m_dDefault;                 
    static const SG::Accessor< float > R_subjets_sublDec    ("R_subjets_subl");    R_subjets_sublDec(xDiTau) = m_dDefault;                 
    static const SG::Accessor< float > R_subjets_subsublDec ("R_subjets_subsubl"); R_subjets_subsublDec(xDiTau) = m_dDefault;                 
    static const SG::Accessor< float > d0_leadtrack_leadDec ("d0_leadtrack_lead"); d0_leadtrack_leadDec(xDiTau) = m_dDefault;                 
    static const SG::Accessor< float > d0_leadtrack_sublDec ("d0_leadtrack_subl"); d0_leadtrack_sublDec(xDiTau) = m_dDefault;                 
    static const SG::Accessor< float > f_isotracksDec       ("f_isotracks");       f_isotracksDec(xDiTau) = m_dDefault;             
    
    n_trackDec(xDiTau)      = n_track(xDiTau, ditauTrackingInfo);
    n_isotrackDec(xDiTau)   = n_isotrack(xDiTau, ditauTrackingInfo);
    R_track_allDec(xDiTau)  = R_track_all(xDiTau, ditauTrackingInfo);

    M_track_allDec(xDiTau)  = mass_track_all(xDiTau, ditauTrackingInfo);
    f_isotracksDec(xDiTau)  = f_isotracks(xDiTau, ditauTrackingInfo);

    if (ditauTrackingInfo.nSubjets > 2) {
        E_frac_subsublDec(xDiTau)     = E_frac         (xDiTau, ditauTrackingInfo, 2);
        R_subjets_subsublDec(xDiTau)  = R_subjets      (xDiTau, ditauTrackingInfo, 2);  
    } 
    if (ditauTrackingInfo.nSubjets > 1) {
        ditau_ptDec(xDiTau)           = ditau_pt       (xDiTau, ditauTrackingInfo);
        f_subjetsDec(xDiTau)          = f_subjets      (xDiTau, ditauTrackingInfo);
        R_track_coreDec(xDiTau)       = R_track_core   (xDiTau, ditauTrackingInfo);
        R_trackDec(xDiTau)            = R_track        (xDiTau, ditauTrackingInfo);
        R_isotrackDec(xDiTau)         = R_isotrack     (xDiTau, ditauTrackingInfo);
        M_trackDec(xDiTau)            = mass_track     (xDiTau, ditauTrackingInfo);
        M_track_coreDec(xDiTau)       = mass_track_core(xDiTau, ditauTrackingInfo);
        d0_leadtrack_sublDec(xDiTau)  = d0_leadtrack   (xDiTau, ditauTrackingInfo, 1);
        f_core_sublDec(xDiTau)        = f_core         (xDiTau, ditauTrackingInfo, 1);
        f_subjet_sublDec(xDiTau)      = f_subjet       (xDiTau, ditauTrackingInfo, 1);
        f_track_sublDec(xDiTau)       = f_track        (xDiTau, ditauTrackingInfo, 1);
        R_max_sublDec(xDiTau)         = R_max          (xDiTau, ditauTrackingInfo, 1);
        n_tracks_sublDec(xDiTau)      = n_tracks       (xDiTau, ditauTrackingInfo, 1);
        R_core_sublDec(xDiTau)        = R_core         (xDiTau, ditauTrackingInfo, 1);
        R_tracks_sublDec(xDiTau)      = R_tracks       (xDiTau, ditauTrackingInfo, 1);
        M_core_sublDec(xDiTau)        = mass_core      (xDiTau, ditauTrackingInfo, 1);
        M_tracks_sublDec(xDiTau)      = mass_tracks    (xDiTau, ditauTrackingInfo, 1);
        E_frac_sublDec(xDiTau)        = E_frac         (xDiTau, ditauTrackingInfo, 1);
        R_subjets_sublDec(xDiTau)     = R_subjets      (xDiTau, ditauTrackingInfo, 1);
    }  
    if (ditauTrackingInfo.nSubjets > 0) {
        d0_leadtrack_leadDec(xDiTau)  = d0_leadtrack   (xDiTau, ditauTrackingInfo, 0);
        f_core_leadDec(xDiTau)        = f_core         (xDiTau, ditauTrackingInfo, 0);
        f_subjet_leadDec(xDiTau)      = f_subjet       (xDiTau, ditauTrackingInfo, 0);
        f_track_leadDec(xDiTau)       = f_track        (xDiTau, ditauTrackingInfo, 0);
        R_max_leadDec(xDiTau)         = R_max          (xDiTau, ditauTrackingInfo, 0);
        n_tracks_leadDec(xDiTau)      = n_tracks       (xDiTau, ditauTrackingInfo, 0);
        R_core_leadDec(xDiTau)        = R_core         (xDiTau, ditauTrackingInfo, 0);
        R_tracks_leadDec(xDiTau)      = R_tracks       (xDiTau, ditauTrackingInfo, 0);
        M_core_leadDec(xDiTau)        = mass_core      (xDiTau, ditauTrackingInfo, 0);
        M_tracks_leadDec(xDiTau)      = mass_tracks    (xDiTau, ditauTrackingInfo, 0);
    }
    return StatusCode::SUCCESS;
}

float DiTauIDVarDecorator::n_subjets(const xAOD::DiTauJet& xDiTau, const DitauTrackingInfo&) const {
    int nSubjet = 0;
    while (xDiTau.subjetPt(nSubjet) > 0. ){
        nSubjet++;
    }
    return nSubjet;
}

float DiTauIDVarDecorator::ditau_pt(const xAOD::DiTauJet& xDiTau, const DitauTrackingInfo&) const
{
  return xDiTau.subjetPt(0)+xDiTau.subjetPt(1);
}

float DiTauIDVarDecorator::f_core(const xAOD::DiTauJet& xDiTau, const DitauTrackingInfo&, int iSubjet) const 
{
  return xDiTau.fCore(iSubjet);
}

float DiTauIDVarDecorator::f_subjet(const xAOD::DiTauJet& xDiTau, const DitauTrackingInfo&, int iSubjet) const {
    return xDiTau.subjetPt(iSubjet) / xDiTau.pt();
}

float DiTauIDVarDecorator::f_subjets(const xAOD::DiTauJet& xDiTau, const DitauTrackingInfo&) const
{
  return (xDiTau.subjetPt(0) + xDiTau.subjetPt(1))/ xDiTau.pt();
}

float DiTauIDVarDecorator::f_track(const xAOD::DiTauJet&, const DitauTrackingInfo& ditauInfo, int iSubjet) const
{
    SubjetTrackingInfo subjetInfo = ditauInfo.vSubjetInfo.at(iSubjet);
    float leadTrackPt = subjetInfo.leadTrack ? subjetInfo.leadTrack->pt() : 0;
    float subjetPt = subjetInfo.subjet_p4.Pt();
  return leadTrackPt / subjetPt;
}

float DiTauIDVarDecorator::R_max(const xAOD::DiTauJet&, const DitauTrackingInfo& ditauInfo, int iSubjet) const { 
    SubjetTrackingInfo subjetInfo = ditauInfo.vSubjetInfo.at(iSubjet);
    float Rmax = 0;
    for (const xAOD::TrackParticle* xTrack: subjetInfo.vTracks) {
        if (subjetInfo.subjet_p4.DeltaR(xTrack->p4()) > Rmax) {
            Rmax = subjetInfo.subjet_p4.DeltaR(xTrack->p4());
        }
    }
    return Rmax;
}

int DiTauIDVarDecorator::n_track(const xAOD::DiTauJet& xDiTau, const DitauTrackingInfo&) const{ 
    return xDiTau.nTracks();
}

int DiTauIDVarDecorator::n_tracks(const xAOD::DiTauJet&, const DitauTrackingInfo& ditauInfo, int iSubjet) const { 
    SubjetTrackingInfo subjetInfo = ditauInfo.vSubjetInfo.at(iSubjet);
    return subjetInfo.vTracks.size();
}

int DiTauIDVarDecorator::n_isotrack(const xAOD::DiTauJet& xDiTau, const DitauTrackingInfo&) const {
  return xDiTau.nIsoTracks();
}

float DiTauIDVarDecorator::R_tracks(const xAOD::DiTauJet&, const DitauTrackingInfo& ditauInfo, int iSubjet) const { 
    double R_sum = 0;
    double pt = 0;

    SubjetTrackingInfo subjetInfo = ditauInfo.vSubjetInfo.at(iSubjet);
    for (const xAOD::TrackParticle* xTrack: subjetInfo.vTracks) {
        R_sum += subjetInfo.subjet_p4.DeltaR(xTrack->p4()) * xTrack->pt();
        pt += xTrack->pt();
    }
    if (pt == 0) {
        return m_dDefault;
    }
    return R_sum / pt;
}

float DiTauIDVarDecorator::R_core(const xAOD::DiTauJet&, const DitauTrackingInfo& ditauInfo, int iSubjet) const { 
    double R_sum = 0;
    double pt = 0;
    SubjetTrackingInfo subjetInfo = ditauInfo.vSubjetInfo.at(iSubjet);
    for (const xAOD::TrackParticle* xTrack: subjetInfo.vCoreTracks) {
        R_sum += subjetInfo.subjet_p4.DeltaR(xTrack->p4()) * xTrack->pt();
        pt += xTrack->pt();
    }
    if (pt == 0) {
        return m_dDefault;
    }
    return R_sum / pt;

}

float DiTauIDVarDecorator::R_track_core(const xAOD::DiTauJet&, const DitauTrackingInfo& ditauInfo) const { 
    float R_sum = 0;
    float pt = 0;
    for (int i = 0; i < 2; i++) {
        SubjetTrackingInfo subjetInfo = ditauInfo.vSubjetInfo.at(i);
        for (const xAOD::TrackParticle* xTrack: subjetInfo.vCoreTracks) {
            R_sum += subjetInfo.subjet_p4.DeltaR(xTrack->p4()) * xTrack->pt();
            pt += xTrack->pt();
        }
    }
    if (pt == 0) {
        return m_dDefault;
    }
    return R_sum / pt;
}

float DiTauIDVarDecorator::R_track(const xAOD::DiTauJet&, const DitauTrackingInfo& ditauInfo) const
{
    float R_sum = 0;
    float pt = 0;
    for (int i = 0; i < 2; i++) {
        SubjetTrackingInfo subjetInfo = ditauInfo.vSubjetInfo.at(i);
        for (const xAOD::TrackParticle* xTrack: subjetInfo.vTracks) {
            R_sum += subjetInfo.subjet_p4.DeltaR(xTrack->p4()) * xTrack->pt();
            pt += xTrack->pt();
        }
    }
    if (pt == 0) {
        return m_dDefault;
    }
    return R_sum / pt;
}

float DiTauIDVarDecorator::R_track_all(const xAOD::DiTauJet&, const DitauTrackingInfo& ditauInfo) const
{
    double R_sum = 0;
    double pt = 0;

    for (const SubjetTrackingInfo& subjetInfo : ditauInfo.vSubjetInfo) {
        for (const xAOD::TrackParticle* xTrack: subjetInfo.vTracks) { 
            R_sum += subjetInfo.subjet_p4.DeltaR(xTrack->p4())*xTrack->pt();
            pt += xTrack->pt();
        }
    }
    if (pt == 0) {
        return m_dDefault;
    }
    return R_sum / pt;
}

float DiTauIDVarDecorator::R_isotrack(const xAOD::DiTauJet&, const DitauTrackingInfo& ditauInfo) const
{
    float R_sum = 0;
    float pt = 0;
    for (int i = 0; i < 2; i++) {
        SubjetTrackingInfo subjetInfo = ditauInfo.vSubjetInfo.at(i);
        for (const xAOD::TrackParticle* xTrack: subjetInfo.vIsoTracks) {
            R_sum += subjetInfo.subjet_p4.DeltaR(xTrack->p4()) * xTrack->pt();
            pt += xTrack->pt();
        }
    }
    if (pt == 0) {
        return m_dDefault;
    }
    return R_sum / pt;
}

float DiTauIDVarDecorator::mass_track_core(const xAOD::DiTauJet&, const DitauTrackingInfo& ditauInfo) const
{
    TLorentzVector allCoreTracks_p4;
    for (int i = 0; i < 2; i++) {
        SubjetTrackingInfo subjetInfo = ditauInfo.vSubjetInfo.at(i);
        for (const xAOD::TrackParticle* xTrack: subjetInfo.vCoreTracks) {
            allCoreTracks_p4 += xTrack->p4();
        }
    }
    float mass = allCoreTracks_p4.M();
    if (mass < 0) {
        return m_dDefault;
    }
    return mass;
}

float DiTauIDVarDecorator::mass_core(const xAOD::DiTauJet&, const DitauTrackingInfo& ditauInfo, int iSubjet) const {
    TLorentzVector allCoreTracks_p4;
    SubjetTrackingInfo subjetInfo = ditauInfo.vSubjetInfo.at(iSubjet);
    for (const xAOD::TrackParticle* xTrack: subjetInfo.vCoreTracks) {
        allCoreTracks_p4 += xTrack->p4();
    }
    float mass = allCoreTracks_p4.M();
    if (mass < 0) {
        return m_dDefault;
    }
    return mass;
}

float DiTauIDVarDecorator::mass_tracks(const xAOD::DiTauJet&, const DitauTrackingInfo& ditauInfo, int iSubjet) const {
    TLorentzVector allTracks_p4;
    SubjetTrackingInfo subjetInfo = ditauInfo.vSubjetInfo.at(iSubjet);
    for (const xAOD::TrackParticle* xTrack: subjetInfo.vTracks) {
        allTracks_p4 += xTrack->p4();
    }
    float mass = allTracks_p4.M();
    if (mass < 0) {
        return m_dDefault;
    }
    return mass;
}

float DiTauIDVarDecorator::mass_track(const xAOD::DiTauJet&, const DitauTrackingInfo& ditauInfo) const {
    TLorentzVector allTracks_p4;
    for (const xAOD::TrackParticle* xTrack: ditauInfo.vTracks) {
        allTracks_p4 += xTrack->p4();
    }
    float mass = allTracks_p4.M();
    if (mass < 0) {
        return m_dDefault;
    }
    return mass;
}

float DiTauIDVarDecorator::mass_track_all(const xAOD::DiTauJet&, const DitauTrackingInfo& ditauInfo) const {
    TLorentzVector allTracks_p4;
    for (const xAOD::TrackParticle* xTrack: ditauInfo.vTracks) {
        allTracks_p4 += xTrack->p4();
    }
    for (const xAOD::TrackParticle* xTrack: ditauInfo.vIsoTracks) {
        allTracks_p4 += xTrack->p4();
    }
    float mass = allTracks_p4.M();
    if (mass < 0) {
        return m_dDefault;
    }
    return mass;
}

float DiTauIDVarDecorator::E_frac(const xAOD::DiTauJet& xDiTau, const DitauTrackingInfo&, int iSubjet) const { 
  return xDiTau.subjetE(iSubjet) / xDiTau.subjetE(0);
}

float DiTauIDVarDecorator::R_subjets(const xAOD::DiTauJet&, const DitauTrackingInfo& ditauInfo, int iSubjet) const { 
  TLorentzVector leadSubjet_p4 = ditauInfo.vSubjetInfo.at(0).subjet_p4;
  TLorentzVector subjet_p4 = ditauInfo.vSubjetInfo.at(iSubjet).subjet_p4;
  return leadSubjet_p4.DeltaR(subjet_p4);
}

float DiTauIDVarDecorator::d0_leadtrack(const xAOD::DiTauJet&, const DitauTrackingInfo& ditauInfo, int iSubjet) const { 
    SubjetTrackingInfo subjetInfo = ditauInfo.vSubjetInfo.at(iSubjet);
    if (!subjetInfo.leadTrack) {
        return m_dDefault;
    }
    return subjetInfo.leadTrack->d0();
}

float DiTauIDVarDecorator::f_isotracks(const xAOD::DiTauJet& xDiTau, const DitauTrackingInfo& ditauInfo) const { 
    double iso_pt = 0;
    for (const xAOD::TrackParticle* xTrack: ditauInfo.vIsoTracks) {
        iso_pt += xTrack->pt();
    }
    return iso_pt / xDiTau.pt();
}

StatusCode DiTauIDVarDecorator::getTrackingInfo(xAOD::DiTauJet& xDiTau, DitauTrackingInfo& trackingInfo) const {
    static const SG::ConstAccessor<std::vector<ElementLink<xAOD::TrackParticleContainer>>> trackLinksAcc("trackLinks");
    static const SG::ConstAccessor<std::vector<ElementLink<xAOD::TrackParticleContainer>>> isoTrackLinksAcc("isoTrackLinks");
    static const SG::ConstAccessor<float> R_subjetAcc("R_subjet");
    static const SG::ConstAccessor<float> R_coreAcc("R_core");
    

    if (!trackLinksAcc.isAvailable(xDiTau) || !isoTrackLinksAcc.isAvailable(xDiTau)) {
        ATH_MSG_WARNING("Track " << (!trackLinksAcc.isAvailable(xDiTau) ? "DiTauJet.trackLinks" : "DiTauJet.isoTrackLinks") << " links not available.");
        return StatusCode::FAILURE;
    } 

    int nSubjets = n_subjets(xDiTau, trackingInfo);
    float Rsubjet = R_subjetAcc(xDiTau);
    float RCore = R_coreAcc(xDiTau);

    trackingInfo.nSubjets = nSubjets;
    trackingInfo.vSubjetInfo.clear();
    trackingInfo.vIsoTracks.clear();
    trackingInfo.vTracks.clear();
    
    // Get the track links from the DiTauJet and store them in the tracking info 
    std::vector<ElementLink<xAOD::TrackParticleContainer>> isoTrackLinks = xDiTau.isoTrackLinks();
    for (const auto &trackLink: isoTrackLinks) {
        if (!trackLink.isValid()) {
        ATH_MSG_WARNING("Iso track link is not valid");
        continue;
        }
        const xAOD::TrackParticle* xTrack = *trackLink;
        trackingInfo.vIsoTracks.push_back(xTrack);
    } 
    std::vector<ElementLink<xAOD::TrackParticleContainer>> trackLinks = xDiTau.trackLinks();
    for (const auto &trackLink : trackLinks)  {
        if (!trackLink.isValid()) {
        ATH_MSG_WARNING("track link is not valid");
        continue;
        }
        const xAOD::TrackParticle* xTrack = *trackLink;
        trackingInfo.vTracks.push_back(xTrack);
    } 
    // store subjet p4
    for (int i=0; i<nSubjets; ++i){
        SubjetTrackingInfo subjetTrackingInfo;
        TLorentzVector subjet_p4 = TLorentzVector();
        subjet_p4.SetPtEtaPhiE( xDiTau.subjetPt(i), xDiTau.subjetEta(i), xDiTau.subjetPhi(i), xDiTau.subjetE(i));
        subjetTrackingInfo.subjet_p4 = subjet_p4;
        trackingInfo.vSubjetInfo.push_back(subjetTrackingInfo);
    }
    for (const auto track : trackingInfo.vTracks) {
        float dRMin = 999;
        int inSubjet = -1;
        for (int i=0; i<nSubjets; ++i){
            float dRTrackSubjet = trackingInfo.vSubjetInfo[i].subjet_p4.DeltaR(track->p4());
            if (dRTrackSubjet < Rsubjet && dRTrackSubjet < dRMin){
                dRMin = dRTrackSubjet;
                inSubjet = i;
            }
        }
        if (inSubjet >= 0){
            trackingInfo.vSubjetInfo[inSubjet].vTracks.push_back(track);
        }
    }
    // find leading track in subjets
    for (int i=0; i<nSubjets; ++i){
        float ptLeadTrack = 0;
        for (const auto track : trackingInfo.vSubjetInfo[i].vTracks){
            if (track->pt() > ptLeadTrack){
                ptLeadTrack = track->pt();
                trackingInfo.vSubjetInfo[i].leadTrack = track;
            }
        }
    }
    // find core track in subjets
    for (int i=0; i<nSubjets; ++i){
        for (const auto track : trackingInfo.vSubjetInfo[i].vTracks){
            auto subjetTrackingInfo = trackingInfo.vSubjetInfo[i];
            if (subjetTrackingInfo.subjet_p4.DeltaR(track->p4()) < RCore){
                trackingInfo.vSubjetInfo[i].vCoreTracks.push_back(track);
            }
        }
    }
    //find isotracks in subjets
    for (const auto track : trackingInfo.vIsoTracks){
        float RIso = 0.4;
        float dRMin = 999;
        int inSubjet = -1;
        for (int i=0; i<nSubjets; ++i){
            float dRTrackSubjet = trackingInfo.vSubjetInfo[i].subjet_p4.DeltaR(track->p4());
            if (dRTrackSubjet > Rsubjet && dRTrackSubjet < RIso && dRTrackSubjet < dRMin){
                dRMin = dRTrackSubjet;
                inSubjet = i;
            }
        }
        if (inSubjet >= 0){
            trackingInfo.vSubjetInfo[inSubjet].vIsoTracks.push_back(track);
        }
    }
    return StatusCode::SUCCESS;
}
