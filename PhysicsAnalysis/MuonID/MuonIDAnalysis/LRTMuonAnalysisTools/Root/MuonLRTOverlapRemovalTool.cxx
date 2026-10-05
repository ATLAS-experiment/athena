/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
//   MuonLRTOverlapRemovalTool
//   Author: Max Goblirsch, goblirsc@SPAMNOT_CERN.ch

#include "LRTMuonAnalysisTools/MuonLRTOverlapRemovalTool.h"
#include <algorithm>
#include <AsgTools/AsgToolConfig.h>

namespace CP {

  MuonLRTOverlapRemovalTool::MuonLRTOverlapRemovalTool(const std::string& name) :
    asg::AsgTool(name)
  {
    // nothing to do here
  }

  ///////////////////////////////////////////////////////////////////
  // Initialisation
  ///////////////////////////////////////////////////////////////////

  StatusCode MuonLRTOverlapRemovalTool::initialize()
  {
    if(m_muonSelectionTool.empty()){
      asg::AsgToolConfig config("CP::MuonSelectionTool/MuonSelectionTool");
      ATH_CHECK(config.setProperty("TurnOffMomCorr",true));
      ATH_CHECK(config.setProperty("IsRun3Geo",m_useRun3WP.value()));
      ATH_CHECK(config.makePrivateTool(m_muonSelectionTool));
    }

    ATH_MSG_DEBUG("retrieving Muon selection tool");
    ATH_CHECK( m_muonSelectionTool.retrieve() );
    return StatusCode::SUCCESS;
  }

  //////////////////////////////////////////////////////////////////////
  // Check overlap between the muon collections and decorate duplicates
  //////////////////////////////////////////////////////////////////////
  void MuonLRTOverlapRemovalTool::checkOverlap( const xAOD::MuonContainer & promptMuonCol,
                                                const xAOD::MuonContainer & LRTMuonCol,
                                                std::vector<bool>& promptMuonsSelectedToKeep,
                                                std::vector<bool>& lrtMuonsSelectedToKeep ) const
  {

    /// pre-fill vectors with the default 'accept' decision
    promptMuonsSelectedToKeep.resize(promptMuonCol.size(), true); 
    lrtMuonsSelectedToKeep.resize(LRTMuonCol.size(), true); 
    std::fill(promptMuonsSelectedToKeep.begin(), promptMuonsSelectedToKeep.end(), true); 
    std::fill(lrtMuonsSelectedToKeep.begin(), lrtMuonsSelectedToKeep.end(), true); 

    /// pre-fill vectors with the default '0 = no overlap decision'
    std::vector<int> promptMuonsOverlapDecision, lrtMuonsOverlapDecision;
    promptMuonsOverlapDecision.resize(promptMuonCol.size(), 0);
    lrtMuonsOverlapDecision.resize(LRTMuonCol.size(), 0);

    // loop over prompt muons
    u_int promptMuonIndex = 0;
    for (const xAOD::Muon* promptMuon : promptMuonCol){

      // loop over LRT muons
      u_int lrtMuonIndex = 0;
      for( const xAOD::Muon* lrtMuon : LRTMuonCol){
        // check for overlap
        std::pair<bool,bool> writeDecision = {true,true};
        switch(m_strategy){
          case CP::IMuonLRTOverlapRemovalTool::defaultStrategy:
            /// Baseline strategy
            if(hasOverlap(promptMuon,lrtMuon)) { writeDecision = resolveOverlap(promptMuon, lrtMuon); }
            break;
          case CP::IMuonLRTOverlapRemovalTool::passThroughAndDecorate:
            /// passThroughAndDecorate strategy
            if ( (promptMuonsOverlapDecision.at(promptMuonIndex) == 0) && (lrtMuonsOverlapDecision.at(lrtMuonIndex) == 0) ) {
              // overwrite the decision only if no overlaps have been found yet. Do not check again if either of the leptons have found overlaps previously.
              std::tie(promptMuonsOverlapDecision.at(promptMuonIndex), lrtMuonsOverlapDecision.at(lrtMuonIndex)) = checkOverlapForDecor(promptMuon, lrtMuon);
            }
            break;
          default:
            ATH_MSG_FATAL("Unsupported overlap removal strategy type. Choose from 0 (`defaultStrategy`) or 1 (`passThroughAndDecorate`)");
            break;
        }
        // write decision into vectors
        if(!writeDecision.first){
          promptMuonsSelectedToKeep.at(promptMuon->index()) = false;
        }
        if(!writeDecision.second){
          lrtMuonsSelectedToKeep.at(lrtMuon->index()) = false;
        }
        ++lrtMuonIndex;
      } // LRT muon loop ends
      ++promptMuonIndex;
    } // prompt muon loop ends

    if (m_strategy == CP::IMuonLRTOverlapRemovalTool::passThroughAndDecorate) {
      // if the passThroughAndDecorate strategy is selected, run a final loop over the collections to decorate the muons with the overlap resolution result.
      static const SG::Decorator<int> MuonLRTOverlapDecision("MuonLRTOverlapDecision"); //0 if no overlap, 1 if overlaps and rejected, 2 if overlaps and retained
      //final loop over prompt muons
      u_int promptMuonIndex = 0;
      for (const xAOD::Muon* promptMuon : promptMuonCol){
        MuonLRTOverlapDecision(*promptMuon) = promptMuonsOverlapDecision.at(promptMuonIndex);
        ++promptMuonIndex;
      }
      //final loop over LRT muons
      u_int lrtMuonIndex = 0;
      for (const xAOD::Muon* lrtMuon : LRTMuonCol){
        MuonLRTOverlapDecision(*lrtMuon) = lrtMuonsOverlapDecision.at(lrtMuonIndex);
        ++lrtMuonIndex;
      }
    }

  }

  bool MuonLRTOverlapRemovalTool::hasOverlap(const xAOD::Muon* promptMuon,
                                             const xAOD::Muon* lrtMuon) const{

    // we compare based on MS track information to detect re-use of the same track  
    using enum xAOD::Muon::TrackParticleType;
    const xAOD::TrackParticle* lrtMsTrack = lrtMuon->trackParticle( MuonSpectrometerTrackParticle );
    const xAOD::TrackParticle* promptMsTrack = promptMuon->trackParticle( MuonSpectrometerTrackParticle );

    // baseline case: if no two MS tracks or two different MS tracks, no overlap possible 
    if ( (!promptMsTrack && !lrtMsTrack) || (promptMsTrack != lrtMsTrack)){
      return false;
    }

    else {
      ATH_MSG_DEBUG("Found an overlap, solving");
      ATH_MSG_DEBUG("  Prompt muon has author "<< promptMuon->author()<<", type "<<promptMuon->muonType()<<", pT "<<promptMuon->pt()<<", eta "<<promptMuon->eta()<<", phi "<<promptMuon->phi());
      ATH_MSG_DEBUG("  LRT muon has author "<< lrtMuon->author()<<", type "<<lrtMuon->muonType()<<", pT "<<lrtMuon->pt()<<", eta "<<lrtMuon->eta()<<", phi "<<lrtMuon->phi());
      return true;
    }
  }

  std::pair<bool, bool> MuonLRTOverlapRemovalTool::resolveOverlap(const xAOD::Muon* promptMuon,
                                                                  const xAOD::Muon* lrtMuon) const{

    // apply the loosest available ID to resolve most overlaps using existing MCP recommendations
    bool promptPassQuality = (m_muonSelectionTool->getQuality(*promptMuon) < xAOD::Muon::Quality::VeryLoose);
    bool lrtPassQuality = (m_muonSelectionTool->getQuality(*lrtMuon) < xAOD::Muon::Quality::VeryLoose);

    if (promptPassQuality && !lrtPassQuality) { 
      return {true,false}; 
    }
    else if (!promptPassQuality && lrtPassQuality) {
      return {false,true};
    }

    // still here? Next prefer combined muons over others 
    bool promptIsCombined = promptMuon->muonType() == xAOD::Muon::MuonType::Combined;
    bool lrtIsCombined = lrtMuon->muonType() == xAOD::Muon::MuonType::Combined;

    if (promptIsCombined && !lrtIsCombined) {
      return {true,false};
    }
    else if (!promptIsCombined && lrtIsCombined) {
      return {false,true};
    }

    // still here? Next choose the muon with a lower ID-ME delta eta value
    float promptIDMEdEta = getIDMEdEta(promptMuon);
    float lrtIDMEdEta    = getIDMEdEta(lrtMuon);

    if (promptIDMEdEta <= lrtIDMEdEta) {
      return {true,false};
    }
    else {
      return {false,true};
    }

    // fail-safe case: choose prompt over LRT.
    ATH_MSG_DEBUG("Resolution reached the fail-safe point. Why?");
    return {true,false};
  }

  std::tuple<int, int> MuonLRTOverlapRemovalTool::checkOverlapForDecor(const xAOD::Muon* promptMuon,
                                                       const xAOD::Muon* lrtMuon) const{
    //return values: 0 if no overlap, 1 if overlaps and rejected, 2 if overlaps and retained.

    if (!hasOverlap(promptMuon, lrtMuon)){
      return std::make_tuple(0, 0);
    }
    else {
      std::pair<bool, bool> overlapDecision = resolveOverlap(promptMuon, lrtMuon);
      if (overlapDecision.first && !overlapDecision.second) {
        return std::make_tuple(2, 1);
      }
      else {
        return std::make_tuple(1, 2);
      }
    }
  }

  float MuonLRTOverlapRemovalTool::getIDMEdEta(const xAOD::Muon* muon) const{
    const xAOD::TrackParticle* ID_track = muon->trackParticle(xAOD::Muon::TrackParticleType::InnerDetectorTrackParticle);
    const xAOD::TrackParticle* ME_track = muon->trackParticle(xAOD::Muon::TrackParticleType::ExtrapolatedMuonSpectrometerTrackParticle);
    if (!ID_track || !ME_track) return FLT_MAX;
    return ( std::abs( ID_track->eta() - ME_track->eta() ) );
  }

} // end namespace CP
