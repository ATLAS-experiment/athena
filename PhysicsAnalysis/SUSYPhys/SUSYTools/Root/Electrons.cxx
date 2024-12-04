/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// This source file implements all of the functions related to Electrons
// in the SUSYObjDef_xAOD class

// Local include(s):
#include "SUSYTools/SUSYObjDef_xAOD.h"

#include "xAODBase/IParticleHelpers.h"
#include "xAODTracking/TrackParticlexAODHelpers.h"
#include "AthContainers/ConstDataVector.h"

#include "EgammaAnalysisInterfaces/IEgammaCalibrationAndSmearingTool.h"
#include "EgammaAnalysisInterfaces/IAsgElectronEfficiencyCorrectionTool.h"
#include "AsgAnalysisInterfaces/IEfficiencyScaleFactorTool.h"
#include "EgammaAnalysisInterfaces/IAsgElectronIsEMSelector.h"
#include "EgammaAnalysisInterfaces/IAsgPhotonIsEMSelector.h"
#include "EgammaAnalysisInterfaces/IAsgElectronLikelihoodTool.h"
#include "EgammaAnalysisInterfaces/IAsgDeadHVCellRemovalTool.h"
#include "EgammaAnalysisInterfaces/IEGammaAmbiguityTool.h"
#include "EgammaAnalysisInterfaces/IElectronLRTOverlapRemovalTool.h"
#include "xAODEgamma/ElectronAuxContainer.h"

#include "IsolationCorrections/IIsolationCorrectionTool.h"
#include "IsolationSelection/IIsolationSelectionTool.h"
//disable #include "IsolationSelection/IIsolationLowPtPLVTool.h"

#include "TriggerAnalysisInterfaces/ITrigGlobalEfficiencyCorrectionTool.h"

//disable #include "PATCore/TResult.h"

// For getting the beam spot information
#include "xAODEventInfo/EventInfo.h"

#ifndef XAOD_STANDALONE // For now metadata is Athena-only
#include "AthAnalysisBaseComps/AthAnalysisHelper.h"
#endif

namespace ST {

  const static SG::Decorator<char>      dec_passSignalID("passSignalID");
  const static SG::ConstAccessor<char>  acc_passSignalID("passSignalID");

  const static SG::Decorator<char>      dec_passChID("passChID");
  const static SG::ConstAccessor<char>  acc_passChID("passChID");
  const static SG::Decorator<double>    dec_ecisBDT("ecisBDT");

  const static SG::Decorator<float>     dec_sfChIDEff("chargeIDEffiSF"); //tools' default
  const static SG::ConstAccessor<float> acc_sfChIDEff("chargeIDEffiSF"); //tools' default

  const static SG::Decorator<float>     dec_z0sinTheta("z0sinTheta");
  const static SG::Decorator<float>     dec_d0sig("d0sig");
  const static SG::Decorator<char> dec_isLRT("isLRT");
  const static SG::ConstAccessor<char> acc_isLRT("isLRT");

  const static SG::ConstAccessor<char> acc_passECIDS("DFCommonElectronsECIDS"); // Loose 97% WP


StatusCode SUSYObjDef_xAOD::MergeElectrons(const xAOD::ElectronContainer & electrons, xAOD::ElectronContainer* outputCol, const std::set<const xAOD::Electron *> &ElectronsToRemove) const{

    if (electrons.empty()) return StatusCode::SUCCESS;
    for (const xAOD::Electron* electron: electrons) {
        if (ElectronsToRemove.find(electron) != ElectronsToRemove.end()){
            ATH_MSG_DEBUG( "Removing electron from output collection (isLRT?) : ("<< static_cast<int>(acc_isLRT(*electron)) << ")" );
            ATH_MSG_DEBUG( "ELECTRON cl eta: "                                    << electron->caloCluster()->eta());
            ATH_MSG_DEBUG( "ELECTRON cl phi: "                                    << electron->caloCluster()->phi());
            continue;
        // add electron into output 
        } else {
            ATH_MSG_DEBUG( "Adding electron to output collection (isLRT?) : ("    << static_cast<int>(acc_isLRT(*electron)) << ")" );
            ATH_MSG_DEBUG( "ELECTRON cl eta: "                                    << electron->caloCluster()->eta());
            ATH_MSG_DEBUG( "ELECTRON cl phi: "                                    << electron->caloCluster()->phi());
            auto newElectron = new xAOD::Electron(*electron);

            if ( getOriginalObject(*electron) != nullptr ) {
              setOriginalObjectLink(*getOriginalObject(*electron), *newElectron);
            } else {
              setOriginalObjectLink(*electron, *newElectron);
            }

            outputCol->push_back(newElectron); 
        }
    }
    return StatusCode::SUCCESS;
}


StatusCode SUSYObjDef_xAOD::prepareLRTElectrons(const xAOD::ElectronContainer* inElectrons, xAOD::ElectronContainer* copy) const{
  for (const xAOD::Electron *electron: *inElectrons){
    const xAOD::TrackParticle* idtrack = electron->trackParticle();
    
    // Save electron if the id track passes the LRT filter
    if ( acc_lrtFilter.isAvailable(*idtrack) )
    {
      if ( static_cast<int>(acc_lrtFilter(*idtrack) ) ){
        std::unique_ptr<xAOD::Electron> copyElectron = std::make_unique<xAOD::Electron>(*electron);

        // transfer original electron link

        setOriginalObjectLink(*electron, *copyElectron);
        copy->push_back( std::move(copyElectron) );        
      }  
    } 
    else // Keep electron if flag is not available
    {
      std::unique_ptr<xAOD::Electron> copyElectron = std::make_unique<xAOD::Electron>(*electron);
      
      setOriginalObjectLink(*electron, *copyElectron);
      copy->push_back( std::move(copyElectron) );
    }
  }
  return StatusCode::SUCCESS;
}


StatusCode SUSYObjDef_xAOD::GetElectrons(xAOD::ElectronContainer*& copy, xAOD::ShallowAuxContainer*& copyaux, bool recordSG, const std::string& elekey, const std::string& lrtelekey, const xAOD::ElectronContainer* containerToBeCopied)
{
  if (!m_tool_init) {
    ATH_MSG_ERROR("SUSYTools was not initialized!!");
    return StatusCode::FAILURE;
  }

   // Initializing prompt/LRT OR procedure
  auto outputCol = std::make_unique<xAOD::ElectronContainer>();
  std::unique_ptr<xAOD::ElectronAuxContainer> outputAuxCol;
  outputAuxCol = std::make_unique<xAOD::ElectronAuxContainer>();
  outputCol->setStore(outputAuxCol.get());
  ATH_CHECK( m_outElectronLocation.initialize() );

  if (bool(m_eleLRT) && !lrtelekey.empty() && evtStore()->contains<xAOD::ElectronContainer>(lrtelekey)){
    ATH_MSG_DEBUG("Applying prompt/LRT electron OR procedure"); 

    // First identify if merged container has already been made (for instances where GetElectrons() is called more than once)
    if (evtStore()->contains<xAOD::ElectronContainer>("StdWithLRTElectrons")) {
      ATH_MSG_DEBUG("Merged prompt/LRT container already created in TStore");  
    } else {
      ATH_MSG_DEBUG("Creating merged prompt/LRT container in TStore");

      // Retrieve prompt and LRT electrons from TStore
      ATH_CHECK( evtStore()->retrieve(prompt_electrons, elekey) );
      ATH_CHECK( evtStore()->retrieve(lrt_electrons, lrtelekey) );

      // Remove LRT electrons as flagged by filter for uncertainty
      auto filtered_electrons = std::make_unique<xAOD::ElectronContainer>();
      std::unique_ptr<xAOD::ElectronAuxContainer> filtered_electrons_aux = std::make_unique<xAOD::ElectronAuxContainer>();
      filtered_electrons->setStore(filtered_electrons_aux.get());
      ATH_CHECK(prepareLRTElectrons(lrt_electrons, filtered_electrons.get()));

      // Check overlap between prompt and LRT collections
      std::set<const xAOD::Electron *> ElectronsToRemove;
      m_elecLRTORTool->checkOverlap(*prompt_electrons, *filtered_electrons, ElectronsToRemove);
    
      // Decorate electrons with prompt/LRT
      for (const xAOD::Electron* el : *prompt_electrons)   dec_isLRT(*el) = 0;
      for (const xAOD::Electron* el : *filtered_electrons) dec_isLRT(*el) = 1;

      // Create merged StdWithLRTElectrons container
      outputCol->reserve(prompt_electrons->size() + filtered_electrons->size());
      ATH_CHECK(MergeElectrons(*prompt_electrons, outputCol.get(), ElectronsToRemove ));
      ATH_CHECK(MergeElectrons(*filtered_electrons, outputCol.get(), ElectronsToRemove ));

      // Save merged StdWithLRTElectrons container to TStore
      ATH_CHECK(evtStore()->record(std::move(outputCol), m_outElectronLocation.key())); 
      ATH_CHECK(evtStore()->record(std::move(outputAuxCol), m_outElectronLocation.key() + "Aux.") );
    }
  } else if (!lrtelekey.empty()) {
    if(evtStore()->contains<xAOD::ElectronContainer>(lrtelekey) == false && bool(m_eleLRT) == true) ATH_MSG_WARNING("prompt/LRT OR procedure attempted but " << lrtelekey << " not in ROOT file, check config!");
    ATH_MSG_DEBUG("Not applying prompt/LRT electron OR procedure"); 
  }

  if (m_isPHYSLITE && elekey.find("AnalysisElectrons")==std::string::npos){
    ATH_MSG_ERROR("You are running on PHYSLITE derivation. Please change the Electrons container to 'AnalysisElectrons'");
    return StatusCode::FAILURE;
  }

  const xAOD::ElectronContainer* electrons = nullptr;
  if (bool(m_eleLRT) && evtStore()->contains<xAOD::ElectronContainer>(lrtelekey)){
      ATH_MSG_DEBUG("Using container: " << m_outElectronLocation.key());
      ATH_CHECK( evtStore()->retrieve(electrons, m_outElectronLocation.key())); 
  }
  else { 
    if (copy==nullptr) { // empty container provided
        ATH_MSG_DEBUG("Empty container provided");
      if (containerToBeCopied != nullptr) {
        ATH_MSG_DEBUG("Containter to be copied not nullptr");
        electrons = containerToBeCopied;
      }
      else {
        ATH_MSG_DEBUG("Getting Electrons collection");
        ATH_CHECK( evtStore()->retrieve(electrons, elekey) );
      }
    }
  }

  std::pair<xAOD::ElectronContainer*, xAOD::ShallowAuxContainer*> shallowcopy = xAOD::shallowCopyContainer(*electrons);
  copy = shallowcopy.first;
  copyaux = shallowcopy.second;
  bool setLinks = xAOD::setOriginalObjectLink(*electrons, *copy);
  if (!setLinks) {
    ATH_MSG_WARNING("Failed to set original object links on " << elekey);
  } else { // use the user-supplied collection instead
    ATH_MSG_DEBUG("Not retrieving electron collection, using existing one provided by user");
    electrons=copy;
  }

  for (const auto& electron : *copy) {
    ATH_CHECK( this->FillElectron(*electron, m_eleBaselinePt, m_eleBaselineEta) );
    this->IsSignalElectron(*electron, m_elePt, m_eled0sig, m_elez0, m_eleEta);
  }

  if (recordSG) {
    ATH_CHECK( evtStore()->record(copy, "STCalib" + elekey + m_currentSyst.name()) );
    ATH_CHECK( evtStore()->record(copyaux, "STCalib" + elekey + m_currentSyst.name() + "Aux.") );
  }
  return StatusCode::SUCCESS;
}


StatusCode SUSYObjDef_xAOD::FillElectron(xAOD::Electron& input, float etcut, float etacut) {

  ATH_MSG_VERBOSE( "Starting FillElectron on el with pre-calibration pt=" << input.pt() );

  // According to https://twiki.cern.ch/twiki/bin/view/AtlasProtected/EGammaIdentificationRun2#Electron_identification:
  // "Please apply the identification to uncalibrated electron object. ID scale factors are to be applied to calibrated objects."
  dec_baseline(input) = false;
  dec_selected(input) = 0;
  dec_signal(input) = false;
  dec_isol(input) = false;
  dec_isolHighPt(input) = false;
  dec_passSignalID(input) = false;
  dec_passChID(input) = false;
  dec_ecisBDT(input) = -999.;

  const xAOD::EventInfo* evtInfo = nullptr;
  ATH_CHECK( evtStore()->retrieve( evtInfo, "EventInfo" ) );
  const xAOD::Vertex* pv = this->GetPrimVtx();
  const xAOD::TrackParticle* track =  input.trackParticle();
  if (!track) {
    ATH_MSG_DEBUG("No primary track particle for this electron. Skipping.");
    return StatusCode::SUCCESS;
  }
  double primvertex_z = pv ? pv->z() : 0;
  double el_z0 = track->z0() + track->vz() - primvertex_z;
  dec_z0sinTheta(input) = el_z0 * TMath::Sin(input.p4().Theta());
  //protect against exception thrown for null or negative d0sig
  try{
    dec_d0sig(input) = xAOD::TrackingHelpers::d0significance( track , evtInfo->beamPosSigmaX(), evtInfo->beamPosSigmaY(), evtInfo->beamPosSigmaXY() );
  }
  catch(...){
    float d0sigError = -99.;
    ATH_MSG_WARNING("FillElectron : Exception catched from d0significance() calculation. Setting dummy decoration d0sig=" << d0sigError );
    dec_d0sig(input) = d0sigError;
  }

  // don't bother calibrating or computing WP
  if ( input.pt() < 4e3 ) return StatusCode::SUCCESS;
  if ( !input.caloCluster() ) { ATH_MSG_WARNING( "FillElectron: no caloCluster found: " << input.caloCluster() ); return StatusCode::SUCCESS; }

  //Check DeadHVCellRemoval
  bool pass_deadHVTool = m_deadHVTool->accept(&input);

  if (m_debug) {
    unsigned char el_nPixHits(0), el_nSCTHits(0);
    input.trackParticle()->summaryValue(el_nPixHits, xAOD::numberOfPixelHits);
    input.trackParticle()->summaryValue(el_nSCTHits, xAOD::numberOfSCTHits);

    ATH_MSG_INFO( "ELECTRON eta: "         << input.eta());
    ATH_MSG_INFO( "ELECTRON phi: "         << input.phi());
    ATH_MSG_INFO( "ELECTRON cl eta: "      << input.caloCluster()->eta());
    ATH_MSG_INFO( "ELECTRON cl phi: "      << input.caloCluster()->phi());
    ATH_MSG_INFO( "ELECTRON cl e: "        << input.caloCluster()->e());
    ATH_MSG_INFO( "ELECTRON trk eta: "     << input.trackParticle()->eta());
    ATH_MSG_INFO( "ELECTRON trk phi: "     << input.trackParticle()->phi());
    ATH_MSG_INFO( "ELECTRON author: "      << input.author());
    ATH_MSG_INFO( "ELECTRON OQ: "          << acc_OQ(input));
    ATH_MSG_INFO( "ELECTRON nPixHits: "    << static_cast<int>(el_nPixHits));
    ATH_MSG_INFO( "ELECTRON nSCTHits: "    << static_cast<int>(el_nSCTHits));
    ATH_MSG_INFO( "ELECTRON deadHVTools: " << static_cast<bool>(pass_deadHVTool));
  }

  if (!pass_deadHVTool) return StatusCode::SUCCESS;
  if (!input.isGoodOQ(xAOD::EgammaParameters::BADCLUSELECTRON)) return StatusCode::SUCCESS;

  if ( m_elecSelLikelihoodBaseline.empty()) {
    ATH_MSG_ERROR("No baseline electron selector defined!");
    return StatusCode::FAILURE;
  }

  bool passBaseID = false;
  if (m_eleIdExpert) {
    passBaseID = bool(m_elecSelLikelihoodBaseline->accept(&input));
  } else {
    if (m_acc_eleIdBaseline.isAvailable(input)) {
      passBaseID = m_acc_eleIdBaseline(input); 
    } else {
      ATH_MSG_VERBOSE ("DFCommonElectronsLHxxx variables are not found. Calculating the ID from LH tool..");
      passBaseID = bool(m_elecSelLikelihoodBaseline->accept(&input));
    }
  }

  if ( !passBaseID && !m_force_noElId ) return StatusCode::SUCCESS;

  //baseline ID decoration for TauEl OR
  //dec_passBaseID(input) = true;

  // calibrate the electron 4-vector here only if within eta window
  if (std::abs(input.caloCluster()->etaBE(2)) >= etacut) return StatusCode::SUCCESS;

  if (m_eleBaselineCrackVeto){
    if  ( std::abs( input.caloCluster()->etaBE(2) ) >1.37 &&  std::abs( input.caloCluster()->etaBE(2) ) <1.52) {
      return StatusCode::SUCCESS;
    }
  }

  // corrections described in https://twiki.cern.ch/twiki/bin/view/AtlasProtected/EGammaCalibrationRun2
  if ( m_egammaCalibTool->applyCorrection(input) != CP::CorrectionCode::Ok)
    ATH_MSG_ERROR( "FillElectron: EgammaCalibTool applyCorrection failed ");

  if (m_isoCorrTool->applyCorrection(input)  != CP::CorrectionCode::Ok)
    ATH_MSG_ERROR("FillElectron: IsolationCorrectionTool applyCorrection failed");

  ATH_MSG_VERBOSE( "FillElectron: post-calibration pt=" << input.pt() );

  if (input.pt() < etcut) return StatusCode::SUCCESS;

  if (m_elebaselinez0>0. && std::abs(acc_z0sinTheta(input))>m_elebaselinez0) return StatusCode::SUCCESS;
  if (m_elebaselined0sig>0. && std::abs(acc_d0sig(input))>m_elebaselined0sig) return StatusCode::SUCCESS;

  //--- Do baseline isolation check
  if ( !( m_eleBaselineIso_WP.empty() ) &&  !( m_isoBaselineTool->accept(input) ) ) return StatusCode::SUCCESS;

  dec_baseline(input) = true;
  dec_selected(input) = 2;
  //disable if (!m_eleIso_WP.empty() && m_eleIso_WP.find("PLV")!=std::string::npos) ATH_CHECK(  ->augmentPLV(input) );
  if (!m_eleIso_WP.empty()) dec_isol(input) = bool(m_isoTool->accept(input));
  if (!m_eleIsoHighPt_WP.empty()) dec_isolHighPt(input) = bool(m_isoHighPtTool->accept(input));

  //ChargeIDSelector
  if( m_runECIS ){
    if (acc_passECIDS.isAvailable(input)) {
      dec_passChID(input) = acc_passECIDS(input); // Loose 97% WP!
    } else {
      dec_passChID(input) = bool(m_elecChargeIDSelectorTool->accept(&input));
      //disable double bdt = m_elecChargeIDSelectorTool->calculate(&input).getResult("bdt");
      //disable dec_ecisBDT(input) = bdt;
    }
  }
  else{
    dec_passChID(input) = true;
  }

  ATH_MSG_VERBOSE( "FillElectron: passed baseline selection" );

  return StatusCode::SUCCESS;
}


bool SUSYObjDef_xAOD::IsSignalElectron(const xAOD::Electron & input, float etcut, float d0sigcut, float z0cut, float etacut) const
{
  if (!acc_baseline(input)) return false;

  dec_passSignalID(input) = false;

  if (m_eleIdExpert) {
    if ( !m_elecSelLikelihood.empty() && m_elecSelLikelihood->accept(&input) ) dec_passSignalID(input) = true;
    } 
  else {
    if (m_acc_eleId.isAvailable(input)) {
      dec_passSignalID(input) = m_acc_eleId(input); 
    } else {
      ATH_MSG_VERBOSE ("DFCommonElectronsLHxxx variables are not found. Calculating the ID from LH tool..");
      if ( !m_elecSelLikelihood.empty() && m_elecSelLikelihood->accept(&input) ) dec_passSignalID(input) = true;
    }
  }

  //overwrite ID selection if forced by user
  if(m_force_noElId) dec_passSignalID(input) = true;

  if (!acc_passSignalID(input)) return false;

  if (input.p4().Perp2() <= etcut * etcut || input.p4().Perp2() == 0) return false; // eT cut (might be necessary for leading electron to pass trigger)
  if ( etacut==DUMMYDEF ){
    if(std::abs(input.caloCluster()->etaBE(2)) > m_eleEta ) return false;
  }
  else if ( std::abs(input.caloCluster()->etaBE(2)) > etacut ) return false;

  if (m_eleCrackVeto){
    if  ( std::abs( input.caloCluster()->etaBE(2) ) >1.37 &&  std::abs( input.caloCluster()->etaBE(2) ) <1.52) {
      return false;
    }
  }

  if (acc_d0sig(input) != 0) {
    if (d0sigcut > 0.0 && std::abs(acc_d0sig(input)) > d0sigcut) return false; // transverse IP cut
  }

  if (z0cut > 0.0 && std::abs(acc_z0sinTheta(input)) > z0cut) return false; // longitudinal IP cut


  ATH_MSG_VERBOSE( "IsSignalElectron: " << m_eleId << " " << acc_passSignalID(input) << " d0sig " << acc_d0sig(input) << " z0 sin(theta) " << acc_z0sinTheta(input) );

  if (m_doElIsoSignal) {
    if ( !( (acc_isol(input) && input.pt()<m_eleIsoHighPtThresh) || (acc_isolHighPt(input) && input.pt()>m_eleIsoHighPtThresh)) ) return false;
    ATH_MSG_VERBOSE( "IsSignalElectron: passed isolation" );
  }

  if(m_eleChID_signal && !acc_passChID(input)) return false; //add charge flip check to signal definition

  dec_signal(input) = true;

  ATH_MSG_VERBOSE( "IsSignalElectron: passed selection" );
  return true;
}


float SUSYObjDef_xAOD::GetSignalElecSF(const xAOD::Electron& el,
                                       const bool recoSF,
                                       const bool idSF,
                                       const bool triggerSF,
                                       const bool isoSF,
                                       const std::string& trigExpr,
				                               const bool ecidsSF,
                                       const bool cidSF ) {

  if ((m_eleId == "VeryLooseLLH" || m_eleId == "LooseLLH" || m_eleId == "Loose" || m_eleId == "Medium" || m_eleId == "Tight") && (idSF || triggerSF || isoSF)) {
    ATH_MSG_ERROR("No signal electron ID or trigger scale factors provided for the selected working point!");
    ATH_MSG_ERROR("I will now die messily.");
  }

  //shortcut keys for trigger SF config
  std::string singleLepStr = "singleLepton";
  std::string diLepStr     = "diLepton";
  std::string multiLepStr  = "multiLepton";

  float sf(1.);

  if (recoSF) {
    double reco_sf(1.);

    CP::CorrectionCode result = m_elecEfficiencySFTool_reco->getEfficiencyScaleFactor(el, reco_sf);
    switch (result) {
    case CP::CorrectionCode::Ok:
      sf *= reco_sf;
      break;
    case CP::CorrectionCode::Error:
      ATH_MSG_ERROR( "Failed to retrieve signal electron reco SF");
      break;
    case CP::CorrectionCode::OutOfValidityRange:
      ATH_MSG_VERBOSE( "OutOfValidityRange found for signal electron reco SF");
      break;
    default:
      ATH_MSG_WARNING( "Don't know what to do for signal electron reco SF");
    }
  }

  if (idSF) {
    double id_sf(1.);

    CP::CorrectionCode result = m_elecEfficiencySFTool_id->getEfficiencyScaleFactor(el, id_sf);
    switch (result) {
    case CP::CorrectionCode::Ok:
      sf *= id_sf;
      break;
    case CP::CorrectionCode::Error:
      ATH_MSG_ERROR( "Failed to retrieve signal electron id SF");
      break;
    case CP::CorrectionCode::OutOfValidityRange:
      ATH_MSG_VERBOSE( "OutOfValidityRange found for signal electron id SF");
      break;
    default:
      ATH_MSG_WARNING( "Don't know what to do for signal electron id SF");
    }
  }

  if (triggerSF) {

    std::vector<std::string> trigMChains={};
    std::string theExpr ("");
    if(trigExpr==singleLepStr) {
      if (this->treatAsYear()==2015) trigMChains = m_v_trigs15_cache_singleEle;
      else if (this->treatAsYear()==2016) trigMChains = m_v_trigs16_cache_singleEle;
      else if (this->treatAsYear()==2017) trigMChains = m_v_trigs17_cache_singleEle;
      else if (this->treatAsYear()==2018) trigMChains = m_v_trigs18_cache_singleEle;
      else trigMChains = m_v_trigs22_cache_singleEle;
      theExpr=m_electronTriggerSFStringSingle;
    }
    else{
      ATH_MSG_WARNING( "Only single lepton trigger SFs are supported in GetSignalElecSF(). Use GetTriggerGlobalEfficiencySF() for dilepton or multilepton triggers!");
    }

    //check matching
    this->TrigMatch({&el}, trigMChains);

    if(!acc_trigmatched(el)){
      ATH_MSG_DEBUG( "Electron was not matched to trigger " << theExpr << " - scale factor does not apply (year " << this->treatAsYear() << ")  Returning 1." );
    }
    else{ //is trig-matched electron, go for it!
      if (trigExpr==multiLepStr || trigExpr==diLepStr) {
        ATH_MSG_WARNING( "The dilepton or multilepton trigger SFs are not supported in GetSignalElecSF(). Use GetTriggerGlobalEfficiencySF()!");
      }
      else {
        double trig_sf = GetEleTriggerEfficiencySF( el , theExpr );
        sf *= trig_sf;
      }
    }
  }

  if (isoSF) {
    double iso_sf(1.);
    CP::CorrectionCode result;
    if (acc_isolHighPt(el) && el.pt()>m_eleIsoHighPtThresh)
      result = m_elecEfficiencySFTool_isoHighPt->getEfficiencyScaleFactor(el, iso_sf);
    else
      result = m_elecEfficiencySFTool_iso->getEfficiencyScaleFactor(el, iso_sf);

    switch (result) {
    case CP::CorrectionCode::Ok:
      sf *= iso_sf;
      break;
    case CP::CorrectionCode::Error:
      ATH_MSG_ERROR( "Failed to retrieve signal electron iso SF");
      break;
    case CP::CorrectionCode::OutOfValidityRange:
      ATH_MSG_VERBOSE( "OutOfValidityRange found for signal electron iso SF");
      break;
    default:
      ATH_MSG_WARNING( "Don't know what to do for signal electron iso SF");
    }
  }

  // Charge flip SF: combined ECIDs & charge ID
  if ( ecidsSF || cidSF ) {
    double chf_sf(1.);
    // 1. ECIDs SF
    if ( ecidsSF ) {
      sf *= chf_sf;
      ATH_MSG_WARNING( "ECID SF ARE NOT YET SUPPORTED IN R22" );
      }
    // 2. CID SF
    if ( cidSF ) {
        sf *= chf_sf;
        dec_sfChIDEff(el) = chf_sf;
        ATH_MSG_WARNING( "CID SF ARE NOT YET SUPPORTED IN R22" );
    }
  }

  dec_effscalefact(el) = sf;
  return sf;
}


double SUSYObjDef_xAOD::GetEleTriggerEfficiencySF(const xAOD::Electron& el, const std::string& trigExpr) const {

  double trig_sf(1.);

  std::string single_str = "SINGLE_E";
  std::string single_str_2022 = "2022_";
  std::string dilep_str  = "DI_E";
  std::string multi_str  = "MULTI_L";

  CP::CorrectionCode result;
  if ( trigExpr.find(single_str) != std::string::npos || trigExpr.find(single_str_2022) != std::string::npos)
    result = m_elecEfficiencySFTool_trig_singleLep->getEfficiencyScaleFactor(el, trig_sf);
  else if ( trigExpr.find(dilep_str) != std::string::npos )
    ATH_MSG_ERROR( "Use GetTriggerGlobalEfficiency for logical OR of lepton triggers");
  else if ( trigExpr.find(multi_str) != std::string::npos )
    ATH_MSG_ERROR( "Use GetTriggerGlobalEfficiency for logical OR of lepton triggers");
  else
    ATH_MSG_ERROR( "The trigger expression (" << trigExpr << ") is not supported by the electron trigger SF!");

  switch (result) {
    case CP::CorrectionCode::Error:
      ATH_MSG_ERROR( "Failed to retrieve signal electron trigger SF");
      return 1.;
    case CP::CorrectionCode::OutOfValidityRange:
      ATH_MSG_VERBOSE( "OutOfValidityRange found for signal electron trigger SF");
      return 1.;
    default:
      break;
  }

  return trig_sf;
}


double SUSYObjDef_xAOD::GetEleTriggerEfficiency(const xAOD::Electron& el, const std::string& trigExpr) const {

  std::string single_str = "SINGLE_E";
  std::string single_str_2022 = "2022_";
  std::string dilep_str  = "DI_E";
  std::string multi_str  = "MULTI_L";

  double trig_eff(1.);

  CP::CorrectionCode result;
  if ( trigExpr.find(single_str) != std::string::npos || trigExpr.find(single_str_2022) != std::string::npos)
    result = m_elecEfficiencySFTool_trigEff_singleLep->getEfficiencyScaleFactor(el, trig_eff);
  else if ( trigExpr.find(dilep_str) != std::string::npos )
    ATH_MSG_ERROR( "Use GetTriggerGlobalEfficiency for logical OR of lepton triggers");
  else if ( trigExpr.find(multi_str) != std::string::npos )
    ATH_MSG_ERROR( "Use GetTriggerGlobalEfficiency for logical OR of lepton triggers");
  else
    ATH_MSG_ERROR( "The trigger expression (" << trigExpr << ") is not supported by the electron trigger efficiency!");

  switch (result) {
  case CP::CorrectionCode::Error:
    ATH_MSG_ERROR( "Failed to retrieve signal electron trigger efficiency");
    return 1.;
  case CP::CorrectionCode::OutOfValidityRange:
    ATH_MSG_VERBOSE( "OutOfValidityRange found for signal electron trigger efficiency");
    return 1.;
  default:
    break;
  }

  return trig_eff;
}



  float SUSYObjDef_xAOD::GetTotalElectronSF(const xAOD::ElectronContainer& electrons, const bool recoSF, const bool idSF, const bool triggerSF, const bool isoSF, const std::string& trigExpr, const bool ecidsSF, const bool cidSF) {
  float sf(1.);

  for (const xAOD::Electron* electron : electrons) {
    if (!acc_passOR(*electron)) continue;
    if (acc_signal(*electron)) { sf *= this->GetSignalElecSF(*electron, recoSF, idSF, triggerSF, isoSF, trigExpr, ecidsSF, cidSF); }
    else { this->GetSignalElecSF(*electron, recoSF, idSF, triggerSF, isoSF, trigExpr, ecidsSF, cidSF); }
  }

  return sf;
}


  float SUSYObjDef_xAOD::GetTotalElectronSFsys(const xAOD::ElectronContainer& electrons, const CP::SystematicSet& systConfig, const bool recoSF, const bool idSF, const bool triggerSF, const bool isoSF, const std::string& trigExpr, const bool ecidsSF, const bool cidSF) {
  float sf(1.);

  //Set the new systematic variation
  StatusCode ret = m_elecEfficiencySFTool_reco->applySystematicVariation(systConfig);
  if (ret != StatusCode::SUCCESS) {
    ATH_MSG_ERROR("Cannot configure AsgElectronEfficiencyCorrectionTool (reco) for systematic var. " << systConfig.name() );
  }

  ret = m_elecEfficiencySFTool_id->applySystematicVariation(systConfig);
  if (ret != StatusCode::SUCCESS) {
    ATH_MSG_ERROR("Cannot configure AsgElectronEfficiencyCorrectionTool (id) for systematic var. " << systConfig.name() );
  }

  ret = m_elecEfficiencySFTool_trig_singleLep->applySystematicVariation(systConfig);
  if (ret != StatusCode::SUCCESS) {
    ATH_MSG_ERROR("Cannot configure AsgElectronEfficiencyCorrectionTool (trigger) for systematic var. " << systConfig.name() );
  }

  ret = m_elecEfficiencySFTool_iso->applySystematicVariation(systConfig);
  if (ret != StatusCode::SUCCESS) {
    ATH_MSG_ERROR("Cannot configure AsgElectronEfficiencyCorrectionTool (iso) for systematic var. " << systConfig.name() );
  }

  ret = m_elecEfficiencySFTool_isoHighPt->applySystematicVariation(systConfig);
  if (ret != StatusCode::SUCCESS) {
    ATH_MSG_ERROR("Cannot configure AsgElectronEfficiencyCorrectionTool (iso high-pt) for systematic var. " << systConfig.name() );
  }

  ret = m_elecChargeEffCorrTool->applySystematicVariation(systConfig);
  if (ret != StatusCode::SUCCESS) {
    ATH_MSG_ERROR("Cannot configure ElectronChargeEfficiencyCorrectionTool for systematic var. " << systConfig.name() );
  }


  //Get the total SF for new config
  sf = GetTotalElectronSF(electrons, recoSF, idSF, triggerSF, isoSF, trigExpr, ecidsSF, cidSF);

  //Roll back to default
  ret = m_elecEfficiencySFTool_reco->applySystematicVariation(m_currentSyst);
  if (ret != StatusCode::SUCCESS) {
    ATH_MSG_ERROR("Cannot configure AsgElectronEfficiencyCorrectionTool (reco) back to default.");
  }

  ret = m_elecEfficiencySFTool_id->applySystematicVariation(m_currentSyst);
  if (ret != StatusCode::SUCCESS) {
    ATH_MSG_ERROR("Cannot configure AsgElectronEfficiencyCorrectionTool (id) back to default.");
  }

  ret = m_elecEfficiencySFTool_trig_singleLep->applySystematicVariation(m_currentSyst);
  if (ret != StatusCode::SUCCESS) {
    ATH_MSG_ERROR("Cannot configure AsgElectronEfficiencyCorrectionTool (trigger) back to default.");
  }

  ret = m_elecEfficiencySFTool_iso->applySystematicVariation(m_currentSyst);
  if (ret != StatusCode::SUCCESS) {
    ATH_MSG_ERROR("Cannot configure AsgElectronEfficiencyCorrectionTool (iso) back to default.");
  }

  ret = m_elecEfficiencySFTool_isoHighPt->applySystematicVariation(m_currentSyst);
  if (ret != StatusCode::SUCCESS) {
    ATH_MSG_ERROR("Cannot configure AsgElectronEfficiencyCorrectionTool (iso high-pt) back to default.");
  }

  ret = m_elecChargeEffCorrTool->applySystematicVariation(m_currentSyst);
  if (ret != StatusCode::SUCCESS) {
    ATH_MSG_ERROR("Cannot configure ElectronChargeEfficiencyCorrectionTool back to default.");
  }

  return sf;
}

}
