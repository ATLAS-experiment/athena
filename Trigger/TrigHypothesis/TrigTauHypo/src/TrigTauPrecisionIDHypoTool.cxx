/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "AthenaMonitoringKernel/Monitored.h"
#include "TrigCompositeUtils/TrigCompositeUtils.h"
#include "xAODTau/TauJetContainer.h"
#include "GaudiKernel/SystemOfUnits.h"

#include "TrigTauPrecisionIDHypoTool.h"


using namespace TrigCompositeUtils;

TrigTauPrecisionIDHypoTool::TrigTauPrecisionIDHypoTool(const std::string& type, const std::string& name, const IInterface* parent)
  : base_class(type, name, parent), 
    m_decisionId(HLT::Identifier::fromToolName(name))
{

}


TrigTauPrecisionIDHypoTool::~TrigTauPrecisionIDHypoTool()
{  

}


StatusCode TrigTauPrecisionIDHypoTool::initialize()
{
    ATH_MSG_DEBUG(name() << ": in initialize()");

    ATH_MSG_DEBUG("TrigTauPrecisionIDHypoTool will cut on:");
    ATH_MSG_DEBUG(" - PtMin: " << m_ptMin.value());
    ATH_MSG_DEBUG(" - NTracksMin: " << m_numTrackMin.value());
    ATH_MSG_DEBUG(" - NTracksMax: " << m_numTrackMax.value());
    ATH_MSG_DEBUG(" - NIsoTracksMax: " << m_numIsoTrackMax.value());
    if(m_trackPtCut >= 0) ATH_MSG_DEBUG(" - trackPtCut: " << m_trackPtCut.value());
    ATH_MSG_DEBUG(" - IDMethod: " << m_idMethod.value());
    ATH_MSG_DEBUG(" - IDWP: " << m_idWP.value());
    if(m_idMethod == IDMethod::Decorator) ATH_MSG_DEBUG("   - IDWPNames: " << m_idWPNames.value());
    ATH_MSG_DEBUG("   - HighPtSelectionTrkThr: " << m_highPtTrkThr.value());
    ATH_MSG_DEBUG("   - HighPtSelectionLooseIDThr: " << m_highPtLooseIDThr.value());
    ATH_MSG_DEBUG("   - HighPtSelectionJetThr: " << m_highPtJetThr.value());

    if((m_numTrackMin > m_numTrackMax) || (m_highPtLooseIDThr > m_highPtJetThr)) {
        ATH_MSG_ERROR("Invalid tool configuration!");
        return StatusCode::FAILURE;
    }

    if(!(m_idMethod == IDMethod::Disabled || m_idMethod == IDMethod::RNN || m_idMethod == IDMethod::Decorator)) {
        ATH_MSG_ERROR("Invalid IDMethod value, " << m_idMethod.value());
        return StatusCode::FAILURE;
    } else if(m_idWP < IDWP::None || m_idWP > IDWP::Tight) {
        ATH_MSG_ERROR("Invalid IDWP value, " << m_idWP.value());
        return StatusCode::FAILURE;
    } else if(m_idMethod == IDMethod::Disabled && m_idWP != IDWP::None) {
        ATH_MSG_ERROR("IDMethod=0 must be set together with IDWP=-1");
        return StatusCode::FAILURE;
    } else if(m_idMethod == IDMethod::Decorator && m_idWPNames.size() != 4) {
        ATH_MSG_ERROR("There need to be 4 TauID WPs passed to IDWPNames if using IDMethod=2 (Decorator)");
        return StatusCode::FAILURE;
    }

    // Now create the "cache" of TauID score accessors for the Monitoring...
    ATH_MSG_DEBUG("TauID score monitoring: ");
    for(const auto& [key, p] : m_monitoredIdScores) {
        ATH_MSG_DEBUG("   - IDName: " << key);
        ATH_MSG_DEBUG("      - IDScoreName: " << p.first);
        ATH_MSG_DEBUG("      - IDScoreSigTransName: " << p.second);
        if(p.first.empty() || p.second.empty()) {
            ATH_MSG_ERROR("Invalid score variable names; skipping this entry for the monitoring!");
            continue;
        }

        m_monitoredIdAccessors.emplace(key, std::make_pair(SG::ConstAccessor<float>(p.first), SG::ConstAccessor<float>(p.second)));
    }

    return StatusCode::SUCCESS;
}


bool TrigTauPrecisionIDHypoTool::decide(const ITrigTauPrecisionHypoTool::ToolInfo& input) const
{
    ATH_MSG_DEBUG(name() << ": in execute()");

    auto NInputTaus         = Monitored::Scalar<int>("NInputTaus", -1);
    auto passedCuts         = Monitored::Scalar<int>("CutCounter", 0);
    auto PtAccepted         = Monitored::Scalar<float>("PtAccepted", -1);
    auto NTracksAccepted    = Monitored::Scalar<int>("NTracksAccepted", -1);
    auto NIsoTracksAccepted = Monitored::Scalar<int>("NIsoTracksAccepted", -1);

    std::map<std::string, Monitored::Scalar<float>> monitoredIdVariables;
    for(const auto& [key, p] : m_monitoredIdScores) {
        monitoredIdVariables.emplace(key + "_TauJetScoreAccepted_0p", Monitored::Scalar<float>(key + "_TauJetScoreAccepted_0p", -1));
        monitoredIdVariables.emplace(key + "_TauJetScoreTransAccepted_0p", Monitored::Scalar<float>(key + "_TauJetScoreTransAccepted_0p", -1));
        monitoredIdVariables.emplace(key + "_TauJetScoreAccepted_1p", Monitored::Scalar<float>(key + "_TauJetScoreAccepted_1p", -1));
        monitoredIdVariables.emplace(key + "_TauJetScoreTransAccepted_1p", Monitored::Scalar<float>(key + "_TauJetScoreTransAccepted_1p", -1));
        monitoredIdVariables.emplace(key + "_TauJetScoreAccepted_mp", Monitored::Scalar<float>(key + "_TauJetScoreAccepted_mp", -1));
        monitoredIdVariables.emplace(key + "_TauJetScoreTransAccepted_mp", Monitored::Scalar<float>(key + "_TauJetScoreTransAccepted_mp", -1));
    }

    std::vector<std::reference_wrapper<Monitored::IMonitoredVariable>> monVars = {
        std::ref(NInputTaus), std::ref(passedCuts), std::ref(PtAccepted), std::ref(NTracksAccepted), std::ref(NIsoTracksAccepted)
    };
    for(auto& [key, var] : monitoredIdVariables) monVars.push_back(std::ref(var));
    auto monitorIt = Monitored::Group(m_monTool, monVars);


    // Tau pass flag
    bool pass = false;

    if(m_acceptAll) {
        pass = true;
        ATH_MSG_DEBUG("AcceptAll property is set: taking all events");
    }

    // Debugging location of the TauJet RoI
    ATH_MSG_DEBUG("Input RoI eta: " << input.roi->eta() << ", phi: " << input.roi->phi() << ", z: " << input.roi->zed());

    const xAOD::TauJetContainer* TauContainer = input.tauContainer;
    NInputTaus = TauContainer->size();
    // There should only be a single TauJet in the TauJetContainer; just in case we still run the loop
    for(const xAOD::TauJet* Tau : *TauContainer) {
        ATH_MSG_DEBUG(" New HLT TauJet candidate:");
        
        float pT = Tau->pt();
        
        //---------------------------------------------------------
        // Calibrated tau pT cut ('idperf' step)
        //---------------------------------------------------------
        ATH_MSG_DEBUG(" pT: " << pT / Gaudi::Units::GeV);

        if(!(pT > m_ptMin)) continue;
        passedCuts++;
        PtAccepted = pT / Gaudi::Units::GeV;


        //---------------------------------------------------------
        // Track counting ('perf' step)
        //---------------------------------------------------------
        int numTrack = 0, numIsoTrack = 0;
        if(m_trackPtCut > 0.) {
            // Raise the track pT threshold when counting tracks in the 'perf' step, to reduce sensitivity to pileup tracks
            // Overrides the default 1 GeV cut by the InDetTrackSelectorTool used during the TauJet construction
            for(const auto* track : Tau->tracks(xAOD::TauJetParameters::TauTrackFlag::classifiedCharged)) {
	      if(track->pt() > m_trackPtCut) numTrack++;
            }
            for(const auto* track : Tau->tracks(xAOD::TauJetParameters::TauTrackFlag::classifiedIsolation)) {
	      if(track->pt() > m_trackPtCut) numIsoTrack++;
            }
        } else {
            // Use the default 1 GeV selection in the InDetTrackSelectorTool, executed during the TauJet construction
            numTrack = Tau->nTracks();
            numIsoTrack = Tau->nTracksIsolation();
        }

        ATH_MSG_DEBUG(" N Tracks: " << numTrack);
        ATH_MSG_DEBUG(" N Iso Tracks: " << numIsoTrack);

        // Apply track multiplicity cuts, except for idperf
        if(!m_acceptAll) {
	  // NTrackMin and NIsoTracksMax
	  if(pT < m_highPtTrkThr) {
	    if(numTrack < m_numTrackMin) continue;
            if(numIsoTrack > m_numIsoTrackMax) continue;
	  }
	  // NTrackMax
	  if(pT < m_highPtJetThr) {
            if(numTrack > m_numTrackMax) continue;
	  }
	}
        // Note: we disabled the track selection for high pT taus

        passedCuts++;
        NTracksAccepted = numTrack;
        NIsoTracksAccepted = numIsoTrack;  


        //---------------------------------------------------------
        // ID WP selection (ID step)
        //---------------------------------------------------------
        int local_idWP = m_idWP;
        
        // Loosen/disable the ID WP cut for high pT taus
        if(pT > m_highPtLooseIDThr && m_idWP > IDWP::Loose) local_idWP = IDWP::Loose; // Set ID WP to Loose
        if(pT > m_highPtJetThr) local_idWP = IDWP::None; // Disable the ID WP cut

        ATH_MSG_DEBUG(" Local Tau ID WP: " << local_idWP);

        if(m_idMethod == IDMethod::RNN) { // RNN/DeepSet scores
            if(!Tau->hasDiscriminant(xAOD::TauJetParameters::RNNJetScoreSigTrans)) {
                ATH_MSG_WARNING(" RNNJetScoreSigTrans not available. Make sure the TauWPDecorator is run for the RNN Tau ID!");
            }

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

        } else if(m_idMethod == IDMethod::Decorator) { // Decorated scores (e.g. for GNTau)
            const static SG::ConstAccessor<char> tauid_veryloose(m_idWPNames[0]);
            const static SG::ConstAccessor<char> tauid_loose(m_idWPNames[1]);
            const static SG::ConstAccessor<char> tauid_medium(m_idWPNames[2]);
            const static SG::ConstAccessor<char> tauid_tight(m_idWPNames[3]);

            if(!tauid_veryloose.isAvailable(*Tau) || !tauid_loose.isAvailable(*Tau) || !tauid_medium.isAvailable(*Tau) || !tauid_tight.isAvailable(*Tau))
            ATH_MSG_WARNING("The TauID WP variables for the current configuration are missing! Make sure the correct inferences are included in the chain reconstruction sequence!");

            if(!m_acceptAll && local_idWP != IDWP::None) {
                if(local_idWP == IDWP::VeryLoose && !tauid_veryloose(*Tau)) {
                    continue;
                } else if(local_idWP == IDWP::Loose && !tauid_loose(*Tau)) {
                    continue;
                } else if(local_idWP == IDWP::Medium && !tauid_medium(*Tau)) {
                    continue;
                } else if(local_idWP == IDWP::Tight && !tauid_tight(*Tau)) {
                    continue;
                }
            }


        }

        // TauID Score monitoring
        for(const auto& [key, p] : m_monitoredIdAccessors) {
            if(!p.first.isAvailable(*Tau))
	      ATH_MSG_WARNING("TauID Score " << m_monitoredIdScores.value().at(key).first << " is not available. Make sure the correct inferences are included in the chain reconstruction sequence!");

            if(!p.second.isAvailable(*Tau))
	      ATH_MSG_WARNING("TauID ScoreSigTrans " << m_monitoredIdScores.value().at(key).second << " is not available. Make sure the correct inferences are included in the chain reconstruction sequence!");

            ATH_MSG_DEBUG(" TauID \"" << key << "\" ScoreSigTrans: " << p.second(*Tau));

            // Monitor ID scores
            if(Tau->nTracks() == 0) {
                monitoredIdVariables.at(key + "_TauJetScoreAccepted_0p") = p.first(*Tau);
                monitoredIdVariables.at(key + "_TauJetScoreTransAccepted_0p") = p.second(*Tau);
            } else if(Tau->nTracks() == 1) {
                monitoredIdVariables.at(key + "_TauJetScoreAccepted_1p") = p.first(*Tau);
                monitoredIdVariables.at(key + "_TauJetScoreTransAccepted_1p") = p.second(*Tau);
            } else { // MP tau
                monitoredIdVariables.at(key + "_TauJetScoreAccepted_mp") = p.first(*Tau);
                monitoredIdVariables.at(key + "_TauJetScoreTransAccepted_mp") = p.second(*Tau);
            }
        }

        passedCuts++;

        
        //---------------------------------------------------------
        // At least one Tau passed all the cuts. Accept the event!
        //---------------------------------------------------------
        pass = true;

        ATH_MSG_DEBUG(" Pass hypo tool: " << pass);
    }
  
    return pass;
}


StatusCode TrigTauPrecisionIDHypoTool::decide(std::vector<ITrigTauPrecisionHypoTool::ToolInfo>& input) const {
    for(auto& i : input) {
        if(passed(m_decisionId.numeric(), i.previousDecisionIDs)) {
            if(decide(i)) {
	            addDecisionID(m_decisionId, i.decision);
            }
        }
    }

    return StatusCode::SUCCESS;
}

