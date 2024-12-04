/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// This source file implements all of the functions related to Muons
// in the SUSYObjDef_xAOD class

// Local include(s):
#include "SUSYTools/SUSYObjDef_xAOD.h"

#include "xAODBase/IParticleHelpers.h"
#include "EventPrimitives/EventPrimitivesHelpers.h"
#include "FourMomUtils/xAODP4Helpers.h"
#include "xAODTracking/TrackParticlexAODHelpers.h"
#include "AthContainers/ConstDataVector.h"

#include "AsgAnalysisInterfaces/IPileupReweightingTool.h"

#include "MuonAnalysisInterfaces/IMuonCalibrationAndSmearingTool.h"
#include "MuonAnalysisInterfaces/IMuonEfficiencyScaleFactors.h"
#include "MuonAnalysisInterfaces/IMuonSelectionTool.h"
#include "MuonAnalysisInterfaces/IMuonTriggerScaleFactors.h"
#include "MuonAnalysisInterfaces/IMuonLRTOverlapRemovalTool.h"
#include "xAODMuon/MuonAuxContainer.h"

#include "IsolationCorrections/IIsolationCorrectionTool.h"
#include "IsolationSelection/IIsolationSelectionTool.h"
//disable  #include "IsolationSelection/IIsolationLowPtPLVTool.h"

#include "TriggerAnalysisInterfaces/ITrigGlobalEfficiencyCorrectionTool.h"

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/replace.hpp>

#ifndef XAOD_STANDALONE // For now metadata is Athena-only
#include "AthAnalysisBaseComps/AthAnalysisHelper.h"
#endif

namespace ST {

  const static SG::Decorator<char>      dec_passedHighPtCuts("passedHighPtCuts");

  const static SG::Decorator<char>      dec_passSignalID("passSignalID");
  const static SG::ConstAccessor<char>  acc_passSignalID("passSignalID");

  const static SG::Decorator<float>     dec_DFCommonJetDr("DFCommonJetDr");
  const static SG::ConstAccessor<float> acc_DFCommonJetDr("DFCommonJetDr");
  const static SG::Decorator<float>     dec_dRJet("dRJet");
  const static SG::Decorator<float>     dec_z0sinTheta("z0sinTheta");
  const static SG::Decorator<float>     dec_d0sig("d0sig");
  const static SG::Decorator<char>      dec_isLRT("isLRT");
  const static SG::Decorator<char>      dec_cosmic("cosmic");


StatusCode SUSYObjDef_xAOD::MergeMuons(const xAOD::MuonContainer & muons, const std::vector<bool> &writeMuon, xAOD::MuonContainer* outputCol) const{
    if (muons.empty()) return StatusCode::SUCCESS;
    for (const xAOD::Muon* muon: muons) {
        // add muon into output 
        if (writeMuon.at(muon->index())){
            auto newMuon = new xAOD::Muon(*muon);

            if ( getOriginalObject(*muon) != nullptr ) {
              setOriginalObjectLink(*getOriginalObject(*muon), *newMuon);
            } else {
              setOriginalObjectLink(*muon, *newMuon);
            }
            outputCol->push_back(newMuon); 
        }
    }
    return StatusCode::SUCCESS;
}

StatusCode SUSYObjDef_xAOD::prepareLRTMuons(const xAOD::MuonContainer* inMuons, xAOD::MuonContainer* copy) const{
  for (const xAOD::Muon *muon: *inMuons){
    const xAOD::TrackParticle* idtrack = muon->trackParticle(xAOD::Muon::InnerDetectorTrackParticle);

    // Save muon if the id track passes the LRT filter
    if ( acc_lrtFilter.isAvailable(*idtrack) )
    {
      if ( static_cast<int>(acc_lrtFilter(*idtrack) ) ){ 
        std::unique_ptr<xAOD::Muon> copyMuon = std::make_unique<xAOD::Muon>(*muon);

        // transfer original muon link
        setOriginalObjectLink(*muon, *copyMuon);
        copy->push_back( std::move(copyMuon) );
      } 
    }
    else // Keep muon if flag is not available
    {  
      std::unique_ptr<xAOD::Muon> copyMuon = std::make_unique<xAOD::Muon>(*muon);

      setOriginalObjectLink(*muon, *copyMuon);
      copy->push_back( std::move(copyMuon) );
    }
    
  }
    return StatusCode::SUCCESS;
}

StatusCode SUSYObjDef_xAOD::GetMuons(xAOD::MuonContainer*& copy, xAOD::ShallowAuxContainer*& copyaux, bool recordSG, const std::string& muonkey, const std::string& lrtmuonkey, const xAOD::MuonContainer* containerToBeCopied)
{
  if (!m_tool_init) {
    ATH_MSG_ERROR("SUSYTools was not initialized!!");
    return StatusCode::FAILURE;
  }

  // Initializing prompt/LRT OR procedure
  auto outputCol = std::make_unique<xAOD::MuonContainer>();
  std::unique_ptr<xAOD::MuonAuxContainer> outputAuxCol;
  outputAuxCol = std::make_unique<xAOD::MuonAuxContainer>();
  outputCol->setStore(outputAuxCol.get());
  ATH_CHECK( m_outMuonLocation.initialize() );

  if (bool(m_muLRT) && !lrtmuonkey.empty() && evtStore()->contains<xAOD::MuonContainer>(lrtmuonkey)){
    ATH_MSG_DEBUG("Applying prompt/LRT muon OR procedure"); 

    // First identify if merged container has already been made (for instances where GetMuons() is called more than once)
    if (evtStore()->contains<xAOD::MuonContainer>("StdWithLRTMuons")) {
      ATH_MSG_DEBUG("Merged prompt/LRT container already created in TStore");  
    } else {
      ATH_MSG_DEBUG("Creating merged prompt/LRT container in TStore");

      // Retrieve prompt and LRT muons from TStore
      ATH_CHECK( evtStore()->retrieve(prompt_muons, muonkey) );
      ATH_CHECK( evtStore()->retrieve(lrt_muons, lrtmuonkey) );

      // Remove LRT muons as flagged by filter for uncertainty
      auto filtered_muons = std::make_unique<xAOD::MuonContainer>();
      std::unique_ptr<xAOD::MuonAuxContainer> filtered_muons_aux = std::make_unique<xAOD::MuonAuxContainer>();
      filtered_muons->setStore(filtered_muons_aux.get());
      ATH_CHECK(prepareLRTMuons(lrt_muons, filtered_muons.get()));

      // Check overlap between prompt and LRT collections
      std::vector<bool> writePromptMuon;
      std::vector<bool> writeLRTMuon;
      m_muonLRTORTool->checkOverlap(*prompt_muons, *filtered_muons, writePromptMuon, writeLRTMuon);
    
      // Decorate muons with prompt/LRT
      for (const xAOD::Muon* mu : *prompt_muons)   dec_isLRT(*mu) = 0;
      for (const xAOD::Muon* mu : *filtered_muons) dec_isLRT(*mu) = 1;

      // Create merged StdWithLRTMuons container
      outputCol->reserve(prompt_muons->size() + filtered_muons->size());
      ATH_CHECK(MergeMuons(*prompt_muons, writePromptMuon, outputCol.get()) );
      ATH_CHECK(MergeMuons(*filtered_muons, writeLRTMuon, outputCol.get()) );

      // Save merged StdWithLRTMuons container to TStore
      ATH_CHECK(evtStore()->record(std::move(outputCol), m_outMuonLocation.key())); 
      ATH_CHECK(evtStore()->record(std::move(outputAuxCol), m_outMuonLocation.key() + "Aux.") );

    }
  } else if (!lrtmuonkey.empty()) {
    if (evtStore()->contains<xAOD::MuonContainer>(lrtmuonkey) == false && bool(m_muLRT) == true) ATH_MSG_WARNING("prompt/LRT OR procedure attempted but " << lrtmuonkey << " not in ROOT file, check config!");
    ATH_MSG_DEBUG("Not applying prompt/LRT muon OR procedure"); 
  }
  
  if (m_isPHYSLITE && muonkey.find("AnalysisMuons")==std::string::npos){
    ATH_MSG_ERROR("You are running on PHYSLITE derivation. Please change the Muons container to 'AnalysisMuons'");
    return StatusCode::FAILURE;
  }

  const xAOD::MuonContainer* muons = nullptr;
  if (bool(m_muLRT) && evtStore()->contains<xAOD::MuonContainer>(lrtmuonkey)){
      ATH_MSG_DEBUG("Using container: " << m_outMuonLocation.key());
      ATH_CHECK( evtStore()->retrieve(muons, m_outMuonLocation.key())); 
  }
  else { 
    if (copy==nullptr) { // empty container provided
        ATH_MSG_DEBUG("Empty container provided");
      if (containerToBeCopied != nullptr) {
        ATH_MSG_DEBUG("Containter to be copied not nullptr");
        muons = containerToBeCopied;
      }
      else {
        ATH_MSG_DEBUG("Getting Muons collection");
        ATH_CHECK( evtStore()->retrieve(muons, muonkey) );
      }
    }
  }

  std::pair<xAOD::MuonContainer*, xAOD::ShallowAuxContainer*> shallowcopy = xAOD::shallowCopyContainer(*muons);
  copy = shallowcopy.first;
  copyaux = shallowcopy.second;
  bool setLinks = xAOD::setOriginalObjectLink(*muons, *copy);
  if (!setLinks) {
    ATH_MSG_WARNING("Failed to set original object links on " << muonkey);
  } else { // use the user-supplied collection instead 
      ATH_MSG_DEBUG("Not retrieving muon collection, using existing one provided by user");
      muons=copy;
  }

  for (const auto& muon : *copy) {
    ATH_CHECK( this->FillMuon(*muon, m_muBaselinePt, m_muBaselineEta) );
    this->IsSignalMuon(*muon, m_muPt, m_mud0sig, m_muz0, m_muEta);
    this->IsCosmicMuon(*muon, m_muCosmicz0, m_muCosmicd0);
    this->IsBadMuon(*muon, m_badmuQoverP);
  }
  if (recordSG) {
    ATH_CHECK( evtStore()->record(copy, "STCalib" + muonkey + m_currentSyst.name()) );
    ATH_CHECK( evtStore()->record(copyaux, "STCalib" + muonkey + m_currentSyst.name() + "Aux.") );
  }
  return StatusCode::SUCCESS;
}

StatusCode SUSYObjDef_xAOD::FillMuon(xAOD::Muon& input, float ptcut, float etacut) {
  
  ATH_MSG_VERBOSE( "Starting FillMuon on mu with pt=" << input.pt() );
  
  dec_baseline(input) = false;
  dec_selected(input) = 0;
  dec_signal(input) = false;
  dec_isol(input) = false;
  dec_isolHighPt(input) = false;
  dec_passedHighPtCuts(input) = false;
  dec_passSignalID(input) = false;

  if (m_muEffCorrForce1D) {
    dec_DFCommonJetDr(input) = -2.0;
  } else if (!acc_DFCommonJetDr.isAvailable(input)) {
    dec_dRJet(input) = -2.0;
  }

  // don't bother calibrating or computing WP
  if ( input.pt() < 3e3 ) return StatusCode::SUCCESS;

  ATH_MSG_VERBOSE( "MUON pt before calibration " << input.pt() );

  ATH_MSG_VERBOSE( "MUON eta  = " << input.eta() );
  ATH_MSG_VERBOSE( "MUON type = " << input.muonType() );
  ATH_MSG_VERBOSE( "MUON author = " << input.author() );

  if (m_muonCalibTool->applyCorrection( input ) == CP::CorrectionCode::OutOfValidityRange){
    ATH_MSG_VERBOSE("FillMuon: applyCorrection out of validity range");
  }

  ATH_MSG_VERBOSE( "MUON pt after calibration " << input.pt() );

  const xAOD::EventInfo* evtInfo = nullptr;
  ATH_CHECK( evtStore()->retrieve( evtInfo, "EventInfo" ) );
  const xAOD::Vertex* pv = this->GetPrimVtx();
  double primvertex_z = pv ? pv->z() : 0;
  //const xAOD::TrackParticle* track = input.primaryTrackParticle();
  const xAOD::TrackParticle* track;
  if (input.muonType() == xAOD::Muon::SiliconAssociatedForwardMuon) {
    track = input.trackParticle(xAOD::Muon::CombinedTrackParticle);
    if (!track) return StatusCode::SUCCESS; // don't treat SAF muons without CB track further  
  }
  else {
    track = input.primaryTrackParticle();
  }

  //impact parameters (after applyCorrection() so to have the primaryTrack links restored in old buggy samples)
  if (track){
    dec_z0sinTheta(input) = (track->z0() + track->vz() - primvertex_z) * TMath::Sin(input.p4().Theta());
  } else {
    ATH_MSG_WARNING("FillMuon: Muon of pT and eta " << input.pt() << " MeV " << input.eta() << " has no associated track");
  }
  //protect against exception thrown for null or negative d0sig
  try {
    if (track)
      dec_d0sig(input) = xAOD::TrackingHelpers::d0significance( track , evtInfo->beamPosSigmaX(), evtInfo->beamPosSigmaY(), evtInfo->beamPosSigmaXY() );
    else
      dec_d0sig(input) = -99.;
  }
  catch (...) {
    float d0sigError = -99.; 
    ATH_MSG_WARNING("FillMuon : Exception caught from d0significance() calculation. Setting dummy decoration d0sig=" << d0sigError );
    dec_d0sig(input) = d0sigError;
  }
  
  if (m_debug) {
    // Summary variables in
    // /cvmfs/atlas.cern.ch/repo/sw/ASG/AnalysisBase/2.0.3/xAODTracking/Root/TrackSummaryAccessors_v1.cxx

    unsigned char nBLHits(0), nPixHits(0), nPixelDeadSensors(0), nPixHoles(0),
      nSCTHits(0), nSCTDeadSensors(0), nSCTHoles(0), nTRTHits(0), nTRTOutliers(0);

    if (track){
      track->summaryValue( nBLHits, xAOD::numberOfBLayerHits);
      track->summaryValue( nPixHits, xAOD::numberOfPixelHits);
      track->summaryValue( nPixelDeadSensors, xAOD::numberOfPixelDeadSensors);
      track->summaryValue( nPixHoles, xAOD::numberOfPixelHoles);

      track->summaryValue( nSCTHits, xAOD::numberOfSCTHits);
      track->summaryValue( nSCTDeadSensors, xAOD::numberOfSCTDeadSensors);
      track->summaryValue( nSCTHoles, xAOD::numberOfSCTHoles);

      track->summaryValue( nTRTHits, xAOD::numberOfTRTHits);
      track->summaryValue( nTRTOutliers, xAOD::numberOfTRTOutliers);
    }

    ATH_MSG_INFO( "MUON pt:   " << input.pt() );
    ATH_MSG_INFO( "MUON eta:  " << input.eta() );
    ATH_MSG_INFO( "MUON phi:  " << input.phi() );
    ATH_MSG_INFO( "MUON comb: " << (input.muonType() == xAOD::Muon::Combined));
    ATH_MSG_INFO( "MUON sTag: " << (input.muonType() == xAOD::Muon::SegmentTagged));
    ATH_MSG_INFO( "MUON loose:" << (input.quality() == xAOD::Muon::Loose));
    ATH_MSG_INFO( "MUON bHit: " << static_cast<int>( nBLHits ));
    ATH_MSG_INFO( "MUON pHit: " << static_cast<int>( nPixHits ));
    ATH_MSG_INFO( "MUON pDead:" << static_cast<int>( nPixelDeadSensors ));
    ATH_MSG_INFO( "MUON pHole:" << static_cast<int>( nPixHoles ));
    ATH_MSG_INFO( "MUON sHit: " << static_cast<int>( nSCTHits));
    ATH_MSG_INFO( "MUON sDead:" << static_cast<int>( nSCTDeadSensors ));
    ATH_MSG_INFO( "MUON sHole:" << static_cast<int>( nSCTHoles ));
    ATH_MSG_INFO( "MUON tHit: " << static_cast<int>( nTRTHits ));
    ATH_MSG_INFO( "MUON tOut: " << static_cast<int>( nTRTOutliers ));

    const xAOD::TrackParticle* idtrack =
      input.trackParticle( xAOD::Muon::InnerDetectorTrackParticle );

    if ( !idtrack) {
      ATH_MSG_VERBOSE( "No ID track!! " );
    } else {
      ATH_MSG_VERBOSE( "ID track pt: "  << idtrack->pt());
    }
  }
  
  if ( !m_force_noMuId && !m_muonSelectionToolBaseline->accept(input)) return StatusCode::SUCCESS;
  
  if (input.pt() <= ptcut || std::abs(input.eta()) >= etacut) return StatusCode::SUCCESS;

  if (m_mubaselinez0>0. && std::abs(acc_z0sinTheta(input))>m_mubaselinez0) return StatusCode::SUCCESS;
  if (m_mubaselined0sig>0. && std::abs(acc_d0sig(input))>m_mubaselined0sig) return StatusCode::SUCCESS;

  //--- Do baseline isolation check
  if ( !( m_muBaselineIso_WP.empty() ) &&  !( m_isoBaselineTool->accept(input) ) ) return StatusCode::SUCCESS;

  dec_baseline(input) = true;
  dec_selected(input) = 2;

  //disable  if (!m_muIso_WP.empty() && m_muIso_WP.find("PLV")!=std::string::npos) ATH_CHECK( m_isoToolLowPtPLV->augmentPLV(input) );
  if (!m_muIso_WP.empty()) dec_isol(input) = bool(m_isoTool->accept(input));
  if (!m_muIsoHighPt_WP.empty()) dec_isolHighPt(input) = bool(m_isoHighPtTool->accept(input));
  dec_passSignalID(input) = bool(m_muonSelectionTool->accept(input));
  
  ATH_MSG_VERBOSE("FillMuon: passed baseline selection");
  return StatusCode::SUCCESS;
}
  

bool SUSYObjDef_xAOD::IsSignalMuon(const xAOD::Muon & input, float ptcut, float d0sigcut, float z0cut, float etacut) const
{
  if (!acc_baseline(input)) return false;
  if (!acc_passSignalID(input)) return false;

  if (input.pt() <= ptcut || input.pt() == 0) return false; // pT cut (might be necessary for leading muon to pass trigger)
  if ( etacut==DUMMYDEF ){
    if(std::abs(input.eta()) > m_muEta ) return false;
  }
  else if ( std::abs(input.eta()) > etacut ) return false;

  if (z0cut > 0.0 && std::abs(acc_z0sinTheta(input)) > z0cut) return false; // longitudinal IP cut
  if (acc_d0sig(input) != 0) {
    if (d0sigcut > 0.0 && std::abs(acc_d0sig(input)) > d0sigcut) return false; // transverse IP cut
  }

  if (m_doMuIsoSignal) {
    if ( !( (acc_isol(input) && input.pt()<m_muIsoHighPtThresh) || (acc_isolHighPt(input) && input.pt()>m_muIsoHighPtThresh)) ) return false;
    ATH_MSG_VERBOSE( "IsSignalMuon: passed isolation");
  } 

  //set HighPtMuon decoration
  IsHighPtMuon(input);

  dec_signal(input) = true;

  if (m_muId == 4) { //i.e. HighPt muons
    ATH_MSG_VERBOSE( "IsSignalMuon: mu pt "   << input.pt()
                     << " signal? "           << static_cast<int>(acc_signal(input))
                     << " isolation? "        << static_cast<int>(acc_isol(input))
                     << " passedHighPtCuts? " << static_cast<int>(acc_passedHighPtCuts(input)));
  } else {
    ATH_MSG_VERBOSE( "IsSignalMuon: mu pt "   << input.pt()
                     << " signal? "           << static_cast<int>( acc_signal(input))
                     << " isolation? "        << static_cast<int>( acc_isol(input)));
    // Don't show HighPtFlag ... we didn't set it!
  }

  return acc_signal(input);
}


bool SUSYObjDef_xAOD::IsHighPtMuon(const xAOD::Muon& input) const
// See https://indico.cern.ch/event/371499/contribution/1/material/slides/0.pdf and 
//     https://indico.cern.ch/event/397325/contribution/19/material/slides/0.pdf and 
//     https://twiki.cern.ch/twiki/bin/view/Atlas/MuonSelectionTool
{
  if (input.pt() < 3e3){
    ATH_MSG_DEBUG("No HighPt check supported for muons below 3GeV! False.");
    dec_passedHighPtCuts(input) = false;
    return false;
  }

  bool isHighPt=false;
  isHighPt = bool(m_muonSelectionHighPtTool->accept(input));
  dec_passedHighPtCuts(input) = isHighPt;

  return isHighPt;
}


bool SUSYObjDef_xAOD::IsBadMuon(const xAOD::Muon& input, float qopcut) const
{
  const static SG::Decorator<char> dec_bad("bad");
  dec_bad(input) = false;

  const static SG::Decorator<char> dec_bad_highPt("bad_highPt");
  dec_bad_highPt(input) = false;

  const xAOD::TrackParticle* track;
  if (input.muonType() == xAOD::Muon::SiliconAssociatedForwardMuon) {
    track = input.trackParticle(xAOD::Muon::CombinedTrackParticle);
    if (!track) return false; // don't treat SAF muons without CB track further
  }
  else{
    track = input.primaryTrackParticle();
    if (!track){
      ATH_MSG_WARNING("Non-SAF muon without a track; cannot test IsBadMuon criteria");
      return false;
    }
  }

  float Rerr = Amg::error(track->definingParametersCovMatrix(), 4) / std::abs(track->qOverP());
  ATH_MSG_VERBOSE( "Track momentum error (%): " << Rerr * 100 );
  bool isbad = Rerr > qopcut;
  bool isbadHighPt = Rerr > qopcut;

  //new recommendation from MCP
  isbad |= m_muonSelectionTool->isBadMuon(input);

  //new recommendation from MCP (at HighPT)
  isbadHighPt |= m_muonSelectionHighPtTool->isBadMuon(input);

  dec_bad(input) = isbad;
  dec_bad_highPt(input) = isbadHighPt;

  ATH_MSG_VERBOSE( "MUON isbad?: " << isbad );
  return isbad;
}

bool SUSYObjDef_xAOD::IsCosmicMuon(const xAOD::Muon& input, float z0cut, float d0cut) const
{
  dec_cosmic(input) = false;

  const xAOD::TrackParticle* track(nullptr);
  if (input.muonType() == xAOD::Muon::SiliconAssociatedForwardMuon) {
    track = input.trackParticle(xAOD::Muon::CombinedTrackParticle);
    if (!track){
      ATH_MSG_VERBOSE("WARNING: SAF muon without CB track found. Not possible to check cosmic muon criteria");
      return false; // don't treat SAF muons without CB track further  
    }
  }
  else {
    track = input.primaryTrackParticle();
    if (!track){
      ATH_MSG_WARNING("Non-SAF muon without primary track particle found. Not possible to check cosmic muon criteria");
      return false;
    }
  }

  double mu_d0 = track->d0();
  const xAOD::Vertex* pv = this->GetPrimVtx();
  double primvertex_z = pv ? pv->z() : 0;
  double mu_z0_exPV = track->z0() + track->vz() - primvertex_z;
  
  bool isCosmicMuon = (std::abs(mu_z0_exPV) >= z0cut || std::abs(mu_d0) >= d0cut);

  if (isCosmicMuon) {
    ATH_MSG_VERBOSE("COSMIC PV Z = " << primvertex_z << ", track z0 = " << mu_z0_exPV << ", track d0 = " << mu_d0);
  }

  dec_cosmic(input) = isCosmicMuon;
  return isCosmicMuon;
}


  float SUSYObjDef_xAOD::GetSignalMuonSF(const xAOD::Muon& mu, const bool recoSF, const bool isoSF, const bool doBadMuonHP, const bool warnOVR)
{
  float sf(1.);

  if (recoSF) {
    float sf_reco(1.);
    if (m_muonEfficiencySFTool->getEfficiencyScaleFactor( mu, sf_reco ) == CP::CorrectionCode::OutOfValidityRange) {
      if(warnOVR) ATH_MSG_WARNING(" GetSignalMuonSF: Reco getEfficiencyScaleFactor out of validity range");
    }
    ATH_MSG_VERBOSE( "MuonReco ScaleFactor " << sf_reco );
    sf *= sf_reco;

    float sf_ttva(1.);
    if(m_doTTVAsf){
      if (m_muonTTVAEfficiencySFTool->getEfficiencyScaleFactor( mu, sf_ttva ) == CP::CorrectionCode::OutOfValidityRange) {
	if(warnOVR) ATH_MSG_WARNING(" GetSignalMuonSF: TTVA getEfficiencyScaleFactor out of validity range");
      }
      ATH_MSG_VERBOSE( "MuonTTVA ScaleFactor " << sf_ttva );
      sf *= sf_ttva;
    }

    float sf_badHighPt(1.);
    if(m_muId == 4 && doBadMuonHP){
      if (m_muonEfficiencyBMHighPtSFTool->getEfficiencyScaleFactor( mu, sf_badHighPt ) == CP::CorrectionCode::OutOfValidityRange) {
	if(warnOVR) ATH_MSG_WARNING(" GetSignalMuonSF: BadMuonHighPt getEfficiencyScaleFactor out of validity range");
      }
      ATH_MSG_VERBOSE( "MuonBadMuonHighPt ScaleFactor " << sf_badHighPt );
      sf *= sf_badHighPt;
    }
  }


  if (isoSF) {
    float sf_iso(1.);
    if (acc_isolHighPt(mu) && mu.pt()>m_muIsoHighPtThresh) {
      if (m_muonHighPtIsolationSFTool->getEfficiencyScaleFactor( mu, sf_iso ) == CP::CorrectionCode::OutOfValidityRange) {
        if(warnOVR) ATH_MSG_WARNING(" GetSignalMuonSF: high-pt Iso getEfficiencyScaleFactor out of validity range");
      } 
    } else if (acc_isol(mu) && mu.pt()<m_muIsoHighPtThresh) {
      if (m_muonIsolationSFTool->getEfficiencyScaleFactor( mu, sf_iso ) == CP::CorrectionCode::OutOfValidityRange) {
        if(warnOVR) ATH_MSG_WARNING(" GetSignalMuonSF: Iso getEfficiencyScaleFactor out of validity range");
      } 
    }
    ATH_MSG_VERBOSE( "MuonIso ScaleFactor " << sf_iso );
    sf *= sf_iso;
  }


  dec_effscalefact(mu) = sf;
  return sf;
}


double SUSYObjDef_xAOD::GetMuonTriggerEfficiency(const xAOD::Muon& mu, const std::string& trigExpr, const bool isdata) {

  double eff(1.);

  if (m_muonTriggerSFTool->getTriggerEfficiency(mu, eff, trigExpr, isdata) != CP::CorrectionCode::Ok) {
    ATH_MSG_WARNING("Problem retrieving signal muon trigger efficiency for " << trigExpr );
  }
  else{
    ATH_MSG_DEBUG("Got efficiency " << eff << " for " << trigExpr );
  }
  return eff;
}


double SUSYObjDef_xAOD::GetTotalMuonTriggerSF(const xAOD::MuonContainer& sfmuons, const std::string& trigExpr) {
 
  if (trigExpr.empty() || sfmuons.empty()) return 1.;


  double trig_sf = 1.;

  int mulegs = 0;
  const char *tmp = trigExpr.c_str();
  while( (tmp = strstr(tmp, "mu")) ){
    mulegs++;
    tmp++;
  }

  bool isdimuon = (trigExpr.find("2mu") != std::string::npos);
  bool isOR = (trigExpr.find("OR") != std::string::npos);
  
  if((!isdimuon && mulegs<2) || (isdimuon && sfmuons.size()==2) || (mulegs>=2 && isOR)){   //Case 1: the tool takes easy care of the single, standard-dimuon and OR-of-single chains
    if (m_muonTriggerSFTool->getTriggerScaleFactor( sfmuons, trig_sf, trigExpr ) == CP::CorrectionCode::Ok) {
      ATH_MSG_DEBUG( "MuonTrig ScaleFactor " << trig_sf );
    }
    else{
      ATH_MSG_DEBUG( "MuonTrig FAILED SOMEHOW");
    }
  }
  else if(mulegs!=2 && isOR){ //Case 2: not supported. Not efficiency defined for (at least) one leg. Sorry...
    ATH_MSG_WARNING( "SF for " << trigExpr << " are only supported for two muon events!");
  }
  else{ //Case 3: let's go the hard way...
        //Following https://twiki.cern.ch/twiki/bin/view/Atlas/TrigMuonEfficiency
    std::string newtrigExpr = TString(trigExpr).Copy().ReplaceAll("HLT_2","").Data();

    //redefine dimuon triggers here (2mu14 --> mu14_mu14)
    if (isdimuon) { newtrigExpr += "_"+newtrigExpr;  }
    boost::replace_all(newtrigExpr, "HLT_", "");
    boost::char_separator<char> sep("_");

    for (const auto& mutrig : boost::tokenizer<boost::char_separator<char>>(newtrigExpr, sep)) {
      double dataFactor = 1.;
      double mcFactor   = 1.;
      
      for (const xAOD::Muon* mu : sfmuons) {
        // No need for additional trigger matching
        dataFactor *= (1 - GetMuonTriggerEfficiency(*mu, "HLT_"+mutrig, true));
        mcFactor   *= (1 - GetMuonTriggerEfficiency(*mu, "HLT_"+mutrig, false));
      }
      if( (1-mcFactor) > 0. )
        trig_sf *= (1-dataFactor)/(1-mcFactor);
    }
  }

  return trig_sf;
}


double SUSYObjDef_xAOD::GetTotalMuonSF(const xAOD::MuonContainer& muons, const bool recoSF, const bool isoSF, const std::string& trigExpr, const bool bmhptSF) {
  double sf(1.);

  ConstDataVector<xAOD::MuonContainer> sfmuons(SG::VIEW_ELEMENTS);
  for (const xAOD::Muon* muon : muons) {
    if( !acc_passOR(*muon) ) continue;
    if (acc_signal(*muon)) {
      sfmuons.push_back(muon);
      if (recoSF || isoSF) { sf *= this->GetSignalMuonSF(*muon, recoSF, isoSF, bmhptSF); }
    } else { // decorate baseline muons as well
      if (recoSF || isoSF) { this->GetSignalMuonSF(*muon, recoSF, isoSF, bmhptSF, false); } //avoid OVR warnings in this case
    }
  }

  sf *= GetTotalMuonTriggerSF(*sfmuons.asDataVector(), trigExpr);
  
  return sf;
}


  double SUSYObjDef_xAOD::GetTotalMuonSFsys(const xAOD::MuonContainer& muons, const CP::SystematicSet& systConfig, const bool recoSF, const bool isoSF, const std::string& trigExpr, const bool bmhptSF) {
  double sf(1.);
  //Set the new systematic variation
  StatusCode ret = m_muonEfficiencySFTool->applySystematicVariation(systConfig);
  if ( ret != StatusCode::SUCCESS) {
    ATH_MSG_ERROR("Cannot configure MuonEfficiencyScaleFactors for systematic var. " << systConfig.name() );
  }

  ret = m_muonEfficiencyBMHighPtSFTool->applySystematicVariation(systConfig);
  if ( ret != StatusCode::SUCCESS) {
    ATH_MSG_ERROR("Cannot configure MuonBadMuonHighPtScaleFactors for systematic var. " << systConfig.name() );
  }

  ret = m_muonTTVAEfficiencySFTool->applySystematicVariation(systConfig);
  if ( ret != StatusCode::SUCCESS) {
    ATH_MSG_ERROR("Cannot configure MuonTTVAEfficiencyScaleFactors for systematic var. " << systConfig.name() );
  }

  ret  = m_muonIsolationSFTool->applySystematicVariation(systConfig);
  if ( ret != StatusCode::SUCCESS) {
    ATH_MSG_ERROR("Cannot configure MuonIsolationScaleFactors for systematic var. " << systConfig.name() );
  }

  ret  = m_muonHighPtIsolationSFTool->applySystematicVariation(systConfig);
  if ( ret != StatusCode::SUCCESS) {
    ATH_MSG_ERROR("Cannot configure MuonHighPtIsolationScaleFactors for systematic var. " << systConfig.name() );
  }

  ret  = m_muonTriggerSFTool->applySystematicVariation(systConfig);
  if ( ret != StatusCode::SUCCESS) {
    ATH_MSG_ERROR("Cannot configure MuonTriggerScaleFactors for systematic var. " << systConfig.name() );
  }

  ret = m_trigGlobalEffCorrTool_diLep->applySystematicVariation(systConfig);
  if (ret != StatusCode::SUCCESS) {
    ATH_MSG_ERROR("Cannot configure TrigGlobalEfficiencyCorrectionTool (trigger) for systematic var. " << systConfig.name() );
  }

  ret = m_trigGlobalEffCorrTool_multiLep->applySystematicVariation(systConfig);
  if (ret != StatusCode::SUCCESS) {
    ATH_MSG_ERROR("Cannot configure TrigGlobalEfficiencyCorrectionTool (trigger) for systematic var. " << systConfig.name() );
  }

  sf = GetTotalMuonSF(muons, recoSF, isoSF, trigExpr, bmhptSF);

  //Roll back to default
  ret  = m_muonEfficiencySFTool->applySystematicVariation(m_currentSyst);
  if ( ret != StatusCode::SUCCESS) {
    ATH_MSG_ERROR("Cannot configure MuonEfficiencyScaleFactors back to default.");
  }

  ret = m_muonEfficiencyBMHighPtSFTool->applySystematicVariation(m_currentSyst);
  if ( ret != StatusCode::SUCCESS) {
    ATH_MSG_ERROR("Cannot configure MuonBadMuonHighPtScaleFactors back to default.");
  }

  ret  = m_muonTTVAEfficiencySFTool->applySystematicVariation(m_currentSyst);
  if ( ret != StatusCode::SUCCESS) {
    ATH_MSG_ERROR("Cannot configure MuonTTVAEfficiencyScaleFactors back to default.");
  }

  ret  = m_muonIsolationSFTool->applySystematicVariation(m_currentSyst);
  if ( ret != StatusCode::SUCCESS) {
    ATH_MSG_ERROR("Cannot configure MuonIsolationScaleFactors back to default.");
  }

  ret  = m_muonHighPtIsolationSFTool->applySystematicVariation(m_currentSyst);
  if ( ret != StatusCode::SUCCESS) {
    ATH_MSG_ERROR("Cannot configure MuonIsolationScaleFactors back to default.");
  }

  ret  = m_muonTriggerSFTool->applySystematicVariation(m_currentSyst);
  if ( ret != StatusCode::SUCCESS) {
    ATH_MSG_ERROR("Cannot configure MuonTriggerScaleFactors back to default.");
  }

  ret = m_trigGlobalEffCorrTool_diLep->applySystematicVariation(m_currentSyst);
  if (ret != StatusCode::SUCCESS) {
    ATH_MSG_ERROR("Cannot configure TrigGlobalEfficiencyCorrectionTool (trigger) back to default.");
  }

  ret = m_trigGlobalEffCorrTool_multiLep->applySystematicVariation(m_currentSyst);
  if (ret != StatusCode::SUCCESS) {
    ATH_MSG_ERROR("Cannot configure TrigGlobalEfficiencyCorrectionTool (trigger) back to default.");
  }

  return sf;
}

}
