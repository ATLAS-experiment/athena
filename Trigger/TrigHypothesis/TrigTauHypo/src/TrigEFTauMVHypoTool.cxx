/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <list>
#include <iterator>
#include <sstream>

#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/SystemOfUnits.h"

#include "TrigSteeringEvent/TrigRoiDescriptor.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "xAODTau/TauJetContainer.h"

#include "TrigEFTauMVHypoTool.h"

using namespace TrigCompositeUtils;

TrigEFTauMVHypoTool::TrigEFTauMVHypoTool(const std::string& type, const std::string& name, const IInterface* parent)
  : base_class(type, name, parent), 
    m_decisionId(HLT::Identifier::fromToolName(name))
{

}


TrigEFTauMVHypoTool::~TrigEFTauMVHypoTool()
{  

}


StatusCode TrigEFTauMVHypoTool::initialize()
{
    ATH_MSG_DEBUG(name() << ": in initialize()");

    ATH_MSG_DEBUG("TrigEFTauMVHypoTool will cut on:");
    ATH_MSG_DEBUG(" - PtMin: " << m_ptMin);
    ATH_MSG_DEBUG(" - NTrackMin: " << m_numTrackMin);
    ATH_MSG_DEBUG(" - NTrackMax: " << m_numTrackMax);
    ATH_MSG_DEBUG(" - NWideTrackMax: " << m_numWideTrackMax);
    if(m_trackPtCut >= 0) ATH_MSG_DEBUG(" - trackPtCut: " << m_trackPtCut);
    ATH_MSG_DEBUG(" - IDMethod: " << m_idMethod);
    ATH_MSG_DEBUG(" - IDWP: " << m_idWP);
    ATH_MSG_DEBUG(" - HighPtSelection: " << m_doHighPtSelection);
    if(m_doHighPtSelection) {
        ATH_MSG_DEBUG("   - HighPtSelectionTrkThr: " << m_highPtTrkThr);
        ATH_MSG_DEBUG("   - HighPtSelectionLooseIDThr: " << m_highPtLooseIDThr);
        ATH_MSG_DEBUG("   - HighPtSelectionJetThr: " << m_highPtJetThr);
    }

    if((m_numTrackMin > m_numTrackMax) || (m_highPtLooseIDThr > m_highPtJetThr)) {
        ATH_MSG_ERROR("Invalid tool configuration!");
        return StatusCode::FAILURE;
    }

    if(!(m_idMethod == IDMethod::Disabled || m_idMethod == IDMethod::RNN)) {
        ATH_MSG_ERROR("Invalid IDMethod value, " << m_idMethod);
        return StatusCode::FAILURE;
    } else if(m_idWP < IDWP::None || m_idWP > IDWP::Tight) {
        ATH_MSG_ERROR("Invalid IDWP value, " << m_idWP);
        return StatusCode::FAILURE;
    } else if(m_idMethod == IDMethod::Disabled && m_idWP != IDWP::None) {
        ATH_MSG_ERROR("IDMethod=0 must be set together with IDWP=-1");
    }

    return StatusCode::SUCCESS;
}


bool TrigEFTauMVHypoTool::decide(const ITrigEFTauMVHypoTool::TauJetInfo& input) const
{
    ATH_MSG_DEBUG(name() << ": in execute()");

    // Tau pass flag
    bool pass = false;

    auto nInputTaus         = Monitored::Scalar<int>("nInputTaus", -1);
    auto passedCuts         = Monitored::Scalar<int>("CutCounter", 0);
    auto ptAccepted         = Monitored::Scalar<float>("ptAccepted", -1);
    auto nTrackAccepted	    = Monitored::Scalar<int>("nTrackAccepted", -1);
    auto nWideTrackAccepted = Monitored::Scalar<int>("nWideTrackAccepted", -1);
    auto IDScore_0p         = Monitored::Scalar<float>("RNNJetScoreAccepted_0p", -1);
    auto IDScoreSigTrans_0p = Monitored::Scalar<float>("RNNJetScoreSigTransAccepted_0p", -1);
    auto IDScore_1p         = Monitored::Scalar<float>("RNNJetScoreAccepted_1p", -1);
    auto IDScoreSigTrans_1p = Monitored::Scalar<float>("RNNJetScoreSigTransAccepted_1p", -1);
    auto IDScore_mp         = Monitored::Scalar<float>("RNNJetScoreAccepted_mp", -1);
    auto IDScoreSigTrans_mp = Monitored::Scalar<float>("RNNJetScoreSigTransAccepted_mp", -1);  

    auto monitorIt = Monitored::Group(m_monTool,
                                        nInputTaus, passedCuts,
                                        ptAccepted,  nTrackAccepted, nWideTrackAccepted,
                                        IDScore_0p, IDScoreSigTrans_0p,
                                        IDScore_1p, IDScoreSigTrans_1p, 
                                        IDScore_mp, IDScoreSigTrans_mp);

    if(m_acceptAll) {
        pass = true;
        ATH_MSG_DEBUG("AcceptAll property is set: taking all events that pass the pT cut");
    } else {
        ATH_MSG_DEBUG("AcceptAll property not set: applying selection");
    }

    // Debugging location of the TauJet RoI
    ATH_MSG_DEBUG("Input RoI eta: " << input.roi->eta() << ", phi: " << input.roi->phi() << ", z: " << input.roi->zed());

    const xAOD::TauJetContainer* TauContainer = input.taujetcontainer;
    nInputTaus = TauContainer->size();
    // There should only be a single TauJet in the TauJetContainer; just in case we still run the loop
    for(const xAOD::TauJet* Tau: *TauContainer) {
        ATH_MSG_DEBUG(" New HLT TauJet candidate:");
        
        float pT = Tau->pt();
        
        //---------------------------------------------------------
        // Calibrated tau pT cut ('idperf' step)
        //---------------------------------------------------------
        ATH_MSG_DEBUG(" pT: " << pT / Gaudi::Units::GeV);

        if(!(pT > m_ptMin)) continue;
        passedCuts++;
        ptAccepted = pT / Gaudi::Units::GeV;


        //---------------------------------------------------------
        // Track counting ('perf' step)
        //---------------------------------------------------------
        int numTrack = 0, numWideTrack = 0;
        if(m_trackPtCut >= 0) {
            // Raise the track pT threshold when counting tracks in the 'perf' step, to reduce sensitivity to pileup tracks
            // Overrides the default 1 GeV cut by the InDetTrackSelectorTool used during the TauJet construction
            for(const auto* track : Tau->tracks(xAOD::TauJetParameters::TauTrackFlag::classifiedCharged)) {
	            if(track->pt() > m_trackPtCut) numTrack++;
            }
            for(const auto* track : Tau->tracks(xAOD::TauJetParameters::TauTrackFlag::classifiedIsolation)) {
	            if(track->pt() > m_trackPtCut) numWideTrack++;
            }
        } else {
            // Use the default 1 GeV selection in the InDetTrackSelectorTool, executed during the TauJet construction
            numTrack = Tau->nTracks();
            numWideTrack = Tau->nTracksIsolation();
        }

        ATH_MSG_DEBUG(" N Tracks: " << numTrack);
        ATH_MSG_DEBUG(" N Wide Tracks: " << numWideTrack);

        // Apply NTrackMin and NWideTrackMax cuts:
        if(!m_acceptAll && !(m_doHighPtSelection && pT > m_highPtTrkThr)) {
            if(!(numTrack >= m_numTrackMin)) continue;
            if(!(numWideTrack <= m_numWideTrackMax)) continue;
        }
        // Apply NTrackMax cut:
        if(!m_acceptAll && !(m_doHighPtSelection && pT > m_highPtJetThr)) {
            if(!(numTrack <= m_numTrackMax)) continue;
        }
        // Note: we disabled the track selection for high pT taus

        passedCuts++;
        nTrackAccepted = numTrack;
        nWideTrackAccepted = numWideTrack;  


        //---------------------------------------------------------
        // ID WP selection (ID step)
        //---------------------------------------------------------
        int local_idWP = m_idWP;
        
        // Loosen/disable the ID WP cut for high pT taus
        if(m_doHighPtSelection && pT > m_highPtLooseIDThr && m_idWP > IDWP::Loose) local_idWP = IDWP::Loose; // Set ID WP to Loose
        if(m_doHighPtSelection && pT > m_highPtJetThr) local_idWP = IDWP::None; // Disable the ID WP cut

        ATH_MSG_DEBUG(" Local Tau ID WP: " << local_idWP);

        if(m_idMethod == IDMethod::RNN) { // RNN/DeepSet scores
            if(!Tau->hasDiscriminant(xAOD::TauJetParameters::RNNJetScoreSigTrans)) {
                ATH_MSG_WARNING(" RNNJetScoreSigTrans not available. Make sure the TauWPDecorator too lis run for the RNN Tau ID!");
            }
        
            ATH_MSG_DEBUG(" RNNJetScoreSigTrans: " << Tau->discriminant(xAOD::TauJetParameters::RNNJetScoreSigTrans));
        
            if(!m_acceptAll && local_idWP != IDWP::None) {
                if(local_idWP == IDWP::VeryLoose && !Tau->isTau(xAOD::TauJetParameters::JetRNNSigVeryLoose)) {
                    continue;
                } else if(local_idWP == IDWP::Loose && !Tau->isTau(xAOD::TauJetParameters::JetRNNSigLoose)) {
                    continue;
                } else if(local_idWP == IDWP::Medium && !Tau->isTau(xAOD::TauJetParameters::JetRNNSigMedium)) {
                    continue;
                } else if(local_idWP == IDWP::Tight && !Tau->isTau(xAOD::TauJetParameters::JetRNNSigTight)) {
                    continue;
                }
            }

            // Monitor ID scores
            if(Tau->nTracks() == 0) {
                IDScore_0p = Tau->discriminant(xAOD::TauJetParameters::RNNJetScore);
                IDScoreSigTrans_0p = Tau->discriminant(xAOD::TauJetParameters::RNNJetScoreSigTrans);
            } else if(Tau->nTracks() == 1) {
                IDScore_1p = Tau->discriminant(xAOD::TauJetParameters::RNNJetScore);
                IDScoreSigTrans_1p = Tau->discriminant(xAOD::TauJetParameters::RNNJetScoreSigTrans);
            } else { // MP tau
                IDScore_mp = Tau->discriminant(xAOD::TauJetParameters::RNNJetScore);
                IDScoreSigTrans_mp = Tau->discriminant(xAOD::TauJetParameters::RNNJetScoreSigTrans);
            }
        }

        passedCuts++;

        
        //---------------------------------------------------------
        // At least one Tau passed all the cuts. Accept the event!
        //---------------------------------------------------------
        pass=true;

        ATH_MSG_DEBUG(" Pass hypo tool: " << pass);
    }
  
    return pass;
}


StatusCode TrigEFTauMVHypoTool::decide(std::vector<TauJetInfo>& input) const {
    for(auto& i : input) {
        if(passed(m_decisionId.numeric(), i.previousDecisionIDs)) {
            if(decide(i)) {
	            addDecisionID(m_decisionId, i.decision);
            }
        }
    }

    return StatusCode::SUCCESS;
}

