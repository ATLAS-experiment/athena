/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

//==================================================================================
// Include files...
//==================================================================================
// This files header
#include "InDetPerformanceMonitoring/IDPerfMonZmumu.h"
// Standard headers
#include "TTree.h"
#include "TLorentzVector.h"

#include "InDetPerformanceMonitoring/PerfMonServices.h"
#include "InDetPerformanceMonitoring/ZmumuEvent.h"
#include "InDetPerformanceMonitoring/FourMuonEvent.h"

#include "egammaInterfaces/IegammaTrkRefitterTool.h"
#include "EventPrimitives/EventPrimitivesHelpers.h"

// ATLAS headers
#include "GaudiKernel/IInterface.h"
#include "StoreGate/StoreGateSvc.h"

#include "TrkTruthData/TrackTruthCollection.h"
#include "TrkTruthData/TrackTruthKey.h"

/** needed for IP resolution studies **/
#include "TrkVertexFitterInterfaces/ITrackToVertexIPEstimator.h"

#include "TrigConfxAOD/xAODConfigTool.h"
#include "TrigDecisionTool/TrigDecisionTool.h"
#include "TriggerMatchingTool/MatchingTool.h"
#include "TrigDecisionTool/ChainGroup.h"

#include "InDetTrackSelectionTool/IInDetTrackSelectionTool.h"
#include "MuonAnalysisInterfaces/IMuonSelectionTool.h"

/** missing ET */
#include "xAODMissingET/MissingET.h"
#include "xAODMissingET/MissingETContainer.h"

//==================================================================================
// Public Methods
//==================================================================================
IDPerfMonZmumu::IDPerfMonZmumu(const std::string& name,
  ISvcLocator* pSvcLocator):
  AthAlgorithm(name, pSvcLocator),
  m_isMC(false),
  m_doRefit(false),
  m_TrackRefitter1(""),
  m_TrackRefitter2(""),
  m_trackToVertexTool("Reco::TrackToVertex"),
  m_triggerDecision("Trig::TrigDecisionTool/TrigDecisionTool"),
  m_triggerMatching("Trig::MatchingTool/MatchingTool"),
  m_selTool( "InDet::InDetTrackSelectionTool/TrackSelectionTool"),
  m_trackToVertexIPEstimator("Trk::TrackToVertexIPEstimator"),
  m_extrapolator("Trk::Extrapolator/AtlasExtrapolator"),
  m_muonSelector("CP::MuonSelectionTool/MuonSelectionTool"),
  m_validationMode(true),

  m_commonTreeName ("commonTree"),
  m_defaultTreeName("Default_Particle"),
  m_IDTreeName     ("ID_InDetTrackParticle"),
  m_refit1TreeName ("Refit1_SiAndTRT"),
  m_refit2TreeName ("Refit2_SiOnly"),
  m_truthTreeName  ("TruthParams"),
  m_combTreeName   ("CombinedTrackParticle"),
  m_MSTreeName     ("MS_TrackParticle"),
  m_FourMuTreeName ("FourMu"),

  m_ValidationTreeDescription("Small Tree for Zmumu fits"),

  m_commonTreeFolder ("/ZmumuValidation/common"),
  m_defaultTreeFolder("/ZmumuValidation/default"),
  m_IDTreeFolder     ("/ZmumuValidation/ID"),
  m_refit1TreeFolder ("/ZmumuValidation/refit1"),
  m_refit2TreeFolder ("/ZmumuValidation/refit2"),
  m_truthTreeFolder  ("/ZmumuValidation/truth"),
  m_combTreeFolder   ("/ZmumuValidation/comb"),
  m_MSTreeFolder     ("/ZmumuValidation/ms"),
  m_FourMuTreeFolder ("/ZmumuValidation/fourmu"),

  m_commonTree (nullptr),
  m_defaultTree(nullptr),
  m_IDTree(nullptr),
  m_refit1Tree(nullptr),
  m_refit2Tree(nullptr),
  m_truthTree(nullptr),
  m_combTree(nullptr),
  m_MSTree(nullptr),
  m_FourMuTree(nullptr),
  m_doRemoval(true),
  m_doDebug(false),
  m_Trk2VtxAssociationTool("CP::TrackVertexAssociationTool", this)
{
  // Properties that are set from the python scripts.

  declareProperty("UseTrigger"    ,    m_UseTrigger     = true);
  declareProperty("doIsoSelection",    m_doIsoSelection = true );
  declareProperty("doIPSelection",     m_doIPSelection = true );
  declareProperty("doMCPSelection",    m_doMCPSelection = true );
  declareProperty("isMC",              m_isMC = false);
  declareProperty("doRefit",           m_doRefit = false);
  declareProperty("doIPextrToPV",      m_doIP = false);
  declareProperty("Extrapolator",      m_extrapolator );
  declareProperty("MassWindowLow",     m_MassWindowLow = 60.0, "Lower cut in mu+mu- invariant mass" );
  declareProperty("MassWindowHigh",    m_MassWindowHigh = 120.0, "Upper cut in mu+mu- invariant mass" );
  declareProperty("OpeningAngle",      m_OpeningAngleCut = 0.2, "Opening angle between the two muons (in radians)");
  declareProperty("OutputTracksName",  m_outputTracksName = "ZmumuTracks");
  declareProperty("PtLeadingMuon",     m_LeadingMuonPtCut = 20., "Pt cut on the leading muon");
  declareProperty("PtSecondMuon",      m_SecondMuonPtCut = 15., "Pt cut on the second muon");
  declareProperty("ReFitterTool1",     m_TrackRefitter1, "ToolHandle for track fitter implementation");
  declareProperty("ReFitterTool2",     m_TrackRefitter2, "ToolHandle for track fitter implementation");
  declareProperty("TrackToVertexTool", m_trackToVertexTool);
  declareProperty("TrackTruthName",    m_truthName="TrackTruthCollection");
  declareProperty("TrackParticleName", m_trackParticleName="CombinedTrackParticle");
  declareProperty("triggerChainName",  m_sTriggerChainName);
  declareProperty("ValidationMode",    m_validationMode);
  declareProperty("xAODTruthLinkVector",      m_truthLinkVecName="xAODTruthLinks");
  declareProperty("Z0Gap",                    m_Z0GapCut = 5.0, "maximum gap between the z0 of both muons (in mm)");
  declareProperty("TrackSelectionTool",       m_selTool );
  declareProperty("UseTrackSelectionTool",    m_useTrackSelectionTool = false);
  declareProperty("TrackToVertexIPEstimator", m_trackToVertexIPEstimator);


  declareProperty("commonTreeFolder",  m_commonTreeFolder, "/ZmumuValidationUserSel/common" );
  declareProperty("defaultTreeFolder", m_defaultTreeFolder );
  declareProperty("IDTreeFolder",      m_IDTreeFolder );
  declareProperty("refit1TreeFolder",  m_refit1TreeFolder );
  declareProperty("refit2TreeFolder",  m_refit2TreeFolder );
  declareProperty("truthTreeFolder",   m_truthTreeFolder );
  declareProperty("combTreeFolder",    m_combTreeFolder );
  declareProperty("MSTreeFolder",      m_MSTreeFolder, "/ZmumuValidationUserSel/ms" );

  declareProperty("UnbiasVertex",      m_doRemoval);
  
  declareProperty("commonTree",  m_commonTreeName, "CommonTree" );
  declareProperty("DefaultTree", m_defaultTreeName );
  declareProperty("IDTree",      m_IDTreeName );
  declareProperty("CBTree",      m_combTreeName );
  declareProperty("Refit1Tree",  m_refit1TreeName );
  declareProperty("Refit2Tree",  m_refit2TreeName );
  declareProperty("MSTree",      m_MSTreeName );
  
  declareProperty("doFourMuAnalysis", m_doFourMuAnalysis = false);
  declareProperty("FourMuTreeFolder", m_FourMuTreeFolder);

  declareProperty("StoreZmumuNtuple", m_storeZmumuNtuple = true);
  declareProperty("doZmumuEventDebug", m_doDebug);
  declareProperty("MuonQuality", m_MuonQualityName = "Medium");
  declareProperty("skipMS", m_skipMS = false);
  declareProperty("useCustomMuonSelector", m_useCustomMuonSelector = false );
  declareProperty("MuonSelector", m_muonSelector );

  declareProperty( "MinLumiBlock", m_minGoodLumiBlock = 0, "minimum lumiblock number to be accepted");
  declareProperty( "MaxLumiBlock", m_maxGoodLumiBlock = 0, "maximum lumiblock number to be accepted");

  return;
}


//==================================================================================
IDPerfMonZmumu::~IDPerfMonZmumu()
{}


//==================================================================================
StatusCode IDPerfMonZmumu::initialize()
{
  ATH_MSG_DEBUG ("** IDPerfMonZmumu::Initialize ** START **");
  // Setup the services
  ISvcLocator* pxServiceLocator = serviceLocator();
  if ( pxServiceLocator != nullptr ) {
      StatusCode xSC = PerfMonServices::InitialiseServices( pxServiceLocator );
      if ( xSC == StatusCode::FAILURE ) {
	ATH_MSG_FATAL("Problem Initializing PerfMonServices");
	return StatusCode::FAILURE;
      }
      else {
	ATH_MSG_DEBUG("PerfMonServices::InitialiseServices( pxServiceLocator );  SUCCESS ");
      }
  }

  // Retrieve Track fitter and track to vertex
  if (m_doRefit) { // only if track refit is requested
    if (m_TrackRefitter1.retrieve().isSuccess()) {
      ATH_MSG_INFO("Retrieved tool m_TrackRefitter1: " << m_TrackRefitter1 << " SUCCESS ");
    } 
    else {
      ATH_MSG_FATAL("Unable to retrieve m_TrackRefitter1 " << m_TrackRefitter1 << " FAILURE ");
      return StatusCode::FAILURE;
    }

    // Retrieve the second fitter
    if (m_TrackRefitter2.retrieve().isSuccess()) {
      ATH_MSG_INFO("Retrieved tool m_TrackRefitter2: " << m_TrackRefitter2 << " SUCCESS ");
    } 
    else {
      ATH_MSG_FATAL("Unable to retrieve m_TrackRefitter2 " << m_TrackRefitter2 << " FAILURE ");
      return StatusCode::FAILURE;
    }

    if (m_trackToVertexTool.retrieve().isSuccess()) {
      ATH_MSG_INFO("Retrieved tool m_trackToVertexTool " << m_trackToVertexTool << " SUCCESS ");
    } 
    else {
      ATH_MSG_FATAL("Unable to retrieve m_trackToVertexTool " << m_trackToVertexTool << " FAILURE ");
      return StatusCode::FAILURE;
    }
  }

  if(m_useTrackSelectionTool){
    //    ATH_CHECK( m_selTool.retrieve() );
    if (m_selTool.retrieve().isSuccess()){
      ATH_MSG_INFO("Retrieved tool (track selection tool) m_selTool " << m_selTool << " SUCCESS ");
    }
    else {
      ATH_MSG_FATAL("Unable to retrieve (track selection tool) m_selTool " << m_selTool << " FAILURE ");
      return StatusCode::FAILURE;
    }
  }

  // initializing the eventInfo "accessor"
  ATH_CHECK(  m_EventInfoKey.initialize() );

  if (m_doIP) {
    ATH_MSG_DEBUG("Retrieving tool (trackToVertexIPEstimator)");
    ATH_CHECK (m_trackToVertexIPEstimator.retrieve());
  }


  ATH_CHECK (m_EventInfoKey.initialize());   // initializing the eventInfo "accessor"

  if (m_isMC) ATH_CHECK (m_extrapolator.retrieve()); // this is only used for the truth particles
  
  ATH_CHECK (m_vertexKey.initialize());
  
  ATH_CHECK (m_trackContainerName.initialize());

  ATH_MSG_INFO(" -- IDPerfMonZmumu::initialize() -- m_vertexKey: " << m_vertexKey);

  ATH_MSG_INFO(" -- IDPerfMonZmumu::initialize() -- init m_beamSpotKey ");
  ATH_CHECK(m_beamSpotKey.initialize());

  // START new place for initilization of params
  m_xZmm.setContainer       (PerfMonServices::MUON_COLLECTION);
  m_xZmm.doIsoSelection     (m_doIsoSelection);
  m_xZmm.doIPSelection      (m_doIPSelection);
  m_xZmm.doMCPSelection     (m_doMCPSelection);
  m_xZmm.SetMassWindowLow   (m_MassWindowLow);
  m_xZmm.SetMassWindowHigh  (m_MassWindowHigh);
  m_xZmm.SetLeadingMuonPtCut(m_LeadingMuonPtCut);
  m_xZmm.SetSecondMuonPtCut (m_SecondMuonPtCut);
  m_xZmm.SetOpeningAngleCut (m_OpeningAngleCut);
  m_xZmm.SetZ0GapCut        (m_Z0GapCut);
  m_xZmm.setDebugMode       (m_doDebug);
  m_xZmm.SetMuonQuality     (m_MuonQualityName);
  m_xZmm.SetSkipMSCheck     (m_skipMS);
  m_xZmm.SetMinLumiBlock    (m_minGoodLumiBlock);
  m_xZmm.SetMaxLumiBlock    (m_maxGoodLumiBlock);
  
  if (m_useCustomMuonSelector) {
    m_xZmm.SetMuonSelectionTool (m_muonSelector);    
  }
  // END new place for initialization of params

  if (m_doFourMuAnalysis) {
    m_4mu.Init();
    m_4mu.setDebugMode(true);
    if (m_useCustomMuonSelector) m_4mu.SetMuonSelectionTool (m_muonSelector);
    ATH_MSG_DEBUG(" IDPerfMonZmumu FourMuonAnalysis initialization completed " <<  m_Trk2VtxAssociationTool);
  }

  // m_Trk2VtxAssociationTool = std::make_unique<CP::TrackVertexAssociationTool>("Loose");
  if ( m_Trk2VtxAssociationTool.retrieve().isSuccess() )  {
    ATH_MSG_DEBUG(" IDPerfMonZmumu Success retrieving tool " <<  m_Trk2VtxAssociationTool);
  }
  else { // is Failure
    ATH_MSG_FATAL("IDPerfMonZmumu Failed to retrieve tool m_Trk2VtxAssociationTool " <<  m_Trk2VtxAssociationTool);
    return StatusCode::FAILURE;
  }

  ATH_CHECK ( this->bookTrees() );

  if (m_UseTrigger) { // load trigger decission and matching under user request
    if (m_triggerDecision.retrieve().isFailure()) {                                                                                                                                   
      ATH_MSG_FATAL("Unable to retrieve " << m_triggerDecision << " turn it off");                                                                                             
      return StatusCode::FAILURE;                                                                                                                                               
    }                                                                                                                                                                           
    else {                                                                                                                                                                        
      ATH_MSG_DEBUG("retrieved tool: " << m_triggerDecision );                                                                                                                  
    }

    if(m_triggerMatching.retrieve().isFailure()) {       
      ATH_MSG_FATAL("Unable to retrieve " << m_triggerMatching << " turn it off");                                                                                                
      return StatusCode::FAILURE;                                                                                                                                                 
    }                                                                                                                                                                             
    else {                                                                                                                                                                         
      ATH_MSG_INFO("retrieved tool: " << m_triggerDecision );                                                                                                                    
    }
  }

  ATH_MSG_DEBUG("** IDPerfMonZmumu::Initialize ** Completed **");
  return StatusCode::SUCCESS;
}


  
//==================================================================================
StatusCode IDPerfMonZmumu::bookTrees()
{
  m_h_cutflow = new TH1F("h_cutflow","cut flow histogram",11, -0.5, 9.5);

  ATH_MSG_DEBUG("initialize() ** bookTrees() ** m_commonTree name: " << m_commonTreeName.c_str());    
  ATH_MSG_DEBUG("                              m_defaultTree name: " << m_defaultTreeName.c_str());    
  ATH_MSG_DEBUG("                                   m_IDTree name: " << m_IDTreeName.c_str());    

  if (!m_commonTree) {
    ATH_MSG_INFO("initialize() ** defining m_commonTree with name: " << m_commonTreeName.c_str());    
    m_commonTree = new TTree((m_commonTreeName).c_str(), m_ValidationTreeDescription.c_str());

    m_commonTree->Branch("runNumber"           , &m_runNumber,  "runNumber/I");
    m_commonTree->Branch("eventNumber"         , &m_evtNumber,  "eventNumber/I");
    m_commonTree->Branch("lumi_block"          , &m_lumi_block, "lumi_block/I");
    m_commonTree->Branch("mu"                  , &m_event_mu,   "mu/I");
    m_commonTree->Branch("IDTrack_pt"          , &m_IDTrack_pt); 
    m_commonTree->Branch("IDTrack_eta"         , &m_IDTrack_eta); 
    m_commonTree->Branch("IDTrack_phi"         , &m_IDTrack_phi);
    m_commonTree->Branch("IDTrack_d0"          , &m_IDTrack_d0);
    m_commonTree->Branch("IDTrack_z0"          , &m_IDTrack_z0);
    m_commonTree->Branch("IDTrack_qoverp"      , &m_IDTrack_qoverp);
    m_commonTree->Branch("IDTrack_sigma_pt"    , &m_IDTrack_sigma_pt);
    m_commonTree->Branch("IDTrack_sigma_d0"    , &m_IDTrack_sigma_d0);
    m_commonTree->Branch("IDTrack_sigma_z0"    , &m_IDTrack_sigma_z0);
    m_commonTree->Branch("IDTrack_sigma_qoverp", &m_IDTrack_sigma_qoverp);
    m_commonTree->Branch("CBTrack_pt"          , &m_CBTrack_pt); 
    m_commonTree->Branch("CBTrack_eta"         , &m_CBTrack_eta); 
    m_commonTree->Branch("CBTrack_phi"         , &m_CBTrack_phi); 
    m_commonTree->Branch("CBTrack_d0"          , &m_CBTrack_d0); 
    m_commonTree->Branch("CBTrack_z0"          , &m_CBTrack_z0); 
    m_commonTree->Branch("CBTrack_qoverp"      , &m_CBTrack_qoverp);
    m_commonTree->Branch("CBTrack_sigma_pt"    , &m_CBTrack_sigma_pt);
    m_commonTree->Branch("CBTrack_sigma_d0"    , &m_CBTrack_sigma_d0);
    m_commonTree->Branch("CBTrack_sigma_z0"    , &m_CBTrack_sigma_z0);
    m_commonTree->Branch("CBTrack_sigma_qoverp", &m_CBTrack_sigma_qoverp);
    m_commonTree->Branch("Refit1_pt"           , &m_Refit1_pt); 
    m_commonTree->Branch("Refit1_eta"          , &m_Refit1_eta); 
    m_commonTree->Branch("Refit1_phi"          , &m_Refit1_phi); 
    m_commonTree->Branch("Refit1_d0"           , &m_Refit1_d0); 
    m_commonTree->Branch("Refit1_z0"           , &m_Refit1_z0); 
    m_commonTree->Branch("Refit1_qoverp"       , &m_Refit1_qoverp); 
    m_commonTree->Branch("Refit1_sigma_pt"     , &m_Refit1_sigma_pt);
    m_commonTree->Branch("Refit1_sigma_d0"     , &m_Refit1_sigma_d0);
    m_commonTree->Branch("Refit1_sigma_z0"     , &m_Refit1_sigma_z0);
    m_commonTree->Branch("Refit1_sigma_qoverp" , &m_Refit1_sigma_qoverp);
    m_commonTree->Branch("Refit2_pt"           , &m_Refit2_pt); 
    m_commonTree->Branch("Refit2_eta"          , &m_Refit2_eta); 
    m_commonTree->Branch("Refit2_phi"          , &m_Refit2_phi); 
    m_commonTree->Branch("Refit2_d0"           , &m_Refit2_d0); 
    m_commonTree->Branch("Refit2_z0"           , &m_Refit2_z0); 
    m_commonTree->Branch("Refit2_qoverp"       , &m_Refit2_qoverp); 
    m_commonTree->Branch("Refit2_sigma_pt"     , &m_Refit2_sigma_pt);
    m_commonTree->Branch("Refit2_sigma_d0"     , &m_Refit2_sigma_d0);
    m_commonTree->Branch("Refit2_sigma_z0"     , &m_Refit2_sigma_z0);
    m_commonTree->Branch("Refit2_sigma_qoverp" , &m_Refit2_sigma_qoverp);
    m_commonTree->Branch("Truth_pt"            , &m_Truth_pt); 
    m_commonTree->Branch("Truth_eta"           , &m_Truth_eta); 
    m_commonTree->Branch("Truth_phi"           , &m_Truth_phi); 
    m_commonTree->Branch("Truth_d0"            , &m_Truth_d0); 
    m_commonTree->Branch("Truth_z0"            , &m_Truth_z0); 
    m_commonTree->Branch("Truth_qoverp"        , &m_Truth_qoverp); 
    m_commonTree->Branch("Truth_parent"        , &m_Truth_parent);
    m_commonTree->Branch("numberOfBLayerHits"  , &m_nBLhits);
    m_commonTree->Branch("numberOfPixelHits"   , &m_nPIXhits);
    m_commonTree->Branch("numberOfSCTHits"     , &m_nSCThits);
    m_commonTree->Branch("numberOfTRTHits"     , &m_nTRThits);
  }

  if ( !m_defaultTree){
    ATH_MSG_INFO("initialize() ** defining m_defaultTree with name: " << m_defaultTreeName.c_str());    
    m_defaultTree = new TTree((m_defaultTreeName).c_str(), m_ValidationTreeDescription.c_str());

    m_defaultTree->Branch("runNumber"      ,  &m_runNumber,  "runNumber/I");
    m_defaultTree->Branch("eventNumber"    ,  &m_evtNumber,  "eventNumber/I");
    m_defaultTree->Branch("lumi_block"     ,  &m_lumi_block, "lumi_block/I");
    m_defaultTree->Branch("mu"             ,  &m_event_mu,   "mu/I");

    m_defaultTree->Branch("Negative_Px",  &m_negative_px,  "Negative_Px/D");
    m_defaultTree->Branch("Negative_Py",  &m_negative_py,  "Negative_Py/D");
    m_defaultTree->Branch("Negative_Pt",  &m_negative_pt,  "Negative_Pt/D");
    m_defaultTree->Branch("Negative_Pz",  &m_negative_pz,  "Negative_Pz/D");
    m_defaultTree->Branch("Negative_Phi", &m_negative_phi, "Negative_Phi/D");
    m_defaultTree->Branch("Negative_eta", &m_negative_eta, "Negative_Eta/D");
    m_defaultTree->Branch("Negative_z0",  &m_negative_z0,  "Negative_z0/D");
    m_defaultTree->Branch("Negative_d0",  &m_negative_d0,  "Negative_d0/D");
    m_defaultTree->Branch("Negative_z0_err",  &m_negative_z0_err,  "Negative_z0_err/D");
    m_defaultTree->Branch("Negative_d0_err",  &m_negative_d0_err,  "Negative_d0_err/D");
    m_defaultTree->Branch("Negative_sigma_pt",  &m_negative_sigma_pt,  "Negative_sigma_pt/D");

    m_defaultTree->Branch("Positive_Px",  &m_positive_px,  "Positive_Px/D");
    m_defaultTree->Branch("Positive_Py",  &m_positive_py,  "Positive_Py/D");
    m_defaultTree->Branch("Positive_Pt",  &m_positive_pt,  "Positive_Pt/D");
    m_defaultTree->Branch("Positive_Pz",  &m_positive_pz,  "Positive_Pz/D");
    m_defaultTree->Branch("Positive_Phi", &m_positive_phi, "Positive_Phi/D");
    m_defaultTree->Branch("Positive_eta", &m_positive_eta, "Positive_Eta/D");
    m_defaultTree->Branch("Positive_z0",  &m_positive_z0,  "Positive_z0/D");
    m_defaultTree->Branch("Positive_d0",  &m_positive_d0,  "Positive_d0/D");
    m_defaultTree->Branch("Positive_z0_err",  &m_positive_z0_err,  "Positive_z0_err/D");
    m_defaultTree->Branch("Positive_d0_err",  &m_positive_d0_err,  "Positive_d0_err/D");
    m_defaultTree->Branch("Positive_sigma_pt",  &m_positive_sigma_pt,  "Positive_sigma_pt/D");
    
    if (m_doIP) {
      m_defaultTree->Branch("Negative_d0_PV",      &m_negative_d0_PV   ,  "Negative_d0_PV/D");
      m_defaultTree->Branch("Negative_z0_PV",      &m_negative_z0_PV   ,  "Negative_z0_PV/D");
      m_defaultTree->Branch("Positive_z0_PV",      &m_positive_z0_PV,  "Positive_z0_PV/D");
      m_defaultTree->Branch("Positive_d0_PV",      &m_positive_d0_PV,  "Positive_d0_PV/D");

      m_defaultTree->Branch("Negative_d0_PVerr",   &m_negative_d0_PVerr,  "Negative_d0_PVerr/D");
      m_defaultTree->Branch("Negative_z0_PVerr",   &m_negative_z0_PVerr,  "Negative_z0_PVerr/D");
      m_defaultTree->Branch("Positive_z0_PVerr",   &m_positive_z0_PVerr,  "Positive_z0_PVerr/D");
      m_defaultTree->Branch("Positive_d0_PVerr",   &m_positive_d0_PVerr,  "Positive_d0_PVerr/D");

      m_defaultTree->Branch("pv_x",      &m_pv_x   ,  "pv_x/D");
      m_defaultTree->Branch("pv_y",      &m_pv_y   ,  "pv_y/D");
      m_defaultTree->Branch("pv_z",      &m_pv_z   ,  "pv_z/D");
      m_defaultTree->Branch("nTrkInVtx", &m_nTrkInVtx,  "nTrkInVtx/I");
    }
  }
  
  bool isTreeNone = false;
  if (m_IDTreeName.find("none") != std::string::npos) isTreeNone = true;
  if ( !m_IDTree and !isTreeNone ){
    ATH_MSG_INFO("initialize() ** defining IDPerfMonZmumu m_IDTree with name: " << m_IDTreeName.c_str());    
    m_IDTree = new TTree((m_IDTreeName).c_str(), m_ValidationTreeDescription.c_str());
    
    m_IDTree->Branch("runNumber"      ,  &m_runNumber,  "runNumber/I");
    m_IDTree->Branch("eventNumber"    ,  &m_evtNumber,  "eventNumber/I");
    m_IDTree->Branch("lumi_bLock"     ,  &m_lumi_block,  "lumi_block/I");
    m_IDTree->Branch("mu"             ,  &m_event_mu,  "mu/I");

    m_IDTree->Branch("Negative_Px",  &m_negative_px,  "Negative_Px/D");
    m_IDTree->Branch("Negative_Py",  &m_negative_py,  "Negative_Py/D");
    m_IDTree->Branch("Negative_Pt",  &m_negative_pt,  "Negative_Pt/D");
    m_IDTree->Branch("Negative_Pz",  &m_negative_pz,  "Negative_Pz/D");
    m_IDTree->Branch("Negative_Phi", &m_negative_phi, "Negative_Phi/D");
    m_IDTree->Branch("Negative_Eta", &m_negative_eta, "Negative_Eta/D");
    m_IDTree->Branch("Negative_z0",  &m_negative_z0,  "Negative_z0/D");
    m_IDTree->Branch("Negative_d0",  &m_negative_d0,  "Negative_d0/D");
    m_IDTree->Branch("Negative_z0_err",  &m_negative_z0_err,  "Negative_z0_err/D");
    m_IDTree->Branch("Negative_d0_err",  &m_negative_d0_err,  "Negative_d0_err/D");
    m_IDTree->Branch("Negative_sigma_pt",  &m_negative_sigma_pt,  "Negative_sigma_pt/D");

    m_IDTree->Branch("Positive_Px",  &m_positive_px,  "Positive_Px/D");
    m_IDTree->Branch("Positive_Py",  &m_positive_py,  "Positive_Py/D");
    m_IDTree->Branch("Positive_Pt",  &m_positive_pt,  "Positive_Pt/D");
    m_IDTree->Branch("Positive_Pz",  &m_positive_pz,  "Positive_Pz/D");
    m_IDTree->Branch("Positive_Phi", &m_positive_phi, "Positive_Phi/D");
    m_IDTree->Branch("Positive_Eta", &m_positive_eta, "Positive_Eta/D");
    m_IDTree->Branch("Positive_z0",  &m_positive_z0,  "Positive_z0/D");
    m_IDTree->Branch("Positive_d0",  &m_positive_d0,  "Positive_d0/D");
    m_IDTree->Branch("Positive_z0_err",  &m_positive_z0_err,  "Positive_z0_err/D");
    m_IDTree->Branch("Positive_d0_err",  &m_positive_d0_err,  "Positive_d0_err/D");
    m_IDTree->Branch("Positive_sigma_pt",  &m_positive_sigma_pt,  "Positive_sigma_pt/D");
    
    if(m_doIP){
      m_IDTree->Branch("Negative_d0_PV",      &m_negative_d0_PV   ,  "Negative_d0_PV/D");
      m_IDTree->Branch("Negative_z0_PV",      &m_negative_z0_PV   ,  "Negative_z0_PV/D");
      m_IDTree->Branch("Positive_z0_PV",      &m_positive_z0_PV,  "Positive_z0_PV/D");
      m_IDTree->Branch("Positive_d0_PV",      &m_positive_d0_PV,  "Positive_d0_PV/D");
      
      m_IDTree->Branch("Negative_d0_PVerr",   &m_negative_d0_PVerr,  "Negative_d0_PVerr/D");
      m_IDTree->Branch("Negative_z0_PVerr",   &m_negative_z0_PVerr,  "Negative_z0_PVerr/D");
      m_IDTree->Branch("Positive_z0_PVerr",   &m_positive_z0_PVerr,  "Positive_z0_PVerr/D");
      m_IDTree->Branch("Positive_d0_PVerr",   &m_positive_d0_PVerr,  "Positive_d0_PVerr/D");

      m_IDTree->Branch("pv_x",      &m_pv_x   ,  "pv_x/D");
      m_IDTree->Branch("pv_y",      &m_pv_y   ,  "pv_y/D");
      m_IDTree->Branch("pv_z",      &m_pv_z   ,  "pv_z/D");
      m_IDTree->Branch("nTrkInVtx", &m_nTrkInVtx,  "nTrkInVtx/I");
    }
  }

  // dealing with tree for Refit1 tracks
  isTreeNone = false;
  if (m_refit1TreeName.find("none") != std::string::npos) isTreeNone = true;
  if ( m_doRefit && !m_refit1Tree && !isTreeNone ){
    ATH_MSG_INFO("initialize() ** defining IDPerfMonZmumu m_refit1Tree with name: " << m_refit1TreeName.c_str());    
    m_refit1Tree = new TTree((m_refit1TreeName).c_str(), m_ValidationTreeDescription.c_str()); 
    
    m_refit1Tree->Branch("runNumber"      ,  &m_runNumber,  "runNumber/I");
    m_refit1Tree->Branch("eventNumber"    ,  &m_evtNumber,  "eventNumber/I");
    m_refit1Tree->Branch("lumi_block"     ,  &m_lumi_block,  "lumi_block/I");
    m_refit1Tree->Branch("mu"             ,  &m_event_mu,  "mu/I");
    m_refit1Tree->Branch("preScale"       ,  &m_triggerPrescale, "preScale/I");

    m_refit1Tree->Branch("Negative_Px",  &m_negative_px,  "Negative_Px/D");
    m_refit1Tree->Branch("Negative_Py",  &m_negative_py,  "Negative_Py/D");
    m_refit1Tree->Branch("Negative_Pt",  &m_negative_pt,  "Negative_Pt/D");
    m_refit1Tree->Branch("Negative_Pz",  &m_negative_pz,  "Negative_Pz/D");
    m_refit1Tree->Branch("Negative_Phi", &m_negative_phi, "Negative_Phi/D");
    m_refit1Tree->Branch("Negative_Eta", &m_negative_eta, "Negative_Eta/D");
    m_refit1Tree->Branch("Negative_z0",  &m_negative_z0,  "Negative_z0/D");
    m_refit1Tree->Branch("Negative_d0",  &m_negative_d0,  "Negative_d0/D");
    m_refit1Tree->Branch("Negative_z0_manualBS",  &m_positive_z0_manualBS,  "Negative_z0_manualBS/D");
    m_refit1Tree->Branch("Negative_d0_manualBS",  &m_positive_d0_manualBS,  "Negative_d0_manualBS/D");
    m_refit1Tree->Branch("Negative_z0_err",  &m_negative_z0_err,  "Negative_z0_err/D");
    m_refit1Tree->Branch("Negative_d0_err",  &m_negative_d0_err,  "Negative_d0_err/D");
    m_refit1Tree->Branch("Negative_sigma_pt",  &m_negative_sigma_pt,  "Negative_sigma_pt/D");

    m_refit1Tree->Branch("Positive_Px",  &m_positive_px,  "Positive_Px/D");
    m_refit1Tree->Branch("Positive_Py",  &m_positive_py,  "Positive_Py/D");
    m_refit1Tree->Branch("Positive_Pt",  &m_positive_pt,  "Positive_Pt/D");
    m_refit1Tree->Branch("Positive_Pz",  &m_positive_pz,  "Positive_Pz/D");
    m_refit1Tree->Branch("Positive_Phi", &m_positive_phi, "Positive_Phi/D");
    m_refit1Tree->Branch("Positive_eta", &m_positive_eta, "Positive_Eta/D");
    m_refit1Tree->Branch("Positive_z0",  &m_positive_z0,  "Positive_z0/D");
    m_refit1Tree->Branch("Positive_d0",  &m_positive_d0,  "Positive_d0/D");
    m_refit1Tree->Branch("Positive_z0_manualBS",  &m_positive_z0_manualBS,  "Positive_z0_manualBS/D");
    m_refit1Tree->Branch("Positive_d0_manualBS",  &m_positive_d0_manualBS,  "Positive_d0_manualBS/D");
    m_refit1Tree->Branch("Positive_z0_err",  &m_positive_z0_err,  "Positive_z0_err/D");
    m_refit1Tree->Branch("Positive_d0_err",  &m_positive_d0_err,  "Positive_d0_err/D");
    m_refit1Tree->Branch("Positive_sigma_pt",  &m_positive_sigma_pt,  "Positive_sigma_pt/D");

    if(m_doIP){
      m_refit1Tree->Branch("Negative_d0_PV",      &m_negative_d0_PV   ,  "Negative_d0_PV/D");
      m_refit1Tree->Branch("Negative_z0_PV",      &m_negative_z0_PV   ,  "Negative_z0_PV/D");
      m_refit1Tree->Branch("Positive_z0_PV",      &m_positive_z0_PV,     "Positive_z0_PV/D");
      m_refit1Tree->Branch("Positive_d0_PV",      &m_positive_d0_PV,     "Positive_d0_PV/D");
      
      m_refit1Tree->Branch("Negative_d0_PVerr",      &m_negative_d0_PVerr   ,  "Negative_d0_PVerr/D");
      m_refit1Tree->Branch("Negative_z0_PVerr",      &m_negative_z0_PVerr   ,  "Negative_z0_PVerr/D");
      m_refit1Tree->Branch("Positive_z0_PVerr",     &m_positive_z0_PVerr,     "Positive_z0_PVerr/D");
      m_refit1Tree->Branch("Positive_d0_PVerr",     &m_positive_d0_PVerr,     "Positive_d0_PVerr/D");

      m_refit1Tree->Branch("pv_x",      &m_pv_x   ,  "pv_x/D");
      m_refit1Tree->Branch("pv_y",      &m_pv_y   ,  "pv_y/D");
      m_refit1Tree->Branch("pv_z",      &m_pv_z   ,  "pv_z/D");
      m_refit1Tree->Branch("nTrkInVtx", &m_nTrkInVtx,  "nTrkInVtx/I");
    }    
  }

  // dealing with tree for Refit2 tracks
  isTreeNone = false;
  if (m_refit2TreeName.find("none") != std::string::npos) isTreeNone = true;
  if ( m_doRefit && !m_refit2Tree && !isTreeNone){
    ATH_MSG_INFO("initialize() ** defining m_refit2Tree with name: " << m_refit2TreeName.c_str());
    m_refit2Tree = new TTree((m_refit2TreeName).c_str(), m_ValidationTreeDescription.c_str());

    m_refit2Tree->Branch("runNumber"      ,  &m_runNumber,  "runNumber/I");
    m_refit2Tree->Branch("eventNumber"    ,  &m_evtNumber,  "eventNumber/I");
    m_refit2Tree->Branch("lumi_block"     ,  &m_lumi_block,  "lumi_block/I");
    m_refit2Tree->Branch("mu"             ,  &m_event_mu,  "mu/I");
    m_refit2Tree->Branch("preScale"       ,  &m_triggerPrescale, "preScale/I");

    m_refit2Tree->Branch("Negative_Px",  &m_negative_px,  "Negative_Px/D");
    m_refit2Tree->Branch("Negative_Py",  &m_negative_py,  "Negative_Py/D");
    m_refit2Tree->Branch("Negative_Pt",  &m_negative_pt,  "Negative_Pt/D");
    m_refit2Tree->Branch("Negative_Pz",  &m_negative_pz,  "Negative_Pz/D");
    m_refit2Tree->Branch("Negative_Phi", &m_negative_phi, "Negative_Phi/D");
    m_refit2Tree->Branch("Negative_Eta", &m_negative_eta, "Negative_Eta/D");
    m_refit2Tree->Branch("Negative_z0",  &m_negative_z0,  "Negative_z0/D");
    m_refit2Tree->Branch("Negative_d0",  &m_negative_d0,  "Negative_d0/D");
    m_refit2Tree->Branch("Negative_z0_manualBS",  &m_positive_z0_manualBS,  "Negative_z0_manualBS/D");
    m_refit2Tree->Branch("Negative_d0_manualBS",  &m_positive_d0_manualBS,  "Negative_d0_manualBS/D");
    m_refit2Tree->Branch("Negative_z0_err",  &m_negative_z0_err,  "Negative_z0_err/D");
    m_refit2Tree->Branch("Negative_d0_err",  &m_negative_d0_err,  "Negative_d0_err/D");
    m_refit2Tree->Branch("Negative_sigma_pt",  &m_negative_sigma_pt,  "Negative_sigma_pt/D");

    m_refit2Tree->Branch("Positive_Px",  &m_positive_px,  "Positive_Px/D");
    m_refit2Tree->Branch("Positive_Py",  &m_positive_py,  "Positive_Py/D");
    m_refit2Tree->Branch("Positive_Pt",  &m_positive_pt,  "Positive_Pt/D");
    m_refit2Tree->Branch("Positive_Pz",  &m_positive_pz,  "Positive_Pz/D");
    m_refit2Tree->Branch("Positive_Phi", &m_positive_phi, "Positive_Phi/D");
    m_refit2Tree->Branch("Positive_Eta", &m_positive_eta, "Positive_Eta/D");
    m_refit2Tree->Branch("Positive_z0",  &m_positive_z0,  "Positive_z0/D");
    m_refit2Tree->Branch("Positive_d0",  &m_positive_d0,  "Positive_d0/D");
    m_refit2Tree->Branch("Positive_z0_manualBS",  &m_positive_z0_manualBS,  "Positive_z0_manualBS/D");
    m_refit2Tree->Branch("Positive_d0_manualBS",  &m_positive_d0_manualBS,  "Positive_d0_manualBS/D");
    m_refit2Tree->Branch("Positive_z0_err",  &m_positive_z0_err,  "Positive_z0_err/D");
    m_refit2Tree->Branch("Positive_d0_err",  &m_positive_d0_err,  "Positive_d0_err/D");
    m_refit2Tree->Branch("Positive_sigma_pt",  &m_positive_sigma_pt,  "Positive_sigma_pt/D");


    if(m_doIP){
      m_refit2Tree->Branch("Negative_d0_PV",      &m_negative_d0_PV   ,  "Negative_d0_PV/D");
      m_refit2Tree->Branch("Negative_z0_PV",      &m_negative_z0_PV   ,  "Negative_z0_PV/D");
      m_refit2Tree->Branch("Positive_d0_PV",      &m_positive_d0_PV   ,  "Positive_d0_PV/D");
      m_refit2Tree->Branch("Positive_z0_PV",      &m_positive_z0_PV   ,  "Positive_z0_PV/D");
      
      
      m_refit2Tree->Branch("Negative_z0_PVerr",  &m_negative_z0_PVerr,  "Negative_z0_PVerr/D");
      m_refit2Tree->Branch("Negative_d0_PVerr",  &m_negative_d0_PVerr,  "Negative_d0_PVerr/D");
      m_refit2Tree->Branch("Positive_z0_PVerr",  &m_positive_z0_PVerr,  "Positive_z0_PVerr/D");
      m_refit2Tree->Branch("Positive_d0_PVerr",  &m_positive_d0_PVerr,  "Positive_d0_PVerr/D");

      m_refit2Tree->Branch("pv_x",      &m_pv_x   ,  "pv_x/D");
      m_refit2Tree->Branch("pv_y",      &m_pv_y   ,  "pv_y/D");
      m_refit2Tree->Branch("pv_z",      &m_pv_z   ,  "pv_z/D");

      m_refit2Tree->Branch("nTrkInVtx", &m_nTrkInVtx,  "nTrkInVtx/I");
    }    
  }
  
  isTreeNone = false;
  if (m_combTreeName.find("none") != std::string::npos) isTreeNone = true;
  if( !m_combTree && !m_skipMS && !isTreeNone){
    ATH_MSG_INFO("initialize() ** defining IDPerfMonZmumu m_combTree with name: " << m_combTreeName.c_str());
    m_combTree = new TTree((m_combTreeName).c_str(), m_ValidationTreeDescription.c_str());

    m_combTree->Branch("runNumber"      ,  &m_runNumber,  "runNumber/I");
    m_combTree->Branch("eventNumber"      ,  &m_evtNumber,  "eventNumber/I");
    m_combTree->Branch("lumi_block"      ,  &m_lumi_block,  "lumi_block/I");
    m_combTree->Branch("mu"             ,  &m_event_mu,  "mu/I");
    m_combTree->Branch("preScale"       ,  &m_triggerPrescale, "preScale/I");

    m_combTree->Branch("Negative_Px",  &m_negative_px,  "Negative_Px/D");
    m_combTree->Branch("Negative_Py",  &m_negative_py,  "Negative_Py/D");
    m_combTree->Branch("Negative_Pt",  &m_negative_pt,  "Negative_Pt/D");
    m_combTree->Branch("Negative_Pz",  &m_negative_pz,  "Negative_Pz/D");
    m_combTree->Branch("Negative_Phi", &m_negative_phi, "Negative_Phi/D");
    m_combTree->Branch("Negative_eta", &m_negative_eta, "Negative_eta/D");
    m_combTree->Branch("Negative_z0",  &m_negative_z0,  "Negative_z0/D");
    m_combTree->Branch("Negative_d0",  &m_negative_d0,  "Negative_d0/D");
    m_combTree->Branch("Negative_z0_err",  &m_negative_z0_err,  "Negative_z0_err/D");
    m_combTree->Branch("Negative_d0_err",  &m_negative_d0_err,  "Negative_d0_err/D");
    m_combTree->Branch("Negative_sigma_pt",  &m_negative_sigma_pt,  "Negative_sigma_pt/D");

    m_combTree->Branch("Positive_Px",  &m_positive_px,  "Positive_Px/D");
    m_combTree->Branch("Positive_Py",  &m_positive_py,  "Positive_Py/D");
    m_combTree->Branch("Positive_Pt",  &m_positive_pt,  "Positive_Pt/D");
    m_combTree->Branch("Positive_Pz",  &m_positive_pz,  "Positive_Pz/D");
    m_combTree->Branch("Positive_Phi", &m_positive_phi, "Positive_Phi/D");
    m_combTree->Branch("Positive_eta", &m_positive_eta, "Positive_Eta/D");
    m_combTree->Branch("Positive_z0",  &m_positive_z0,  "Positive_z0/D");
    m_combTree->Branch("Positive_d0",  &m_positive_d0,  "Positive_d0/D");
    m_combTree->Branch("Positive_z0_err",  &m_positive_z0_err,  "Positive_z0_err/D");
    m_combTree->Branch("Positive_d0_err",  &m_positive_d0_err,  "Positive_d0_err/D");
    m_combTree->Branch("Positive_sigma_pt",  &m_positive_sigma_pt,  "Positive_sigma_pt/D");
    
    if(m_doIP){
      m_combTree->Branch("Negative_z0_PV",  &m_negative_z0_PV,  "Negative_z0_PV/D");
      m_combTree->Branch("Negative_d0_PV",  &m_negative_d0_PV,  "Negative_d0_PV/D");
      m_combTree->Branch("Positive_z0_PV",  &m_positive_z0_PV,  "Positive_z0_PV/D");
      m_combTree->Branch("Positive_d0_PV",  &m_positive_d0_PV,  "Positive_d0_PV/D");

      m_combTree->Branch("Negative_z0_PVerr",  &m_negative_z0_PVerr,  "Negative_z0_PVerr/D");
      m_combTree->Branch("Negative_d0_PVerr",  &m_negative_d0_PVerr,  "Negative_d0_PVerr/D");
      m_combTree->Branch("Positive_z0_PVerr",  &m_positive_z0_PVerr,  "Positive_z0_PVerr/D");
      m_combTree->Branch("Positive_d0_PVerr",  &m_positive_d0_PVerr,  "Positive_d0_PVerr/D");

      m_combTree->Branch("pv_x",      &m_pv_x   ,  "pv_x/D");
      m_combTree->Branch("pv_y",      &m_pv_y   ,  "pv_y/D");
      m_combTree->Branch("pv_z",      &m_pv_z   ,  "pv_z/D");

      m_combTree->Branch("nTrkInVtx", &m_nTrkInVtx,  "nTrkInVtx/I");
    }
  }

  
  isTreeNone = false;
  if (m_MSTreeName.find("none") != std::string::npos) isTreeNone = true;
  if( !m_MSTree && !m_skipMS && !isTreeNone){
    ATH_MSG_INFO("initialize() ** defining IDPerfMon MSTree ");

    m_MSTree = new TTree(m_MSTreeName.c_str(), m_ValidationTreeDescription.c_str());

    m_MSTree->Branch("runNumber"      ,  &m_runNumber,  "runNumber/I");
    m_MSTree->Branch("eventNumber"      ,  &m_evtNumber,  "eventNumber/I");
    m_MSTree->Branch("lumi_block"      ,  &m_lumi_block,  "lumi_block/I");
    m_MSTree->Branch("preScale"       ,  &m_triggerPrescale, "preScale/I");
    
    m_MSTree->Branch("Negative_Px",  &m_negative_px,  "Negative_Px/D");
    m_MSTree->Branch("Negative_Py",  &m_negative_py,  "Negative_Py/D");
    m_MSTree->Branch("Negative_Pt",  &m_negative_pt,  "Negative_Pt/D");
    m_MSTree->Branch("Negative_Pz",  &m_negative_pz,  "Negative_Pz/D");
    m_MSTree->Branch("Negative_Phi", &m_negative_phi, "Negative_Phi/D");
    m_MSTree->Branch("Negative_Eta", &m_negative_eta, "Negative_Eta/D");
    m_MSTree->Branch("Negative_z0",  &m_negative_z0,  "Negative_z0/D");
    m_MSTree->Branch("Negative_d0",  &m_negative_d0,  "Negative_d0/D");
    m_MSTree->Branch("Negative_z0_err",  &m_negative_z0_err,  "Negative_z0_err/D");
    m_MSTree->Branch("Negative_d0_err",  &m_negative_d0_err,  "Negative_d0_err/D");
    m_MSTree->Branch("Negative_sigma_pt",  &m_negative_sigma_pt,  "Negative_sigma_pt/D");

    m_MSTree->Branch("Positive_Px",  &m_positive_px,  "Positive_Px/D");
    m_MSTree->Branch("Positive_Py",  &m_positive_py,  "Positive_Py/D");
    m_MSTree->Branch("Positive_Pt",  &m_positive_pt,  "Positive_Pt/D");
    m_MSTree->Branch("Positive_Pz",  &m_positive_pz,  "Positive_Pz/D");
    m_MSTree->Branch("Positive_Phi", &m_positive_phi, "Positive_Phi/D");
    m_MSTree->Branch("Positive_Eta", &m_positive_eta, "Positive_Eta/D");
    m_MSTree->Branch("Positive_z0",  &m_positive_z0,  "Positive_z0/D");
    m_MSTree->Branch("Positive_d0",  &m_positive_d0,  "Positive_d0/D");
    m_MSTree->Branch("Positive_z0_err",  &m_positive_z0_err,  "Positive_z0_err/D");
    m_MSTree->Branch("Positive_d0_err",  &m_positive_d0_err,  "Positive_d0_err/D");
    m_MSTree->Branch("Positive_sigma_pt",  &m_positive_sigma_pt,  "Positive_sigma_pt/D");
  }

  if( m_isMC && !m_truthTree ){
    m_truthTree = new TTree(m_truthTreeName.c_str(), m_ValidationTreeDescription.c_str());

    m_truthTree->Branch("runNumber"      ,  &m_runNumber,  "runNumber/I");
    m_truthTree->Branch("eventNumber"      ,  &m_evtNumber,  "eventNumber/I");
    m_truthTree->Branch("lumi_block"      ,  &m_lumi_block,  "lumi_block/I");
    m_truthTree->Branch("preScale"       ,  &m_triggerPrescale, "preScale/I");

    m_truthTree->Branch("Negative_Px",  &m_negative_px,  "Negative_Px/D");
    m_truthTree->Branch("Negative_Py",  &m_negative_py,  "Negative_Py/D");
    m_truthTree->Branch("Negative_Pt",  &m_negative_pt,  "Negative_Pt/D");
    m_truthTree->Branch("Negative_Pz",  &m_negative_pz,  "Negative_Pz/D");
    m_truthTree->Branch("Negative_Phi", &m_negative_phi, "Negative_Phi/D");
    m_truthTree->Branch("Negative_Eta", &m_negative_eta, "Negative_Eta/D");
    m_truthTree->Branch("Negative_z0",  &m_negative_z0,  "Negative_z0/D");
    m_truthTree->Branch("Negative_d0",  &m_negative_d0,  "Negative_d0/D");
    m_truthTree->Branch("Negative_z0_err",  &m_negative_z0_err,  "Negative_z0_err/D");
    m_truthTree->Branch("Negative_d0_err",  &m_negative_d0_err,  "Negative_d0_err/D");
    m_truthTree->Branch("Negative_parent",  &m_negative_parent,  "Negative_parent/I");

    m_truthTree->Branch("Positive_Px",  &m_positive_px,  "Positive_Px/D");
    m_truthTree->Branch("Positive_Py",  &m_positive_py,  "Positive_Py/D");
    m_truthTree->Branch("Positive_Pt",  &m_positive_pt,  "Positive_Pt/D");
    m_truthTree->Branch("Positive_Pz",  &m_positive_pz,  "Positive_Pz/D");
    m_truthTree->Branch("Positive_Phi", &m_positive_phi, "Positive_Phi/D");
    m_truthTree->Branch("Positive_Eta", &m_positive_eta, "Positive_Eta/D");
    m_truthTree->Branch("Positive_z0",  &m_positive_z0,  "Positive_z0/D");
    m_truthTree->Branch("Positive_d0",  &m_positive_d0,  "Positive_d0/D");
    m_truthTree->Branch("Positive_z0_err",  &m_positive_z0_err,  "Positive_z0_err/D");
    m_truthTree->Branch("Positive_d0_err",  &m_positive_d0_err,  "Positive_d0_err/D");
    m_truthTree->Branch("Positive_parent",  &m_positive_parent,  "Positive_parent/I");
  }

  if (m_doFourMuAnalysis && !m_FourMuTree) {
    ATH_MSG_INFO("initialize() ** defining m_FourMuTree ");
    m_FourMuTree = new TTree((m_FourMuTreeName).c_str(), "Four Muon monitoring");

    m_FourMuTree->Branch("runNumber"  ,  &m_runNumber,       "runNumber/I");
    m_FourMuTree->Branch("eventNumber",  &m_evtNumber,       "eventNumber/I");
    m_FourMuTree->Branch("lumi_block" ,  &m_lumi_block,      "lumi_block/I");
    m_FourMuTree->Branch("mu"         ,  &m_event_mu,        "mu/I");
    m_FourMuTree->Branch("preScale"   ,  &m_triggerPrescale, "preScale/I");
    m_FourMuTree->Branch("nVertex"    ,  &m_nVertex,         "nVertex/I");

    m_FourMuTree->Branch("Negative_1_Px",     &m_negative_px,   "Negative_1_Px/D");
    m_FourMuTree->Branch("Negative_1_Py",     &m_negative_py,   "Negative_1_Py/D");
    m_FourMuTree->Branch("Negative_1_Pz",     &m_negative_pz,   "Negative_1_Pz/D");
    m_FourMuTree->Branch("Negative_1_z0",     &m_negative_z0,   "Negative_1_z0/D");
    m_FourMuTree->Branch("Negative_1_d0",     &m_negative_d0,   "Negative_1_d0/D");
    m_FourMuTree->Branch("Negative_1_z0_err", &m_negative_z0_err,  "Negative_1_z0_err/D");
    m_FourMuTree->Branch("Negative_1_d0_err", &m_negative_d0_err,  "Negative_1_d0_err/D");
    m_FourMuTree->Branch("Negative_1_vtx",    &m_negative_1_vtx,  "Negative_1_vtx/I"); // vertex identifier

    m_FourMuTree->Branch("Negative_2_Px",     &m_negative_2_px,  "Negative_2_Px/D");
    m_FourMuTree->Branch("Negative_2_Py",     &m_negative_2_py,  "Negative_2_Py/D");
    m_FourMuTree->Branch("Negative_2_Pz",     &m_negative_2_pz,  "Negative_2_Pz/D");
    m_FourMuTree->Branch("Negative_2_z0",     &m_negative_2_z0,  "Negative_2_z0/D");
    m_FourMuTree->Branch("Negative_2_d0",     &m_negative_2_d0,  "Negative_2_d0/D");
    m_FourMuTree->Branch("Negative_2_z0_err", &m_negative_2_z0_err,  "Negative_2_z0_err/D");
    m_FourMuTree->Branch("Negative_2_d0_err", &m_negative_2_d0_err,  "Negative_2_d0_err/D");
    m_FourMuTree->Branch("Negative_2_vtx",    &m_negative_2_vtx,  "Negative_2_vtx/I");

    m_FourMuTree->Branch("Positive_1_Px",     &m_positive_px,     "Positive_1_Px/D");
    m_FourMuTree->Branch("Positive_1_Py",     &m_positive_py,     "Positive_1_Py/D");
    m_FourMuTree->Branch("Positive_1_Pz",     &m_positive_pz,     "Positive_1_Pz/D");
    m_FourMuTree->Branch("Positive_1_z0",     &m_positive_z0,     "Positive_1_z0/D");
    m_FourMuTree->Branch("Positive_1_d0",     &m_positive_d0,     "Positive_1_d0/D");
    m_FourMuTree->Branch("Positive_1_z0_err", &m_positive_z0_err, "Positive_1_z0_err/D");
    m_FourMuTree->Branch("Positive_1_d0_err", &m_positive_d0_err, "Positive_1_d0_err/D");
    m_FourMuTree->Branch("Positive_1_vtx",    &m_positive_1_vtx,  "Positive_1_vtx/I");

    m_FourMuTree->Branch("Positive_2_Px",     &m_positive_2_px,    "Positive_1_Px/D");
    m_FourMuTree->Branch("Positive_2_Py",     &m_positive_2_py,    "Positive_1_Py/D");
    m_FourMuTree->Branch("Positive_2_Pz",     &m_positive_2_pz,    "Positive_1_Pz/D");
    m_FourMuTree->Branch("Positive_2_z0",     &m_positive_2_z0,    "Positive_1_z0/D");
    m_FourMuTree->Branch("Positive_2_d0",     &m_positive_2_d0,    "Positive_1_d0/D");
    m_FourMuTree->Branch("Positive_2_z0_err", &m_positive_2_z0_err,"Positive_1_z0_err/D");
    m_FourMuTree->Branch("Positive_2_d0_err", &m_positive_2_d0_err,"Positive_1_d0_err/D");
    m_FourMuTree->Branch("Positive_2_vtx",    &m_positive_2_vtx,   "Positive_2_vtx/I");

    // electrons
    m_FourMuTree->Branch("ElNegative_1_Px",     &m_el_negative1_px,     "ElNegative_1_Px/D");
    m_FourMuTree->Branch("ElNegative_1_Py",     &m_el_negative1_py,     "ElNegative_1_Py/D");
    m_FourMuTree->Branch("ElNegative_1_Pz",     &m_el_negative1_pz,     "ElNegative_1_Pz/D");
    m_FourMuTree->Branch("ElNegative_1_z0",     &m_el_negative1_z0,     "ElNegative_1_z0/D");
    m_FourMuTree->Branch("ElNegative_1_d0",     &m_el_negative1_d0,     "ElNegative_1_d0/D");
    m_FourMuTree->Branch("ElNegative_1_z0_err", &m_el_negative1_z0_err, "ElNegative_1_z0_err/D");
    m_FourMuTree->Branch("ElNegative_1_d0_err", &m_el_negative1_d0_err, "ElNegative_1_d0_err/D");
    m_FourMuTree->Branch("ElNegative_1_vtx",    &m_el_negative1_vtx,    "ElNegative_1_vtx/I"); // vertex identifier

    m_FourMuTree->Branch("ElNegative_2_Px",     &m_el_negative2_px,     "ElNegative_2_Px/D");
    m_FourMuTree->Branch("ElNegative_2_Py",     &m_el_negative2_py,     "ElNegative_2_Py/D");
    m_FourMuTree->Branch("ElNegative_2_Pz",     &m_el_negative2_pz,     "ElNegative_2_Pz/D");
    m_FourMuTree->Branch("ElNegative_2_z0",     &m_el_negative2_z0,     "ElNegative_2_z0/D");
    m_FourMuTree->Branch("ElNegative_2_d0",     &m_el_negative2_d0,     "ElNegative_2_d0/D");
    m_FourMuTree->Branch("ElNegative_2_z0_err", &m_el_negative2_z0_err, "ElNegative_2_z0_err/D");
    m_FourMuTree->Branch("ElNegative_2_d0_err", &m_el_negative2_d0_err, "ElNegative_2_d0_err/D");
    m_FourMuTree->Branch("ElNegative_2_vtx",    &m_el_negative2_vtx,    "ElNegative_2_vtx/I"); // vertex identifier

    m_FourMuTree->Branch("ElPositive_1_Px",     &m_el_positive1_px,     "ElPositive_1_Px/D");
    m_FourMuTree->Branch("ElPositive_1_Py",     &m_el_positive1_py,     "ElPositive_1_Py/D");
    m_FourMuTree->Branch("ElPositive_1_Pz",     &m_el_positive1_pz,     "ElPositive_1_Pz/D");
    m_FourMuTree->Branch("ElPositive_1_z0",     &m_el_positive1_z0,     "ElPositive_1_z0/D");
    m_FourMuTree->Branch("ElPositive_1_d0",     &m_el_positive1_d0,     "ElPositive_1_d0/D");
    m_FourMuTree->Branch("ElPositive_1_z0_err", &m_el_positive1_z0_err, "ElPositive_1_z0_err/D");
    m_FourMuTree->Branch("ElPositive_1_d0_err", &m_el_positive1_d0_err, "ElPositive_1_d0_err/D");
    m_FourMuTree->Branch("ElPositive_1_vtx",    &m_el_positive1_vtx,    "ElPositive_1_vtx/I"); // vertex identifier

    m_FourMuTree->Branch("ElPositive_2_Px",     &m_el_positive2_px,     "ElPositive_2_Px/D");
    m_FourMuTree->Branch("ElPositive_2_Py",     &m_el_positive2_py,     "ElPositive_2_Py/D");
    m_FourMuTree->Branch("ElPositive_2_Pz",     &m_el_positive2_pz,     "ElPositive_2_Pz/D");
    m_FourMuTree->Branch("ElPositive_2_z0",     &m_el_positive2_z0,     "ElPositive_2_z0/D");
    m_FourMuTree->Branch("ElPositive_2_d0",     &m_el_positive2_d0,     "ElPositive_2_d0/D");
    m_FourMuTree->Branch("ElPositive_2_z0_err", &m_el_positive2_z0_err, "ElPositive_2_z0_err/D");
    m_FourMuTree->Branch("ElPositive_2_d0_err", &m_el_positive2_d0_err, "ElPositive_2_d0_err/D");
    m_FourMuTree->Branch("ElPositive_2_vtx",    &m_el_positive2_vtx,    "ElPositive_2_vtx/I"); // vertex identifier

    // other quantities
    m_FourMuTree->Branch("minv4mu",   &m_4mu_minv,  "minv4mu/D");
    m_FourMuTree->Branch("pv_x",      &m_pv_x   ,   "pv_x/D");
    m_FourMuTree->Branch("pv_y",      &m_pv_y   ,   "pv_y/D");
    m_FourMuTree->Branch("pv_z",      &m_pv_z   ,   "pv_z/D");
    m_FourMuTree->Branch("nTrkInVtx", &m_nTrkInVtx, "nTrkInVtx/I");
    
    m_FourMuTree->Branch("met",       &m_met,     "met/D");
    m_FourMuTree->Branch("metphi",    &m_metphi,  "metphi/D");
  }
  
  // now register the Trees
  ATH_MSG_INFO("initialize() Going to register the mu+mu- trees");
  ITHistSvc* tHistSvc = nullptr;
  if (service("THistSvc",tHistSvc).isFailure()){
    ATH_MSG_ERROR("initialize() Could not find Hist Service -> Switching ValidationMode Off !");
    m_validationMode = false;
  }
  
  if (m_commonTree) {
    if ((tHistSvc->regTree(m_commonTreeFolder, m_commonTree)).isSuccess() ) {
      ATH_MSG_INFO("initialize() commonTree succesfully registered!");
    }
    else {
      ATH_MSG_ERROR("initialize() Could not register the validation commonTree -> Switching ValidationMode Off !");
      delete m_commonTree; m_commonTree = nullptr;
      m_validationMode = false;
    }
  }

  if (m_IDTree) {
    if ((tHistSvc->regTree(m_IDTreeFolder, m_IDTree)).isSuccess() ) {
      ATH_MSG_INFO("initialize() IDTree succesfully registered!");
    }
    else {
      ATH_MSG_ERROR("initialize() Could not register the validation IDTree -> Switching ValidationMode Off !");
      delete m_IDTree; m_IDTree = nullptr;
      m_validationMode = false;
    }
  }
  
  if ( m_skipMS ) {
    ATH_MSG_INFO("CBtree has to be skipped: flag ON");
    delete m_combTree; m_combTree = nullptr;
  }
  else {
    if (m_combTree) {
      if ((tHistSvc->regTree(m_combTreeFolder, m_combTree)).isSuccess() ) {
	ATH_MSG_INFO("initialize() CBTree succesfully registered!");
      }
      else {
	ATH_MSG_ERROR("initialize() Could not register the validation CBTree -> Switching ValidationMode Off !");
	delete m_combTree; m_combTree = nullptr;
	m_validationMode = false;
      }
    }
  }
  
  if ( m_skipMS ) {
    ATH_MSG_INFO("MStree has to be skipped: flag ON");
    delete m_MSTree; m_MSTree = nullptr;
  }
  else {
    if (m_MSTree) {
      if ( (tHistSvc->regTree(m_MSTreeFolder, m_MSTree)).isSuccess()  ){
	ATH_MSG_INFO("initialize() MSTree succesfully registered!");
      }
      else {
	ATH_MSG_ERROR("initialize() Could not register the validation MSTree -> Switching ValidationMode Off !");
	delete m_MSTree; m_MSTree = nullptr;
	m_validationMode = false;
      }
    }
  }

  if( m_doRefit ){
    if (m_refit1Tree) {
      if ((tHistSvc->regTree(m_refit1TreeFolder, m_refit1Tree)).isSuccess() ) {
	ATH_MSG_INFO("initialize() Refit1 Tree succesfully registered!");
      }
      else{
	ATH_MSG_ERROR("initialize() Could not register the validation Tree -> Switching ValidationMode Off !");
	delete m_refit1Tree; m_refit1Tree = nullptr;
	m_validationMode = false;
      }
    }

    if (m_refit2Tree) {
      if ((tHistSvc->regTree(m_refit2TreeFolder, m_refit2Tree)).isSuccess() ) {
	ATH_MSG_INFO("initialize() Refit2 Tree succesfully registered!");
      }
      else {
	ATH_MSG_ERROR("initialize() Could not register the validation Tree -> Switching ValidationMode Off !");
	delete m_refit2Tree; m_refit2Tree = nullptr;
	m_validationMode = false;
      }
    }
  }
  
  if (m_isMC) {
    if ((tHistSvc->regTree(m_truthTreeFolder, m_truthTree)).isSuccess() ) {
      ATH_MSG_INFO("initialize() truthTree Tree succesfully registered!");
    }
    else {
      ATH_MSG_ERROR("initialize() Could not register the validation truth Tree -> Switching ValidationMode Off !");
      delete m_truthTree; m_truthTree = nullptr;
      m_validationMode = false;
    }
  }
  
  if (m_doFourMuAnalysis) {
    if ((tHistSvc->regTree(m_FourMuTreeFolder, m_FourMuTree)).isSuccess() ) {
      ATH_MSG_INFO("initialize() FourMu Tree succesfully registered!");
    }
    else {
      ATH_MSG_ERROR("initialize() Could not register the validation FourMu Tree -> Switching ValidationMode Off !");
      delete m_FourMuTree; m_FourMuTree = nullptr;
      m_validationMode = false;
    }
  }

  return StatusCode::SUCCESS;
}


//==================================================================================
void IDPerfMonZmumu::RegisterHistograms()
{
  return;
}


//==================================================================================
StatusCode IDPerfMonZmumu::execute()
{
  ATH_MSG_DEBUG("** IDPerfMonZmumu::execute ** START **");
  
  SG::ReadHandle<xAOD::EventInfo> eventInfo (m_EventInfoKey, getContext());
  if(eventInfo.isValid()) {
    m_runNumber = eventInfo->runNumber();
    m_evtNumber = eventInfo->eventNumber();
    m_lumi_block = eventInfo->lumiBlock();
    m_event_mu = eventInfo->actualInteractionsPerCrossing();
    ATH_MSG_DEBUG(" Execute() starting on --> Run: " << m_runNumber << "  event: " << m_evtNumber << "   Lumiblock: " << m_lumi_block);
  }
  else {
    ATH_MSG_DEBUG(" IDPerfMonZmumu::execute evtStore->retrieve (eventInfo) failed ..  trying another strategy.. ");
    SG::ReadHandle<xAOD::EventInfo> eventInfo2(m_EventInfoKey);
    if (eventInfo2.isValid()) {
      m_runNumber = eventInfo2->runNumber();
      m_evtNumber = eventInfo2->eventNumber();
      m_lumi_block = eventInfo2->lumiBlock();
      m_event_mu = eventInfo2->actualInteractionsPerCrossing();
      ATH_MSG_DEBUG(" Execute() starting on --> Run: " << m_runNumber << "  event: " << m_evtNumber << "   Lumiblock: " << m_lumi_block);
    }
      
    else{
      ATH_MSG_ERROR("** IDPerfMonZmumu::execute ** Could not retrieve event info.");
    }
  }

  /// --  START 4 lepton analysis
  if (m_doFourMuAnalysis) {
    ATH_MSG_DEBUG(" ** IDPerfMonZmumu::execute ** calling FourLeptonAnalysis()...");
    StatusCode fourLeptAnaStatus = this->RunFourLeptonAnalysis();
    if (fourLeptAnaStatus.isSuccess()) ATH_MSG_INFO (" ** IDPerfMonZmumu::execute ** RunFourLeptonAnalysis() SUCCESS -> found a new event");
  }
  /// -- END 4 lepton analysis


  // check if the muon-pair passed the resonance selection cuts:
  ATH_MSG_DEBUG(" ** IDPerfMonZmumu::execute ** calling dimuon analysis m_xZmm.Reco()...");
  if( m_xZmm.Reco( m_lumi_block ) ){
    ATH_MSG_INFO(   "  Run: " << m_runNumber 
		 << "  event: " << m_evtNumber 
		 << "  Lumiblock: " << m_lumi_block 
		 << "  Invariant mass = " << m_xZmm.GetInvMass() << " GeV  ** SUCCESS **");
  }
  else {
    ATH_MSG_DEBUG(   "  Run: " << m_runNumber
		  << "  event: " << m_evtNumber 
		  << "  Lumiblock: " << m_lumi_block
		  << "  Failed dimuon reconstruction. m_xZmm.Reco() returned FALSE ");
    return StatusCode::SUCCESS;
  }

  //
  // If this point is reached -> there is a good mu+mu- pair that stisfies all selection cuts
  //

  if (m_UseTrigger) {
    StatusCode isTriggerPassed = CheckTriggerStatusAndPrescale ();
    if (isTriggerPassed == StatusCode::SUCCESS) {
      ATH_MSG_DEBUG("Trigger passed -> accept event");
    }
    else{
      ATH_MSG_DEBUG("Trigger Failed -> reject event --> leave event");
      return StatusCode::SUCCESS;
    }
  }

  // std::cout << " ** IDPerfMonZmumu ** extracting muon_pos and muon_neg... " << std::endl;
  const xAOD::Muon* muon_pos = m_xZmm.getCombMuon(m_xZmm.getPosMuon(ZmumuEvent::CB));
  const xAOD::Muon* muon_neg = m_xZmm.getCombMuon(m_xZmm.getNegMuon(ZmumuEvent::CB));

  const xAOD::TrackParticle* ppos_comb = nullptr; const xAOD::Vertex* ppos_comb_v = nullptr;
  const xAOD::TrackParticle* pneg_comb = nullptr; const xAOD::Vertex* pneg_comb_v = nullptr;
    
  //To protect against failures of the estimation
  StatusCode success_pos = StatusCode::FAILURE;
  StatusCode success_neg = StatusCode::FAILURE;

  if (muon_pos && muon_neg) { // if both combined muons exist and were sucessfully retrieved        
    ATH_MSG_DEBUG("** IDPerfMonZmumu::execute ** combined muons exist ** retrieving their m_trackparticleName: " << m_trackParticleName.c_str());
    
    if (m_trackParticleName.find("InnerDetectorTrackParticles") != std::string::npos) {
      ATH_MSG_INFO("** IDPerfMonZmumu::execute ** Retrieving InnerDetectorTrackParticles of the accepted muons");
      ppos_comb = muon_pos->trackParticle(xAOD::Muon::InnerDetectorTrackParticle);
      pneg_comb = muon_neg->trackParticle(xAOD::Muon::InnerDetectorTrackParticle);
      
      if (!ppos_comb || !pneg_comb) {
	ATH_MSG_WARNING("** IDPerfMonZmumu::execute ** InnerDetectorTrackParticles are requested but they are not present. Exiting event.");
	return StatusCode::SUCCESS;
      }
    } // InnerDetectorTrackParticles
    if (m_trackParticleName.find("CombinedTrackParticle") != std::string::npos) {
      // 
      ATH_MSG_DEBUG("** IDPerfMonZmumu::execute ** Retrieving CombinedTrackParticles of the accepted muons");
      ppos_comb = muon_pos->trackParticle(xAOD::Muon::CombinedTrackParticle);
      pneg_comb = muon_neg->trackParticle(xAOD::Muon::CombinedTrackParticle);
      
      if (!ppos_comb || !pneg_comb){
	ATH_MSG_WARNING( "** IDPerfMonZmumu::execute ** CombinedTrackParticles are requested but they are not present. Exiting event.");
	return StatusCode::SUCCESS;
      }      
    } // combined tracks
    

    // double check
    if (ppos_comb && pneg_comb) {
      ATH_MSG_DEBUG("** IDPerfMonZmumu::execute ** successfull retrieval of muons as " << m_trackParticleName << ". ppos_comb & pneg_comb both exist");
    }
    else {
      ATH_MSG_WARNING("** IDPerfMonZmumu::execute ** problems retrieving muons as " << m_trackParticleName <<". ppos_comb or pneg_comb are not available");
    }


    // vertex
    const EventContext& ctx = Gaudi::Hive::currentContext();
    SG::ReadHandle<xAOD::VertexContainer> vertices { m_vertexKey, ctx };
    for (const auto V : *vertices) {
      if (V->vertexType() == xAOD::VxType::VertexType::PriVtx) {
	// primaryVertex =V;
	ppos_comb_v = V;
	pneg_comb_v = V;
	break;
      }
    } // vertex
    
    // before continuing check both particles have vertex
    if (!ppos_comb_v || !pneg_comb_v){
      ATH_MSG_WARNING( "Some or all of the requested particles have no vertex. Exiting event");
      return StatusCode::SUCCESS;
    }      
  } // if (muon_pos && muon_neg) 
  else { // this is the else of --> if (muon_pos && muon_neg)
    ATH_MSG_WARNING("** IDPerfMonZmumu::execute ** Could not find CombinedMuon pos/neg in event"
		    << "  Run: " << m_runNumber 
		    << "  event: " << m_evtNumber 
		    << "  Lumiblock: " << m_lumi_block );
    return StatusCode::SUCCESS;
  }
  
  // 
  // If this point is reached -> there is a good mu+mu- pair and the muons have been associated to a vertex
  // So, ntuple can be filled
  //
  if ( m_xZmm.AcceptEvent() ) { 
    ATH_MSG_DEBUG("** IDPerfMonZmumu::execute ** Going to fill ntuples for Run: " << m_runNumber 
		  << "  event: " << m_evtNumber 
		  << "  Lumiblock: " << m_lumi_block 
		  << "  Invariant mass = " << m_xZmm.GetInvMass() << " GeV  ** some preliminaries are due though...");

    if (m_useTrackSelectionTool) {
      // The track selection is applied to the InnerDetectorTrackParticle of the muons
      if ( !m_selTool->accept(muon_pos->trackParticle(xAOD::Muon::InnerDetectorTrackParticle)) ) {
	ATH_MSG_DEBUG("Exiting because the ID segment of muon_pos do not pass the TrackSelection");
	return StatusCode::SUCCESS;
      }	
      if ( !m_selTool->accept(muon_neg->trackParticle(xAOD::Muon::InnerDetectorTrackParticle)) ) {
	ATH_MSG_DEBUG("Exiting because the ID segment of muon_neg do not pass the TrackSelection");
	return StatusCode::SUCCESS;
      }
      ATH_MSG_DEBUG("** IDPerfMonZmumu::execute ** Good news ** pos and neg muon->trackParticle(xAOD::Muon::InnerDetectorTrackParticle) pass the trackSelection :)");
    } 
    else {
      ATH_MSG_DEBUG("** IDPerfMonZmumu::execute ** Track selection is not in use ");
    }
    
    ATH_MSG_DEBUG("** IDPerfMonZmumu::execute ** >> before fill rec with default: " << m_trackParticleName << " tracks << ");
    
    if (m_storeZmumuNtuple) {
      // reset vectors
      this->ResetCommonNtupleVectors();
      
      // Fill Inner Detector Tree 
      ATH_MSG_DEBUG("-- >> going to fill ID muons params << --");
      success_pos = FillRecParametersTP (muon_pos->trackParticle(xAOD::Muon::InnerDetectorTrackParticle), 
					 muon_pos->trackParticle(xAOD::Muon::InnerDetectorTrackParticle), 
					 muon_pos->trackParticle(xAOD::Muon::InnerDetectorTrackParticle)->charge(),
					 ppos_comb_v);
      success_neg = FillRecParametersTP (muon_neg->trackParticle(xAOD::Muon::InnerDetectorTrackParticle), 
					 muon_neg->trackParticle(xAOD::Muon::InnerDetectorTrackParticle), 
					 muon_neg->trackParticle(xAOD::Muon::InnerDetectorTrackParticle)->charge(),
					 pneg_comb_v);
      
      if (success_pos && success_neg) {
	// before filling the ntuple, extract the hits information. This is filled once and it is used for all track collections
	this->ExtractIDHitsInformation(muon_pos, muon_neg);
	
	// Fill ntuple
	if (m_IDTree != nullptr) {
	  ATH_MSG_DEBUG("-- Filling m_IDTree ntuple " << m_IDTree->GetName() << " entry " << m_IDTree->GetEntries()
			<< " for run: " << m_runNumber 
			<< "  event: " << m_evtNumber 
			<< "  Lumiblock: " << m_lumi_block 
			<< "  Invariant mass = " << m_xZmm.GetInvMass() << " GeV ");
	}
	ATH_MSG_DEBUG("mu+  --> pxyz " << m_positive_px 
		      << ", " << m_positive_py
		      << ", " << m_positive_pz
		      << "  pt: " << muon_pos->trackParticle(xAOD::Muon::InnerDetectorTrackParticle)->pt()
		      << "  d0: " << m_positive_d0 
		      << "  z0: " << m_positive_z0
		      << "  d0unc: " << m_positive_d0_err
		      << "  sigma_pt: " << m_positive_sigma_pt);
	ATH_MSG_DEBUG("mu-  --> pxyz: " << m_negative_px
		      << ", " << m_negative_py
		      << ", " << m_negative_pz
		      << "  pt: " << muon_neg->trackParticle(xAOD::Muon::InnerDetectorTrackParticle)->pt()
		      << "  d0: " << m_negative_d0
		      << "  z0: " << m_negative_z0
		      << "  d0unc: " << m_negative_d0_err
		      << "  sigma_pt: " << m_negative_sigma_pt);
	// ntuple variables have been filled in FillRecParametersTP
	if (m_IDTree) m_IDTree->Fill();

	// Fill vectors for common ntuple
	m_IDTrack_pt.push_back(m_positive_pt);
	m_IDTrack_pt.push_back(m_negative_pt);
	m_IDTrack_eta.push_back(m_positive_eta);
	m_IDTrack_eta.push_back(m_negative_eta);
	m_IDTrack_phi.push_back(m_positive_phi);
	m_IDTrack_phi.push_back(m_negative_phi);
	m_IDTrack_d0.push_back(m_positive_d0);
	m_IDTrack_d0.push_back(m_negative_d0);
	m_IDTrack_z0.push_back(m_positive_z0);
	m_IDTrack_z0.push_back(m_negative_z0);
	m_IDTrack_qoverp.push_back(m_positive_qoverp);
	m_IDTrack_qoverp.push_back(m_negative_qoverp);
	
	m_IDTrack_sigma_pt.push_back(m_positive_sigma_pt);
	m_IDTrack_sigma_pt.push_back(m_negative_sigma_pt);
	m_IDTrack_sigma_d0.push_back(m_positive_d0_err);
	m_IDTrack_sigma_d0.push_back(m_negative_d0_err);
	m_IDTrack_sigma_z0.push_back(m_positive_z0_err);
	m_IDTrack_sigma_z0.push_back(m_negative_z0_err);
	m_IDTrack_sigma_qoverp.push_back(m_positive_sigma_qoverp);
	m_IDTrack_sigma_qoverp.push_back(m_negative_sigma_qoverp);
      }
      else {
	ATH_MSG_DEBUG("** IDPerfMonZmumu::execute ** not filling IDTracks in combTree due to problems with muon_xxx->trackParticle(xAOD::Muon::InnerDetectorTrackParticle)");
      }
      // End of fill ID Tree 
      
      //
      // combined muons ntuple
      if ((!m_skipMS && m_combTree) || m_commonTree ) { // if skipMS -> no combined muons
	success_pos = FillRecParametersTP(muon_pos->trackParticle(xAOD::Muon::CombinedTrackParticle), 
					  muon_pos->trackParticle(xAOD::Muon::InnerDetectorTrackParticle), 
					  ppos_comb->charge(), 
					  ppos_comb_v);
	ATH_MSG_DEBUG("** IDPerfMonZmumu::execute ** success_pos TP pt: " << m_positive_pt << " GeV");
	success_neg = FillRecParametersTP(muon_neg->trackParticle(xAOD::Muon::CombinedTrackParticle), 
					  muon_neg->trackParticle(xAOD::Muon::InnerDetectorTrackParticle), 
					  pneg_comb->charge(), 
					  pneg_comb_v);    
	ATH_MSG_DEBUG("** IDPerfMonZmumu::excute ** success_neg TP pt: " << m_negative_pt << " GeV");

	if (success_pos && success_neg) {
	  if (m_combTree != nullptr) {
	    ATH_MSG_DEBUG("-- Filling m_combTree ntuple " << m_combTree->GetName() << " entry " << m_combTree->GetEntries() 
			  << "  run: " << m_runNumber 
			  << "  event: " << m_evtNumber 
			  << "  Lumiblock: " << m_lumi_block 
			  << "  Invariant mass = " << m_xZmm.GetInvMass() << " GeV ");
	  }
	  ATH_MSG_DEBUG("mu+ --> pxyz " << m_positive_px 
			<< ", " << m_positive_py
			<< ", " << m_positive_pz
			<< "  pt: " << m_positive_pt
			<< "  d0: " << m_positive_d0 
			<< "  z0: " << m_positive_z0
			<< "  d0unc: " << m_positive_d0_err);
	  ATH_MSG_DEBUG("mu- --> pxyz: " << m_negative_px
			<< ", " << m_negative_py
			<< ", " << m_negative_pz 
			<< "  pt: " << m_negative_pt
			<< "  d0: " << m_negative_d0
			<< "  z0: " << m_negative_z0
			<< "  d0unc: " << m_negative_d0_err);
	  // ntuple variables have been filled in FillRecParameters
	  if (m_combTree) m_combTree->Fill();
	}
	// Fill vectors for common ntuple
	m_CBTrack_pt.push_back(m_positive_pt);
	m_CBTrack_pt.push_back(m_negative_pt);
	m_CBTrack_eta.push_back(m_positive_eta);
	m_CBTrack_eta.push_back(m_negative_eta);
	m_CBTrack_phi.push_back(m_positive_phi);
	m_CBTrack_phi.push_back(m_negative_phi);
	m_CBTrack_d0.push_back(m_positive_d0);
	m_CBTrack_d0.push_back(m_negative_d0);
	m_CBTrack_z0.push_back(m_positive_z0);
	m_CBTrack_z0.push_back(m_negative_z0);
	m_CBTrack_qoverp.push_back(m_positive_qoverp);
	m_CBTrack_qoverp.push_back(m_negative_qoverp);
	
	m_CBTrack_sigma_pt.push_back(m_positive_sigma_pt);
	m_CBTrack_sigma_pt.push_back(m_negative_sigma_pt);
	m_CBTrack_sigma_d0.push_back(m_positive_d0_err);
	m_CBTrack_sigma_d0.push_back(m_negative_d0_err);
	m_CBTrack_sigma_z0.push_back(m_positive_z0_err);
	m_CBTrack_sigma_z0.push_back(m_negative_z0_err);
	m_CBTrack_sigma_qoverp.push_back(m_positive_sigma_qoverp);
	m_CBTrack_sigma_qoverp.push_back(m_negative_sigma_qoverp);
      } // if skipMS -> no combined muons

      
      
      // MS ntuple
      if (!m_skipMS && m_MSTree) { // if skipMS -> no MS tracks
	ATH_MSG_DEBUG("-- >> going to fill MS muons params << --");
	success_pos = FillRecParametersTP(m_xZmm.getMSTrack(m_xZmm.getPosMuon(ZmumuEvent::CB)),
					  muon_pos->trackParticle(xAOD::Muon::InnerDetectorTrackParticle),
					  ppos_comb->charge(), 
					  ppos_comb_v);

	success_neg = FillRecParametersTP(m_xZmm.getMSTrack(m_xZmm.getNegMuon(ZmumuEvent::CB)),
					  muon_neg->trackParticle(xAOD::Muon::InnerDetectorTrackParticle),
					  pneg_comb->charge(),
					  pneg_comb_v);
	if (success_pos && success_neg) { 
	  if (m_MSTree != nullptr) ATH_MSG_DEBUG("-- Filling m_MSTree " << m_MSTree->GetName() << " entry " << m_MSTree->GetEntries() 
						 << "  run: " << m_runNumber 
						 << "  event: " << m_evtNumber 
						 << "  Lumiblock: " << m_lumi_block 
						 << "  Invariant mass = " << m_xZmm.GetInvMass() << " GeV ");
	  ATH_MSG_DEBUG("mu+ --> pxyz " << m_positive_px 
			<< ", " << m_positive_py
			<< "  pt: " << m_xZmm.getMSTrack(m_xZmm.getPosMuon(ZmumuEvent::CB))->pt()
			<< "  d0: " << m_positive_d0 
			<< "  z0: " << m_positive_z0
			<< "  d0unc: " << m_positive_d0_err);
	  ATH_MSG_DEBUG("mu- --> pxyz: " << m_negative_px
			<< ", " << m_negative_py
			<< "  pt: " << m_xZmm.getMSTrack(m_xZmm.getNegMuon(ZmumuEvent::CB))->pt()
			<< "  d0: " << m_negative_d0
			<< "  z0: " << m_negative_z0
			<< "  d0unc: " << m_negative_d0_err);
	  // ntuple variables have been filled in FillRecParametersTP
	  if (m_MSTree) m_MSTree->Fill();
	}
	else {
	  ATH_MSG_INFO("FAILED filling m_MSTree " 
		       << "  for run: " << m_runNumber 
		       << "  event: " << m_evtNumber 
		       << "  Lumiblock: " << m_lumi_block);
	}
	// MS ntuple
      }
    } // if store Zmumu ntuple
    
    // changed refitting to combined particles since run II DESDM_ZMUMU did not store InDetTrackParticles
    if( m_doRefit ) {
      ATH_MSG_DEBUG("** IDPerfMonZmumu::execute ** Going to build TrackCollections: muonTrks, muonTrksRefit1 and muonTrksRefit2");
      TrackCollection* muonTrks        = new TrackCollection(SG::OWN_ELEMENTS);
      TrackCollection* muonTrksRefit1  = new TrackCollection(SG::OWN_ELEMENTS);
      TrackCollection* muonTrksRefit2  = new TrackCollection(SG::OWN_ELEMENTS);
          
      ATH_MSG_DEBUG("** IDPerfMonZmumu::execute ** Going to refit and store the track parameters "); 
      Trk::Track* defaultMuonTrk1{};
      Trk::Track* defaultMuonTrk2{};
      Trk::Track* refit1MuonTrk1{};
      Trk::Track* refit2MuonTrk1{};
      Trk::Track* refit1MuonTrk2{};
      Trk::Track* refit2MuonTrk2{};
      
      StatusCode fitStatus;
      //save default and refit track parameters
      const EventContext& ctx = Gaudi::Hive::currentContext();
      if( ppos_comb->track() ) {
	defaultMuonTrk1 = new Trk::Track(*ppos_comb->track());
	
	IegammaTrkRefitterTool::Cache fitResult{};
	fitStatus = m_TrackRefitter1->refitTrack( ctx, ppos_comb->track(), fitResult );  
	if (fitStatus.isFailure()) {
	  ATH_MSG_DEBUG("** IDPerfMonZmumu::execute ** Track Refit1 Failed for ppos_comb->track(). Skipping Event");
	  return StatusCode::SUCCESS;
	} else {
	  refit1MuonTrk1 = fitResult.refittedTrack.release();
	  muonTrksRefit1->push_back(refit1MuonTrk1);
	  ATH_MSG_DEBUG("** IDPerfMonZmumu::execute ** Track Refit1 Success of ppos_comb->track()." 
			<< " Original pt: " << ppos_comb->track()->perigeeParameters()->pT() 
			<< "  track refit pt: " << refit1MuonTrk1->perigeeParameters()->pT() );
	}
	
	
	fitStatus = m_TrackRefitter2->refitTrack( ctx, ppos_comb->track(), fitResult );
	if (fitStatus.isFailure()) {
	  ATH_MSG_DEBUG("** IDPerfMonZmumu::execute ** Track Refit2 Failed for ppos_comb->track(). Skipping Event");
	  return StatusCode::SUCCESS;
	} else {
	  refit2MuonTrk1 = fitResult.refittedTrack.release();
	  muonTrksRefit2->push_back(refit2MuonTrk1);
	  ATH_MSG_DEBUG("** IDPerfMonZmumu::execute ** Track Refit2 Success of ppos_comb->track()." 
			<< " Original pt: " << ppos_comb->track()->perigeeParameters()->pT() 
			<< "  track refit pt: " << refit1MuonTrk1->perigeeParameters()->pT() );
	}
      }
      
      if( pneg_comb->track() ) {
	defaultMuonTrk2 = new Trk::Track(*pneg_comb->track());
	
	IegammaTrkRefitterTool::Cache fitResult {};
	fitStatus = m_TrackRefitter1->refitTrack( ctx, pneg_comb->track(), fitResult);
	if (fitStatus.isFailure()) {
	  ATH_MSG_DEBUG("** IDPerfMonZmumu::execute ** Track Refit1 Failed. Skipping Event");
	  return StatusCode::SUCCESS;
	} else {
	  refit1MuonTrk2 = fitResult.refittedTrack.release();
	  muonTrksRefit1->push_back(refit1MuonTrk2);
	  ATH_MSG_DEBUG("** IDPerfMonZmumu::execute ** Track Refit1 Success of pneg_comb->track()." 
			<< " Original pt: " << pneg_comb->track()->perigeeParameters()->pT() 
			<< "  track refit pt: " << refit1MuonTrk2->perigeeParameters()->pT() );
	}
	
	
	fitStatus = m_TrackRefitter2->refitTrack( ctx, pneg_comb->track(), fitResult );
	if (fitStatus.isFailure()) {
	  ATH_MSG_DEBUG("Track Refit2 Failed. Skipping Event");
	  return StatusCode::SUCCESS;
	} else {
	  refit2MuonTrk2 = fitResult.refittedTrack.release();
	  muonTrksRefit2->push_back(refit2MuonTrk2);
	  ATH_MSG_DEBUG("** IDPerfMonZmumu::execute ** Track Refit2 Success of pneg_comb->track()." 
			<< " Original pt: " << pneg_comb->track()->perigeeParameters()->pT() 
			<< "  track refit pt: " << refit2MuonTrk2->perigeeParameters()->pT() );
	}
	
	if (muonTrksRefit1->size() != 2) ATH_MSG_WARNING("** IDPerfMonZmumu::execute ** WARNING ** size of muonTrksRefit1: " << muonTrksRefit1->size());
	if (muonTrksRefit2->size() != 2) ATH_MSG_WARNING("** IDPerfMonZmumu::execute ** WARNING ** size of muonTrksRefit2: " << muonTrksRefit2->size());
      }
      
      //save tracks to storegrate, they can be used by InDetAlignmentMonitoring
      muonTrks->push_back(defaultMuonTrk1);
      muonTrks->push_back(defaultMuonTrk2);

      StatusCode sc = evtStore()->record(muonTrks, m_outputTracksName);
      if (sc.isSuccess()) {
	ATH_MSG_DEBUG ("Stored "<< muonTrks->size() << " " << m_outputTracksName <<" into StoreGate");
      }
      else{
	ATH_MSG_WARNING ("Failed storing " << m_outputTracksName);
      }
      
      if (muonTrksRefit1->size() > 1) { // keep track collection if at least 2 muons were found
	sc = evtStore()->record(muonTrksRefit1, m_outputTracksName + "Refit1");
	if (sc.isSuccess()) {
	  ATH_MSG_DEBUG ("Stored "<< muonTrksRefit1->size() << " " << m_outputTracksName + "Refit1" << " into StoreGate");
	} else {
	  ATH_MSG_WARNING ("Failed storing " << m_outputTracksName + "Refit1");
	}
      }
      
      if (muonTrksRefit2->size() > 1) { // keep track collection if at least 2 muons were found
	sc = evtStore()->record(muonTrksRefit2, m_outputTracksName + "Refit2");
	if (sc.isSuccess()) {
	  ATH_MSG_DEBUG ("Stored "<< muonTrksRefit2->size() << " " << m_outputTracksName + "Refit2" << " into StoreGate");
	} else {
	  ATH_MSG_WARNING ("Failed storing " << m_outputTracksName + "Refit2");
	}
      }
      
      
      //fill refit1 ID parameters
      if (muonTrksRefit1->size()<2) {
	ATH_MSG_WARNING("** IDPerfMonZmumu::execute ** Refit1 muon tracks are missing!");
      }
      else {
	ATH_MSG_DEBUG("** IDPerfMonZmumu::execute **  going to fill refit1tree ");
	success_pos = FillRecParametersSimple (refit1MuonTrk1, ppos_comb->charge(), ppos_comb_v);
	success_neg = FillRecParametersSimple (refit1MuonTrk2, pneg_comb->charge(), pneg_comb_v);
	
	if (m_storeZmumuNtuple) {
	  if (success_pos && success_neg) {
	    if (m_refit1Tree) {
	      ATH_MSG_DEBUG("-- Filling ntuple " << m_refit1Tree->GetName() << " entry " << m_refit1Tree->GetEntries() 
			    << " for run: " << m_runNumber 
			    << "  event: " << m_evtNumber 
			    << "  Lumiblock: " << m_lumi_block 
			    << "  Invariant mass = " << m_xZmm.GetInvMass() << " GeV ");
	    }
	    ATH_MSG_DEBUG("Negative p: (" << m_negative_px << ", "
			  << m_negative_py << ", "
			  << m_negative_pz << ") "
			  << "  d0: " << m_negative_d0
			  << "  z0: " << m_negative_z0
			  << "  sigma_pt: " << m_positive_sigma_pt);
	    ATH_MSG_DEBUG("Positive p: (" << m_positive_px << ","
			  << m_positive_py << ","
			  << m_positive_pz << ") "
			  << "  d0: " << m_positive_d0
			  << "  z0: " << m_positive_z0	  
			  << "  sigma_pt: " << m_negative_sigma_pt);
	    if (m_refit1Tree) {
	      m_refit1Tree->Fill();
	    }
	    // Fill vectors for common ntuple
	    m_Refit1_pt.push_back(m_positive_pt);
	    m_Refit1_pt.push_back(m_negative_pt);	    
	    m_Refit1_eta.push_back(m_positive_eta);
	    m_Refit1_eta.push_back(m_negative_eta);
	    m_Refit1_phi.push_back(m_positive_phi);
	    m_Refit1_phi.push_back(m_negative_phi);
	    m_Refit1_d0.push_back(m_positive_d0);
	    m_Refit1_d0.push_back(m_negative_d0);
	    m_Refit1_z0.push_back(m_positive_z0);
	    m_Refit1_z0.push_back(m_negative_z0);

	    m_Refit1_sigma_pt.push_back(m_positive_sigma_pt);
	    m_Refit1_sigma_pt.push_back(m_negative_sigma_pt);
	    m_Refit1_sigma_d0.push_back(m_positive_d0_err);
	    m_Refit1_sigma_d0.push_back(m_negative_d0_err);
	    m_Refit1_sigma_z0.push_back(m_positive_z0_err);
	    m_Refit1_sigma_z0.push_back(m_negative_z0_err);
	  } // if (success_pos && success_neg) 
	} // if (m_storeZmumuNtuple) 
      } // enough refit1 tracks
      
      //fill refit2 ID parameters
      if (muonTrksRefit2->size()<2) {
	ATH_MSG_WARNING("** IDPerfMonZmumu::execute ** Refit2 muon tracks are missing!");
      }
      else{
	ATH_MSG_DEBUG("-- >> going to fill refit2params << --");
	success_pos = FillRecParametersSimple (refit2MuonTrk1, ppos_comb->charge(), ppos_comb_v);
	success_neg = FillRecParametersSimple (refit2MuonTrk2, pneg_comb->charge(), pneg_comb_v);
	
	if (m_storeZmumuNtuple) {
	  if (success_pos && success_neg) {
	    if (m_refit2Tree) { 
	      ATH_MSG_DEBUG("-- Filling " << m_refit2Tree->GetName() << " entry " << m_refit2Tree->GetEntries() 
			    << "  run: " << m_runNumber 
			    << "  event: " << m_evtNumber 
			    << "  Lumiblock: " << m_lumi_block 
			    << "  Invariant mass = " << m_xZmm.GetInvMass() << " GeV ");
	    }
	    ATH_MSG_DEBUG("Negative p: (" << m_negative_px << ", "
			  << m_negative_py << ", "
			  << m_negative_pz << ") "
			  << "  d0: " << m_negative_d0
			  << "  z0: " << m_negative_z0
			  << "  sigma_pt: " << m_positive_sigma_pt);
	    ATH_MSG_DEBUG("Positive p: (" << m_positive_px << ","
			  << m_positive_py << ","
			  << m_positive_pz << ") "
			  << "  d0: " << m_positive_d0
			  << "  z0: " << m_positive_z0	  
			  << "  sigma_pt: " << m_negative_sigma_pt);
	    if (m_refit2Tree) {
	      m_refit2Tree->Fill();
	    }
	    // Fill vectors for common ntuple
	    m_Refit2_pt.push_back(m_positive_pt);
	    m_Refit2_pt.push_back(m_negative_pt);
	    m_Refit2_eta.push_back(m_positive_eta);
	    m_Refit2_eta.push_back(m_negative_eta);
	    m_Refit2_phi.push_back(m_positive_phi);
	    m_Refit2_phi.push_back(m_negative_phi);
	    m_Refit2_d0.push_back(m_positive_d0);
	    m_Refit2_d0.push_back(m_negative_d0);
	    m_Refit2_z0.push_back(m_positive_z0);
	    m_Refit2_z0.push_back(m_negative_z0);

	    m_Refit2_sigma_pt.push_back(m_positive_sigma_pt);
	    m_Refit2_sigma_pt.push_back(m_negative_sigma_pt);
	    m_Refit2_sigma_d0.push_back(m_positive_d0_err);
	    m_Refit2_sigma_d0.push_back(m_negative_d0_err);
	    m_Refit2_sigma_z0.push_back(m_positive_z0_err);
	    m_Refit2_sigma_z0.push_back(m_negative_z0_err);
	  } // if (success_pos && success_neg) 
	} // if (m_storeZmumuNtuple) 
      } // enough refit2 tracks
      
      ATH_MSG_DEBUG("Execute() All NTUPLES filled  Run: " << m_runNumber << "  event: " << m_evtNumber << "  mass: " << m_xZmm.GetInvMass() << " GeV ");
    }
  } // closing -> if ( m_xZmm.AcceptEvent() )

  if ( !m_xZmm.AcceptEvent() ) {
    // no good muon pair found
    //failed cuts, continue to next event
    ATH_MSG_DEBUG ("** IDPerfMonZmumu::execute ** No good muon pair found. Leaving Execute(). Run: " << m_runNumber << "  event: " << m_evtNumber);
    return StatusCode::SUCCESS;
  }

  //
  // fill truth event iformation even when the reco event has not passed
  //
  if (m_isMC) {
    bool truthStatusIsGood = true;
    if (FillTruthParameters(ppos_comb).isFailure()){
      truthStatusIsGood = false;
      ATH_MSG_WARNING("Failed to fill truth parameters - skipping event");
    }
    if (FillTruthParameters(pneg_comb).isFailure()){
      truthStatusIsGood = false;
      ATH_MSG_WARNING("Failed to fill truth parameters - skipping event");
    }
    if (truthStatusIsGood) { 
      ATH_MSG_DEBUG ("-- Filling " << m_truthTree->GetName() << " entry " << m_truthTree->GetEntries() 
		     << "  run: " << m_runNumber 
		     << "  event: " << m_evtNumber 
		     <<  "  Lumiblock: " << m_lumi_block 
		     << "  Invariant mass = " << m_xZmm.GetInvMass() << " GeV ");
      ATH_MSG_DEBUG ("Negative p: (" << m_negative_px << ", "
		     << m_negative_py << ", "
		     << m_negative_pz << ") "
		     << "  d0: " << m_negative_d0
		     << "  z0: " << m_negative_z0
		    << "  parent: " << m_negative_parent);
      ATH_MSG_DEBUG ("Positive p: (" << m_positive_px << ","
		     << m_positive_py << ","
		     << m_positive_pz << ") "
		     << "  d0: " << m_positive_d0
		     << "  z0: " << m_positive_z0
		     << "  parent: " << m_positive_parent);	  
      
      if (m_storeZmumuNtuple && m_truthTree) m_truthTree->Fill();
      m_Truth_pt.push_back(m_positive_pt);
      m_Truth_pt.push_back(m_negative_pt);
      m_Truth_eta.push_back(m_positive_eta);
      m_Truth_eta.push_back(m_negative_eta);
      m_Truth_phi.push_back(m_positive_phi);
      m_Truth_phi.push_back(m_negative_phi);
      m_Truth_d0.push_back(m_positive_d0);
      m_Truth_d0.push_back(m_negative_d0);
      m_Truth_z0.push_back(m_positive_z0);
      m_Truth_z0.push_back(m_negative_z0);
      m_Truth_qoverp.push_back(m_positive_qoverp);
      m_Truth_qoverp.push_back(m_negative_qoverp);
      m_Truth_parent.push_back(m_positive_parent);
      m_Truth_parent.push_back(m_negative_parent);
      
    } // truth info properly filled
    else {
      ATH_MSG_DEBUG("FAILED filling m_truthTree "
		    << "  for run: " << m_runNumber 
		    << "  event: " << m_evtNumber 
		    << "  Lumiblock: " << m_lumi_block);
    }
  } // if (m_isMC)

  // fill common tree
  if (m_commonTree ) {
    // fill ntuple if some of the collections is filled
    bool dofill = false;
    if (m_IDTrack_pt.size() >= 2) dofill = true;
    if (m_CBTrack_pt.size() >= 2) dofill = true;
    if (m_Refit1_pt.size() >= 2) dofill = true;
    if (m_Refit2_pt.size() >= 2) dofill = true;
    if (m_Truth_pt.size() >= 2) dofill = true;
    
    if (dofill) {
      ATH_MSG_DEBUG("-- Filling m_commonTree " << m_commonTree->GetName() << " entry " << m_commonTree->GetEntries() 
		    << "  run: " << m_runNumber 
		    << "  event: " << m_evtNumber 
		    << "  Lumiblock: " << m_lumi_block 
		    << "  Invariant mass = " << m_xZmm.GetInvMass() << " GeV ");
      m_commonTree->Fill();
    }
    else {
      ATH_MSG_DEBUG("-- Filling m_commonTree " << m_commonTree->GetName() << " FAILED for " 
		    << "  run: " << m_runNumber 
		    << "  event: " << m_evtNumber 
		    << "  Lumiblock: " << m_lumi_block); 
    }
  }

  ATH_MSG_DEBUG(" --IDPerfMonZmumu::execute--  event completed -- Run: " << m_runNumber << "  event: " << m_evtNumber);

  /* ***********************************************************
   * after all, let's make the collection for ALL MUON ID Tracks
   * ***********************************************************/
 
  ATH_MSG_DEBUG("IDTracks monitoring..");
  int nTracks = 0;
  TrackCollection* IDTracks = new TrackCollection(SG::OWN_ELEMENTS);

  // DIFFERENT RETRIEVAL
  const xAOD::MuonContainer* muons = PerfMonServices::getContainer<xAOD::MuonContainer>( PerfMonServices::MUON_COLLECTION );
  if(muons){   
    for (auto muon : *muons) {
      const xAOD::TrackParticle* tp = muon->trackParticle(xAOD::Muon::InnerDetectorTrackParticle);   
      if(!tp) {
	ATH_MSG_DEBUG(" - tpb = "<< tp << " -> is null? " << (!tp));
	continue;
      }
      const Trk::Track* t = tp->track();
      if ( !t ) {
	ATH_MSG_DEBUG(" - tpb does not have track ");
	continue;
      }
      const Trk::Perigee* tPerigee = t->perigeeParameters() ;
      if ( !tPerigee ) {
	ATH_MSG_DEBUG(" - t does not have perigree ");
	continue;    
      }
      IDTracks->push_back ( new Trk::Track (*t));
      nTracks++; 
    }
  }
  else  ATH_MSG_WARNING ("Could not retrieve Muon container");

  ATH_MSG_DEBUG(" --> nTracks = " << nTracks);

  StatusCode scid = evtStore()->record(IDTracks, "IDTracks");
  if (scid.isSuccess()) {
    ATH_MSG_DEBUG ("Stored "<< IDTracks->size() << " IDTracks into StoreGate");
  }
  else{
    ATH_MSG_WARNING ("Failed storing IDTracks");
  }

  /*********************************************************
   * ..finish
   ********************************************************/

  return StatusCode::SUCCESS;
}

//==================================================================================
StatusCode IDPerfMonZmumu::FillRecParametersTP(const xAOD::TrackParticle* trackp, 
					       const xAOD::TrackParticle* trackp_for_unbias, 
					       double charge, 
					       const xAOD::Vertex* vertex)
{
  if (!trackp){
    //ATH_MSG_WARNING("Empty Trackparticle. Skipping.");
    return StatusCode::FAILURE;
  }

  if (m_doRemoval && !trackp_for_unbias && !m_skipMS){
    return StatusCode::FAILURE;
  }
    
  double px = 0;
  double py = 0;
  double pt = 0;
  double pz = 0;
  double phi= 0;
  double eta= 0;
  double d0 = 0;
  double z0 = 0;
  double d0res = 0;
  double z0res = 0;
  double PVd0res = 0;
  double PVz0res = 0;
  double PVd0 = 0;
  double PVz0 = 0;
  double sigma_pt = 0;
  double qoverp = 0;
  double sigma_qOverP = 1.; // use a large value by default as 0 is meaningless


  px = trackp->p4().Px(); 
  py = trackp->p4().Py();
  pt = trackp->p4().Pt(); 
  pz = trackp->p4().Pz();
  phi= trackp->p4().Phi();
  eta= trackp->p4().Eta();
  d0 = trackp->d0();
  z0 = trackp->z0();
  qoverp = trackp->qOverP();

  d0res = std::sqrt(trackp->definingParametersCovMatrix()(Trk::d0,Trk::d0));
  z0res = std::sqrt(trackp->definingParametersCovMatrix()(Trk::z0,Trk::z0));
  sigma_qOverP = std::sqrt(trackp->definingParametersCovMatrix()(Trk::qOverP,Trk::qOverP));
  double sigma_theta  = std::sqrt(trackp->definingParametersCovMatrix()(Trk::theta,Trk::theta));
    
  // computing sigma_pt
  // pt = sin(theta) / qOverP --> sigma(pt) = (sin(theta)/ qOverP^2) x sigma(qOverP) // neglecting sigma(sin(theta))
  double sigma_pt_term1 = (sin(trackp->theta()) / pow(qoverp,2)) * sigma_qOverP;
  double sigma_pt_term2 = (1./qoverp) * cos(trackp->theta()) * sigma_theta;
  double sigma_pt_term3 = (cos(trackp->theta()) / pow(qoverp,2)) * sigma_theta * sigma_qOverP;
  sigma_pt = sqrt( pow(sigma_pt_term1,2) + pow(sigma_pt_term2,2) + 2 * sigma_pt_term3 * trackp->definingParametersCovMatrix()(Trk::qOverP,Trk::theta));

  //
  if (vertex == nullptr) {
    ATH_MSG_WARNING("in FillRecParametersTP. WARNING: Vertex is NULL");
    return StatusCode::FAILURE;
  }
  if (!m_trackToVertexIPEstimator) {
    ATH_MSG_WARNING("in FillRecParametersTP. WARNING: m_trackToVertexIPEstimator is NULL");
    return StatusCode::FAILURE;
  }

  if (m_doIP && vertex != nullptr && m_trackToVertexIPEstimator){
    std::unique_ptr<Trk::ImpactParametersAndSigma> iPandSigma(nullptr);
    ATH_MSG_DEBUG("-- using the trackToVertexIPEstimator --");
    
    //Calling the estimate(trackp,newtrackp,vertex,doRemoval)
    //The first track is used to unbias the vertex, the second to get the extrapolation
    if (!m_skipMS) iPandSigma = m_trackToVertexIPEstimator->estimate(trackp_for_unbias, trackp, vertex, m_doRemoval);
    else iPandSigma = m_trackToVertexIPEstimator->estimate(trackp, vertex);
    ATH_MSG_DEBUG("return from the trackToVertexIPEstimator->estimate()");
    
    if( !iPandSigma ){
      ATH_MSG_WARNING ("FillRecParametersTP::trackToVertexIPEstimator failed !");
      return StatusCode::FAILURE;
    }
    else{
      ATH_MSG_DEBUG("FillRecParametersTP::trackToVertexIPEstimator success !");
      PVd0 = iPandSigma->IPd0;
      PVd0res = iPandSigma->PVsigmad0;
      d0res = iPandSigma->sigmad0;
      PVz0 = iPandSigma->IPz0;
      PVz0res = iPandSigma->PVsigmaz0;
      z0res = iPandSigma->sigmaz0;
    }
  }

  if (charge == 1) {
    m_positive_px = px;
    m_positive_py = py;
    m_positive_pt = pt;
    m_positive_pz = pz;
    m_positive_phi= phi;
    m_positive_eta= eta;
    m_positive_z0 = z0;
    m_positive_z0_err = z0res;
    m_positive_d0 = d0;
    m_positive_d0_err = d0res;
    m_positive_sigma_pt = sigma_pt;
    m_positive_qoverp = qoverp;
    m_positive_sigma_qoverp = sigma_qOverP;
    if(m_doIP){
      m_positive_z0_PV = PVz0;
      m_positive_d0_PV = PVd0;
      m_positive_z0_PVerr = PVz0res;
      m_positive_d0_PVerr = PVd0res;
    }
    ATH_MSG_DEBUG("(Filled charge == 1 ) (reco)-> px : "<< px <<" py: "<<py <<" pt: "<<pt <<" pz: "<<pz 
		  <<"   d0: "<<m_positive_d0 << " d0res : "<< d0res << " PVd0res : "<< PVd0res <<" z0: "<< m_positive_z0 << " z0res : " << z0res <<  " PVz0res : "<< PVz0res );

  } else  if (charge == -1) {
    m_negative_px = px;
    m_negative_py = py;
    m_negative_pt = pt;
    m_negative_pz = pz;
    m_negative_phi= phi;
    m_negative_eta= eta;
    m_negative_z0 = z0;
    m_negative_z0_err = z0res;
    m_negative_d0 = d0;
    m_negative_d0_err = d0res;
    m_negative_sigma_pt = sigma_pt;
    m_negative_qoverp = qoverp;
    m_negative_sigma_qoverp = sigma_qOverP;
    if(m_doIP){
      m_negative_z0_PV = PVz0;
      m_negative_d0_PV = PVd0;
      m_negative_z0_PVerr = PVz0res;
      m_negative_d0_PVerr = PVd0res;
    }
    ATH_MSG_DEBUG("(Filled charge == -1 ) (reco)-> px : "<< px <<" py: "<< py <<" pt: " << pt <<" pz: "<<pz 
		  <<"   d0: "<<m_negative_d0 << " d0res : "<< d0res << " PVd0res : "<< PVd0res <<" z0: "<< m_negative_z0 << " z0res : " << z0res <<  " PVz0res : "<< PVz0res );
  }

  return StatusCode::SUCCESS;
}


//==================================================================================
StatusCode IDPerfMonZmumu::FillRecParameters (const Trk::Track* track, const xAOD::TrackParticle* trackp_for_unbias, double charge,const xAOD::Vertex* vertex, const EventContext& ctx) 
{
  if (!track){
    ATH_MSG_DEBUG("* FillRecParameters * Empty Track: track. Skipping.");
    return StatusCode::FAILURE;
  }

  if (m_doRemoval && !trackp_for_unbias && !m_skipMS) {
    ATH_MSG_DEBUG("* FillRecParameters * Empty Track: trackp_for_unbias. Skipping.");
    return StatusCode::FAILURE;
  }

  const Trk::Perigee* trkPerigee = track->perigeeParameters();
  const Trk::Perigee* trk_for_unbiasPerigee = nullptr;
  if (!m_skipMS)  trk_for_unbiasPerigee = &(trackp_for_unbias->perigeeParameters());

  double px = 0;
  double py = 0;
  double pt = 0;
  double pz = 0;
  double phi= 0;
  double eta= 0;
  double d0 = 0;
  double z0 = 0;
  
  double d0_err = 999.;
  double z0_err = 999.;
  
  double PVd0 = 0;
  double PVz0 = 0;
  double PVd0res = 0;
  double PVz0res = 0;
  

  if (trkPerigee != nullptr){
    double qOverP   = trkPerigee->parameters()[Trk::qOverP];
    if (qOverP) {
      px = trkPerigee->momentum().x();
      py = trkPerigee->momentum().y();
      pt = std::abs(trkPerigee->pT()); 
      pz = trkPerigee->momentum().z();
      phi= trkPerigee->parameters()[Trk::phi];
      eta= trkPerigee->eta();
      d0 = trkPerigee->parameters()[Trk::d0];
      z0 = trkPerigee->parameters()[Trk::z0];
      d0_err = Amg::error(*trkPerigee->covariance(),Trk::d0);
      z0_err = Amg::error(*trkPerigee->covariance(),Trk::z0);
    }
  }

  // access beam spot and extrapolate track till there
  SG::ReadCondHandle<InDet::BeamSpotData> beamSpotHandleRec { m_beamSpotKey, ctx };
  auto beamline = m_trackToVertexTool->GetBeamLine(beamSpotHandleRec.cptr());

  auto trackBLtemp = m_trackToVertexTool->trackAtBeamline(ctx, *track , beamline.get() );
  const Trk::AtaStraightLine*  atBL = dynamic_cast<const Trk::AtaStraightLine*>(trackBLtemp.get());

  if (atBL){
    double qOverP   = atBL->parameters()[Trk::qOverP];
    if(qOverP){
      px = atBL->momentum().x();
      py = atBL->momentum().y();
      pt = std::abs(atBL->pT());
      pz = atBL->momentum().z();
      eta= trkPerigee->eta();
      phi= trkPerigee->parameters()[Trk::phi];
      d0 = atBL->parameters()[Trk::d0];
      z0 = atBL->parameters()[Trk::z0];
      //      z0_err = Amg::error(*trkPerigee->covariance(),Trk::z0);  //->Why not?
      //      d0_err = Amg::error(*trkPerigee->covariance(),Trk::d0);  //->Why not?
    } 
    delete atBL;
  }
  else {
    ATH_MSG_WARNING("FillRecParameters::Failed extrapolation to the BeamLine");
  }

  Amg::Vector3D position = beamSpotHandleRec->beamPos();
  TLorentzVector vtrack = TLorentzVector (trkPerigee->momentum().x(),
					  trkPerigee->momentum().y(),
					  trkPerigee->momentum().z(),
					  trkPerigee->momentum().mag());
  float trkd0 = trkPerigee->parameters()[Trk::d0];
  float trkz0 = trkPerigee->parameters()[Trk::z0];
  float bsX = position.x();
  float bsY = position.y();
  float bsZ = position.z();
  float btiltX = beamSpotHandleRec->beamTilt(0);
  float btiltY = beamSpotHandleRec->beamTilt(1);
  // correct the track parameters for the beamspot position
  float beamX = bsX + std::tan(btiltX) * (trkz0-bsZ);
  float beamY = bsY + std::tan(btiltY) * (trkz0-bsZ);
  float beamD0 = ( -std::sin(vtrack.Phi())*beamX + std::cos(vtrack.Phi())*beamY );
  float d0bscorr = trkd0 - beamD0;
  float z0bscorr = trkz0 - bsZ;

  if(m_doIP && vertex){ //I assume that the vertex is the same of the original track
    std::unique_ptr<Trk::ImpactParametersAndSigma> iPandSigma(nullptr);
    if (!m_skipMS) iPandSigma = m_trackToVertexIPEstimator->estimate(trk_for_unbiasPerigee,trkPerigee,vertex,m_doRemoval);
    else iPandSigma = m_trackToVertexIPEstimator->estimate(trkPerigee,vertex);
    if( iPandSigma==0 ){
      ATH_MSG_WARNING ("FillRecParameters::trackToVertexIPEstimator failed !");
      return StatusCode::FAILURE;
    }
    else{
      ATH_MSG_DEBUG("FillRecParameters::trackToVertexIPEstimator success !");
      PVd0 = iPandSigma->IPd0;
      PVd0res = iPandSigma->PVsigmad0;
      //d0res = iPandSigma->sigmad0;  //-> ? 
      PVz0 = iPandSigma->IPz0;
      PVz0res = iPandSigma->PVsigmaz0;
      //z0res = iPandSigma->sigmaz0;  //-> ?
      
      if (vertex) {
	ATH_MSG_INFO(" FillRecParameters::trackToVertexIPEstimator vertex does exist ");
	m_pv_x = vertex->x();
	m_pv_y = vertex->y();
	m_pv_z = vertex->z();
	m_nTrkInVtx = vertex->nTrackParticles();
      }
      
    }
  }
    
  if (charge == 1) {
    m_positive_px = px;
    m_positive_py = py;
    m_positive_pt = pt;
    m_positive_pz = pz;
    m_positive_phi= phi;
    m_positive_eta= eta;
    m_positive_z0 = z0;
    m_positive_d0 = d0;
    m_positive_z0_manualBS = z0bscorr;
    m_positive_d0_manualBS = d0bscorr;
    m_positive_d0_err = d0_err;
    m_positive_z0_err = z0_err;
    if(m_doIP){
      m_positive_z0_PV    = PVz0;
      m_positive_d0_PV    = PVd0;
      m_positive_z0_PVerr = PVz0res;
      m_positive_d0_PVerr = PVd0res;
    }
    ATH_MSG_DEBUG("(Filled charge == 1 ) (reco)-> px : "<< px <<" py: "<<py <<" pz: "<<pz <<" d0: "<<d0<<" z0: "<< z0);
  }
  if (charge == -1) {
    m_negative_px = px;
    m_negative_py = py;
    m_negative_pt = pt;
    m_negative_pz = pz;
    m_negative_phi= phi;
    m_negative_eta= eta;
    m_negative_z0 = z0;
    m_negative_d0 = d0;
    m_negative_z0_manualBS = z0bscorr;
    m_negative_d0_manualBS = d0bscorr;
    m_negative_d0_err = d0_err;
    m_negative_z0_err = z0_err;
    if(m_doIP){
      m_negative_z0_PV = PVz0;
      m_negative_d0_PV = PVd0;
      m_negative_z0_PVerr = PVz0res;
      m_negative_d0_PVerr = PVd0res;
    }
    ATH_MSG_DEBUG("(Filled charge == -1 ) (reco)-> px : "<< px <<" py: "<<py <<" pz: "<<pz <<" d0: "<<d0<<" z0: "<<z0 );
  }

  return StatusCode::SUCCESS;
}
//==================================================================================
StatusCode IDPerfMonZmumu::FillRecParametersSimple (const Trk::Track* track, float charge, const xAOD::Vertex* vertex)
{
  if (!track){
    ATH_MSG_DEBUG("* FillRecParametersSimple * Empty Track: track. Skipping.");
    return StatusCode::FAILURE;
  }

  const Trk::Perigee* trkPerigee = track->perigeeParameters();
  if (not trkPerigee) {
    ATH_MSG_ERROR("trkPerigee pointer is null in IDPerfMonZmumu::FillRecParametersSimple");
    return StatusCode::FAILURE;
  }
  double px = 0;
  double py = 0;
  double pt = 0;
  double pz = 0;
  double phi= 0;
  double eta= 0;

  double d0 = 0;
  double z0 = 0;  
  double d0_err = 999.;
  double z0_err = 999.;
  
  //
  double qOverP   = trkPerigee->parameters()[Trk::qOverP];
  if (qOverP) {
    px = trkPerigee->momentum().x();
    py = trkPerigee->momentum().y();
    pt = std::abs(trkPerigee->pT()); 
    pz = trkPerigee->momentum().z();
    phi= trkPerigee->parameters()[Trk::phi];
    eta= trkPerigee->eta();
    d0 = trkPerigee->parameters()[Trk::d0];
    z0 = trkPerigee->parameters()[Trk::z0];
    d0_err = Amg::error(*trkPerigee->covariance(),Trk::d0);
    z0_err = Amg::error(*trkPerigee->covariance(),Trk::z0);
  }

  SG::ReadHandle<xAOD::EventInfo> eventInfo (m_EventInfoKey, getContext());
  Amg::Vector3D position (eventInfo->beamPosX(), eventInfo->beamPosY(), eventInfo->beamPosZ());
  TLorentzVector vtrack = TLorentzVector (trkPerigee->momentum().x(),
					  trkPerigee->momentum().y(),
					  trkPerigee->momentum().z(),
					  trkPerigee->momentum().mag());
  float trkd0 = trkPerigee->parameters()[Trk::d0];
  float trkz0 = trkPerigee->parameters()[Trk::z0];
  float bsX = position.x();
  float bsY = position.y();
  float bsZ = position.z();
  float btiltX = eventInfo->beamTiltXZ();
  float btiltY = eventInfo->beamTiltYZ();
  // correct the track parameters for the beamspot position
  float beamX = bsX + std::tan(btiltX) * (trkz0-bsZ);
  float beamY = bsY + std::tan(btiltY) * (trkz0-bsZ);
  float beamD0 = ( -std::sin(vtrack.Phi())*beamX + std::cos(vtrack.Phi())*beamY );
  float d0bscorr = trkd0 - beamD0;
  float z0bscorr = trkz0 - bsZ - vertex->z();

  if (charge == 1) {
    m_positive_px = px;
    m_positive_py = py;
    m_positive_pt = pt;
    m_positive_pz = pz;
    m_positive_phi= phi;
    m_positive_eta= eta;
    m_positive_z0 = z0bscorr;
    m_positive_d0 = d0bscorr;
    m_positive_z0_manualBS = z0bscorr;
    m_positive_d0_manualBS = d0bscorr;
    m_positive_d0_err = d0_err;
    m_positive_z0_err = z0_err;
  }
  if (charge == -1) {
    m_negative_px = px;
    m_negative_py = py;
    m_negative_pt = pt;
    m_negative_pz = pz;
    m_negative_phi= phi;
    m_negative_eta= eta;
    m_negative_z0 = z0bscorr;
    m_negative_d0 = d0bscorr;
    m_negative_z0_manualBS = z0bscorr;
    m_negative_d0_manualBS = d0bscorr;
    m_negative_d0_err = d0_err;
    m_negative_z0_err = z0_err;
  }

  ATH_MSG_DEBUG("-- FillRecParametersSimple -- charge " << charge << "  pt: " << pt << "  d0: " << d0 << "  z0: " << z0);

  return StatusCode::SUCCESS;
}

//==================================================================================
StatusCode IDPerfMonZmumu::FillTruthParameters(const xAOD::TrackParticle* trackParticle)
{

  if (!trackParticle ){//|| !trackParticle->vertex()){
    ATH_MSG_WARNING("-- FillTruthParameters -- Empty Trackparticle. Skipping.");
    return StatusCode::FAILURE;
  }

  float result(std::numeric_limits<float>::quiet_NaN());
  if (trackParticle->isAvailable<float>("truthMatchProbability")){
    result = trackParticle->auxdata<float>("truthMatchProbability" );
    ATH_MSG_DEBUG(" -- FillTruthParameters -- TruthMatchProbablity of trackpartile : > " << result); 
  }


  const xAOD::TruthParticle* particle = getTruthParticle( *trackParticle );
  double charge = 0;
  if (!particle) {
    ATH_MSG_DEBUG(" -- FillTruthParameters -- Failure while retrieving the truth particle. Returning FAILURE.");
    return StatusCode::FAILURE;
  }

  if ( particle->pdgId() ==  13) charge = -1.; // muon-
  if ( particle->pdgId() == -13) charge = 1.; // muon+
  if ( particle->isNeutral() ) {
    ATH_MSG_DEBUG(" -- FillTruthParameters -- reco muon associated to a truth neutral!! Returning FAILURE.");
    return StatusCode::FAILURE;
  }
  if ( charge == 0 ) {
    ATH_MSG_DEBUG(" -- FillTruthParameters -- reco muon associated to a non true muon!! Returning FAILURE.");
    return StatusCode::FAILURE;
  } 

  const Amg::Vector3D momentum(particle->px(), particle->py(), particle->pz());  
  const xAOD::TruthVertex * ptruthVertex(0);
  ptruthVertex=particle->prodVtx();
  if (!ptruthVertex){
    ATH_MSG_DEBUG("A production vertex pointer was retrieved, but it is NULL");
    return StatusCode::FAILURE;
  }


  if (particle->parent(0) != nullptr) {
    if (particle->isAvailable<int>("truthType")) {
      ATH_MSG_DEBUG(" -- FillTruthParameters -- truth particle is good. pdg: " << particle->pdgId() 
		    << "  type: " << particle->type()
		    << "  nparents " << particle->nParents()
		    << "  parent->pdg: " << particle->parent(0)->pdgId()
		    << "  truthType: " << particle->auxdata<int>("truthType")
		    << "  truthOrigin: " << const_cast<xAOD::TruthParticle*>(particle)->auxdata<int>("truthOrigin")
		    );

    }
    else {
      ATH_MSG_DEBUG(" -- FillTruthParameters -- truth particle is good. pdg: " << particle->pdgId() 
		    << "  type: " << particle->type()
		    << "  nparents " << particle->nParents()
		    << "  parent->pdg: " << particle->parent(0)->pdgId()
		    << "  truthType & truthOrigin: " << " NOT AVAILABLE " 
		    );
    }
  }

  const auto xPos=ptruthVertex->x();
  const auto yPos=ptruthVertex->y();
  const auto z_truth=ptruthVertex->z();

  const Amg::Vector3D position(xPos, yPos, z_truth);
  const Trk::CurvilinearParameters cParameters(position, momentum, charge);

  SG::ReadCondHandle<InDet::BeamSpotData> beamSpotHandleTruth { m_beamSpotKey };
  Trk::PerigeeSurface persf( beamSpotHandleTruth->beamPos() );

  const EventContext& ctx = Gaudi::Hive::currentContext();
  std::unique_ptr<const Trk::TrackParameters> tP = m_extrapolator->extrapolate(ctx, cParameters,persf, Trk::anyDirection, false);

  double px = 0;
  double py = 0;
  double pt = 0;
  double pz = 0;
  double phi= 0;
  double eta= 0;
  double d0 = 0;
  double d0res = 0;
  double PVd0res = 0;
  double z0 = 0;
  double z0res = 0;
  double PVz0res = 0;

  double d0recoPos =  m_positive_d0;
  double z0recoPos =  m_positive_z0;
  double d0recoNeg =  m_negative_d0;
  double z0recoNeg =  m_negative_z0;

  ATH_MSG_DEBUG("reco IPs (pos): > d0 : "<<d0recoPos << " z0: " << z0recoPos << " trackp z0 : " << trackParticle->z0() << " trackp d0 : " << trackParticle->d0());
  ATH_MSG_DEBUG("reco IPs (neg): > d0 : "<<d0recoNeg << " z0: " << z0recoNeg << " trackp z0 : " << trackParticle->z0() <<" trackp d0 : " << trackParticle->d0() );

  double qOverP_truth = 0.;
  if (tP){
    qOverP_truth = tP->parameters()[Trk::qOverP];
    if( qOverP_truth ){

      px = tP->momentum().x();
      py = tP->momentum().y();
      pt = tP->pT();
      pz = tP->momentum().z();
      phi= tP->parameters()[Trk::phi];
      eta= tP->eta();
      d0 = tP->parameters()[Trk::d0];
      z0 = tP->parameters()[Trk::z0];

      ATH_MSG_DEBUG("cand perig HEP particle (truth) px : "<< tP->momentum().x());
      ATH_MSG_DEBUG("cand perig HEP particle (truth) py : "<< tP->momentum().y());
      ATH_MSG_DEBUG("cand perig HEP particle (truth) pz : "<< tP->momentum().z());
      ATH_MSG_DEBUG("cand perig HEP particle (truth) d0 : "<< tP->parameters()[Trk::d0]);
      ATH_MSG_DEBUG("cand perig HEP particle (truth) z0 : "<< tP->parameters()[Trk::z0]);
      
    }
  }


  if (charge == 1) {
    m_positive_px = px;
    m_positive_py = py;
    m_positive_pt = pt;
    m_positive_pz = pz;
    m_positive_phi= phi;
    m_positive_eta= eta;
    m_positive_z0 = z0recoPos -z0;
    m_positive_z0_err = z0res;
    m_positive_z0_PVerr = PVz0res;
    m_positive_d0 = d0recoPos -d0;
    m_positive_d0_err = d0res;
    m_positive_d0_PVerr = PVd0res;
    m_positive_qoverp = qOverP_truth;

    bool parentfound = false;
    if (particle->nParents()>0) {
      if (particle->parent(0) != nullptr) {
	m_positive_parent = particle->parent(0)->pdgId();
	parentfound = true;
      } 
    }
    if (!parentfound) m_positive_parent = 0;

    ATH_MSG_DEBUG(" -- FillTruthParameters -- charge = 1  (truth)-> px : "<< m_positive_px 
		  <<"  py: "<<m_positive_py 
		  <<"  pz: "<<m_positive_pz 
		  <<"  d0: "<<m_positive_d0 
		  <<"  z0: "<< m_positive_z0
		  <<"  qoverp: " << m_positive_qoverp
		 << "  parent: " << m_positive_parent);

  } else  if (charge == -1) {
    m_negative_px = px;
    m_negative_py = py;
    m_negative_pt = pt;
    m_negative_pz = pz;
    m_negative_phi= phi;
    m_negative_eta= eta;
    m_negative_z0 = z0recoNeg-z0;
    m_negative_z0_err = z0res;
    m_negative_z0_PVerr = PVz0res;
    m_negative_d0 = d0recoNeg-d0;
    m_negative_d0_err = d0res;
    m_negative_d0_PVerr = PVd0res;
    m_negative_qoverp = qOverP_truth;

    bool parentfound = false;
    if (particle->nParents()>0) {
      if (particle->parent(0) != nullptr) {
	m_negative_parent = particle->parent(0)->pdgId();
	parentfound = true;
      }
    }
    if (!parentfound) m_negative_parent = 0;

    ATH_MSG_DEBUG(" -- FillTruthParameters-- charge = -1 (truth)-> px : "<< m_negative_px 
		  << "  py: "<<m_negative_py 
		  << "  pz: "<<m_negative_pz 
		  << "  d0: "<<m_negative_d0 
		  << "  z0: "<< m_negative_z0
		  <<"  qoverp: " << m_negative_qoverp
		  << "  parent:" << m_negative_parent);
  }
  return StatusCode::SUCCESS;
}


//==================================================================================
const xAOD::TruthParticle* IDPerfMonZmumu::getTruthParticle( const xAOD::IParticle& p ) {
  /// A convenience type declaration
  typedef ElementLink< xAOD::TruthParticleContainer > Link_t;
  /// A static accessor for the information
  static const SG::AuxElement::ConstAccessor< Link_t > acc( "truthParticleLink" );
  // Check if such a link exists on the object:
  if( ! acc.isAvailable( p ) ) {
    return nullptr;
  }
  // Get the link:
  const Link_t& link = acc( p );
  // Check if the link is valid:
  if( ! link.isValid() ) {
    return nullptr;
  }
  // Everything has passed, let's return the pointer:
  return *link;
}

//==================================================================================
StatusCode IDPerfMonZmumu::finalize()
{
  m_xZmm.finalize();
  return StatusCode::SUCCESS;
}

//==================================================================================
StatusCode IDPerfMonZmumu::CheckTriggerStatusAndPrescale ()
{
  m_triggerPrescale = 1; // default value

  // check trigger status
  if(m_triggerDecision.retrieve().isFailure()) {
    ATH_MSG_FATAL("Unable to retrieve " << m_triggerDecision << " turn it off");
    return StatusCode::FAILURE;
  }
  else {
    ATH_MSG_DEBUG("retrieved tool: " << m_triggerDecision );
  }

  float thisEventTriggerPrescale = 999999.9;
  float thisHLTTriggerPrescale = 999999.9;
  bool thisHLTIsPassed = false;
  std::string thisHLTTriggerName;
  std::string thisEventTriggerName;
  
  // HLT triggers
  const std::vector<std::string> myHLTtriglist = m_triggerDecision->getChainGroup("HLT_mu.*, HLT_2mu.*")->getListOfTriggers();
  for (int i=0; i < (int) myHLTtriglist.size(); i++) {
    if ( m_triggerDecision->isPassed(myHLTtriglist.at(i)) ) {
      ATH_MSG_DEBUG("HLT trigger = " << i << " out of " << myHLTtriglist.size() << " --> " << myHLTtriglist.at(i) <<  "  isPassed? " << m_triggerDecision->isPassed(myHLTtriglist.at(i)) << "  prescale: " << m_triggerDecision->getChainGroup(myHLTtriglist.at(i))->getPrescale() );
    }
    if ( m_triggerDecision->isPassed(myHLTtriglist.at(i)) ) { // trigger is passed
      thisHLTIsPassed = true;
      float thisprescale = m_triggerDecision->getChainGroup(myHLTtriglist.at(i))->getPrescale();
      if (thisprescale < thisHLTTriggerPrescale) {
	thisHLTTriggerPrescale = thisprescale;
	thisHLTTriggerName = myHLTtriglist.at(i);
      }
    }
  }
  if (thisHLTTriggerPrescale < 1) thisHLTTriggerPrescale = 1;
  ATH_MSG_DEBUG("Event HLT trigger prescale = " << thisHLTTriggerPrescale);

  //if (thisL1IsPassed && thisHLTIsPassed) {
  if (thisHLTIsPassed) {
    ATH_MSG_DEBUG("Trigger passed -> accept event");
  }
  else{
    ATH_MSG_DEBUG("Trigger Failed -> reject event");
    return StatusCode::FAILURE;
  }

  // event prescale
  thisEventTriggerPrescale = thisHLTTriggerPrescale;
  thisEventTriggerName = thisHLTTriggerName;
  ATH_MSG_DEBUG("Event trigger prescale = " << thisEventTriggerPrescale);
  ATH_MSG_DEBUG("Event trigger name = " << thisEventTriggerName);
  
  m_triggerPrescale = thisEventTriggerPrescale;
  m_triggerName = thisEventTriggerName;

  return StatusCode::SUCCESS;
}


//==================================================================================
const xAOD::Vertex* IDPerfMonZmumu::GetDiMuonVertex(const xAOD::TrackParticle* muon1, const xAOD::TrackParticle* muon2) {
  
  const xAOD::VertexContainer* vxContainer(0);
  const xAOD::Vertex* myVtx(0);
  vxContainer = PerfMonServices::getContainer<xAOD::VertexContainer> (PerfMonServices::VTX_COLLECTION);
  if (!vxContainer) { 
    return myVtx;
  }
  
  for (int ivtx=0; ivtx< (int) vxContainer->size();ivtx++)  {
    myVtx = (*vxContainer)[ivtx];
    if ((myVtx->vertexType() == xAOD::VxType::PriVtx)) {
      if (m_Trk2VtxAssociationTool->isCompatible(*muon1,*myVtx) && (m_Trk2VtxAssociationTool->isCompatible(*muon2,*myVtx)))
	return myVtx;
    }
  }
  return myVtx;
}

//==================================================================================
void IDPerfMonZmumu::ResetCommonNtupleVectors()
{
  m_IDTrack_pt.clear();
  m_CBTrack_pt.clear();
  m_Refit1_pt.clear();
  m_Refit2_pt.clear();
  m_Truth_pt.clear();

  m_IDTrack_eta.clear();
  m_CBTrack_eta.clear();
  m_Refit1_eta.clear();
  m_Refit2_eta.clear();
  m_Truth_eta.clear();

  m_IDTrack_phi.clear();
  m_CBTrack_phi.clear();
  m_Refit1_phi.clear();
  m_Refit2_phi.clear();
  m_Truth_phi.clear();

  m_IDTrack_d0.clear();
  m_CBTrack_d0.clear();
  m_Refit1_d0.clear();
  m_Refit2_d0.clear();
  m_Truth_d0.clear();

  m_IDTrack_z0.clear();
  m_CBTrack_z0.clear();
  m_Refit1_z0.clear();
  m_Refit2_z0.clear();
  m_Truth_z0.clear();

  m_IDTrack_qoverp.clear();
  m_CBTrack_qoverp.clear();
  m_Refit1_qoverp.clear();
  m_Refit2_qoverp.clear();
  m_Truth_qoverp.clear();

  m_Truth_parent.clear();

  m_IDTrack_sigma_pt.clear();
  m_CBTrack_sigma_pt.clear();
  m_Refit1_sigma_pt.clear();
  m_Refit2_sigma_pt.clear();

  m_IDTrack_sigma_d0.clear();
  m_CBTrack_sigma_d0.clear();
  m_Refit1_sigma_d0.clear();
  m_Refit2_sigma_d0.clear();

  m_IDTrack_sigma_z0.clear();
  m_CBTrack_sigma_z0.clear();
  m_Refit1_sigma_z0.clear();
  m_Refit2_sigma_z0.clear();

  m_IDTrack_sigma_qoverp.clear();
  m_CBTrack_sigma_qoverp.clear();
  m_Refit1_sigma_qoverp.clear();
  m_Refit2_sigma_qoverp.clear();

  m_nBLhits.clear();
  m_nPIXhits.clear();
  m_nSCThits.clear();
  m_nTRThits.clear();

  return;
}

//==================================================================================
void IDPerfMonZmumu::Clear4MuNtupleVariables() 
{
  m_positive_px = 0.;
  m_positive_py = 0.;
  m_positive_pz = 0.;
  m_positive_d0 = 0.;
  m_positive_z0 = 0.;
  m_positive_d0_manualBS = 0.;
  m_positive_z0_manualBS = 0.;
  m_positive_d0_err = 0.;
  m_positive_z0_err = 0.;
  m_positive_sigma_pt = 0.;
  m_positive_1_vtx = 0;
  m_positive_parent = 0;

  m_negative_px = 0.;
  m_negative_py = 0.;
  m_negative_pz = 0.;
  m_negative_d0 = 0.;
  m_negative_z0 = 0.;
  m_negative_d0_manualBS = 0.;
  m_negative_z0_manualBS = 0.;
  m_negative_d0_err = 0.;
  m_negative_z0_err = 0.;
  m_negative_sigma_pt = 0.;
  m_negative_1_vtx = 0;
  m_negative_parent = 0;
	
  m_positive_2_px = 0.;
  m_positive_2_py = 0.;
  m_positive_2_pz = 0.;
  m_positive_2_d0 = 0.;
  m_positive_2_z0 = 0.;
  m_positive_2_d0_err = 0.;
  m_positive_2_z0_err = 0.;
  m_positive_2_vtx = 0;
	
  m_negative_2_px = 0.;
  m_negative_2_py = 0.;
  m_negative_2_pz = 0.;
  m_negative_2_d0 = 0.;
  m_negative_2_z0 = 0.;
  m_negative_2_d0_err = 0.;
  m_negative_2_z0_err = 0.;
  m_negative_2_vtx = 0;

  // electrons
  m_el_negative1_px = 0.;
  m_el_negative1_py = 0.;
  m_el_negative1_pz = 0.;
  m_el_negative1_d0 = 0.;
  m_el_negative1_z0 = 0.;
  m_el_negative1_d0_err = 0.;
  m_el_negative1_z0_err = 0.;
  m_el_negative1_vtx = 0;

  m_el_negative2_px = 0.;
  m_el_negative2_py = 0.;
  m_el_negative2_pz = 0.;
  m_el_negative2_d0 = 0.;
  m_el_negative2_z0 = 0.;
  m_el_negative2_d0_err = 0.;
  m_el_negative2_z0_err = 0.;
  m_el_negative2_vtx = 0;

  m_el_positive1_px = 0.;
  m_el_positive1_py = 0.;
  m_el_positive1_pz = 0.;
  m_el_positive1_d0 = 0.;
  m_el_positive1_z0 = 0.;
  m_el_positive1_d0_err = 0.;
  m_el_positive1_z0_err = 0.;
  m_el_positive1_vtx = 0;
  
  m_el_positive2_px = 0.;
  m_el_positive2_py = 0.;
  m_el_positive2_pz = 0.;
  m_el_positive2_d0 = 0.;
  m_el_positive2_z0 = 0.;
  m_el_positive2_d0_err = 0.;
  m_el_positive2_z0_err = 0.;
  m_el_positive2_vtx = 0;
  
  // other quantities
  m_nVertex =        0;
  m_pv_x = 0; m_pv_y = 0; m_pv_z = 0;
  m_4mu_minv = 0.;

  return;
}
//==================================================================================
StatusCode IDPerfMonZmumu::RunFourLeptonAnalysis()
{
  StatusCode thisStatus = StatusCode::SUCCESS;

  if (m_doFourMuAnalysis) {
    ATH_MSG_DEBUG ("** RunFourLeptonAnalysis ** START **    run: " << m_runNumber << "    event: " << m_evtNumber << "    lumiblock:" << m_lumi_block);  

    // Four lepton event
    m_4mu.setContainer(PerfMonServices::MUON_COLLECTION);
    m_4mu.doIsoSelection     (m_doIsoSelection);
    m_4mu.doIPSelection      (m_doIPSelection);
    m_4mu.doMCPSelection     (m_doMCPSelection);
    m_4mu.SetMassWindowLow   (m_MassWindowLow);
    m_4mu.SetMassWindowHigh  (m_MassWindowHigh);
    m_4mu.SetLeadingMuonPtCut(m_LeadingMuonPtCut);
    m_4mu.SetSecondMuonPtCut (m_SecondMuonPtCut);
    m_4mu.SetOpeningAngleCut (m_OpeningAngleCut);
    m_4mu.SetZ0GapCut        (m_Z0GapCut);
    m_4mu.setDebugMode       (m_doDebug);

    // This is a report of the number of true electrons in the event
    if (false) {
      const xAOD::TruthParticleContainer* electronTruth = evtStore()->retrieve< const xAOD::TruthParticleContainer >( "egammaTruthParticles" );
      if( electronTruth) {
	ATH_MSG_DEBUG ( "retrieving electron Truth container with key: \"egammaTruthParticles\" SUCCESS ");
	// Check that the auxiliary store association was made successfully:
	if( electronTruth->hasStore() ) {
	  ATH_MSG_DEBUG ( "Size of Truth gamma-electrons: " << electronTruth->size() );
	  auto tr_itr = electronTruth->begin();
	  auto tr_end = electronTruth->end();
	  int electronCount = 0;
	  for( int i = 0; tr_itr != tr_end; ++tr_itr, ++i ) {
	    ATH_MSG_DEBUG( "Truth : Investigating truth electron #" << i << "    pdgID= " << ( *tr_itr )->pdgId());
	    if (fabs(( *tr_itr )->pdgId()) == 11) {
	      if (( *tr_itr )->pt() > 10000 && fabs(( *tr_itr )->eta()) < 2.47) {
		ATH_MSG_INFO( "        central electron found --> pt: " << ( *tr_itr )->charge()*( *tr_itr )->pt() << " eta  " <<( *tr_itr )->eta()  );
		electronCount++;
	      }
	    }
	  }
	  ATH_MSG_INFO( "#central electrons: " << electronCount );
	}
	else {
	  ATH_MSG_WARNING ( "electronTruth container has no Store !!! FAILURE");
	}
      }
      else {
	ATH_MSG_WARNING ( "retrieve electron Truth container with key: \"egammaTruthParticles\" FAILURE");
      }
    }

    if (m_doDebug) std::cout << " ** RunFourLeptonAnalysis ** calling m_4mu.Reco() .... " << std::endl;  
    bool statusOf4LeptReco = m_4mu.Reco();

    // print message 
    if(statusOf4LeptReco) {
      thisStatus = StatusCode::SUCCESS;
      ATH_MSG_INFO ("4-lepton reconstruction SUCCESS. # accepted events " << m_4mu.getAcceptedEvents() << "  Invariant mass = " << m_4mu.GetInvMass() << " GeV ");
      if (m_doDebug) std::cout << " ** RunFourLeptonAnalysis ** 4-lepton reconstruction SUCCESS. # accepted events " << m_4mu.getAcceptedEvents() << "  Invariant mass = " << m_4mu.GetInvMass() << " GeV " << std::endl;
    }
    else {
      thisStatus = StatusCode::FAILURE;
      ATH_MSG_DEBUG ("4-lepton reconstruction FAILURE.  m_4mu.Reco() returned FALSE --> event failed selection");
      if (m_doDebug) std::cout << " ** RunFourLeptonAnalysis ** 4-lepton reconstruction FAILURE.  m_4mu.Reco() returned FALSE --> event failed selection" << std::endl;
    }

    if(statusOf4LeptReco) {
      if ( m_4mu.EventPassed() ) {

	this->Clear4MuNtupleVariables(); 

	const xAOD::TrackParticle* muon1_pos = m_4mu.getIDTrack(m_4mu.getPosMuon(1));
	const xAOD::TrackParticle* muon2_pos = m_4mu.getIDTrack(m_4mu.getPosMuon(2));
	const xAOD::TrackParticle* muon1_neg = m_4mu.getIDTrack(m_4mu.getNegMuon(1));
	const xAOD::TrackParticle* muon2_neg = m_4mu.getIDTrack(m_4mu.getNegMuon(2));

	// muons
	if (muon1_pos) {
	  m_positive_px = muon1_pos->p4().Px();
	  m_positive_py = muon1_pos->p4().Py();
	  m_positive_pz = muon1_pos->p4().Pz();
	  m_positive_d0 = muon1_pos->d0();
	  m_positive_z0 = muon1_pos->z0();
	  // m_positive_d0_err = muon1_pos->definingParametersCovMatrix()(0,0);
	  m_positive_d0_err = muon1_pos->definingParametersCovMatrixVec()[0];
	  m_positive_z0_err = muon1_pos->definingParametersCovMatrix()(1,1);
	}
	if (muon1_neg) {
	  m_negative_px = muon1_neg->p4().Px();
	  m_negative_py = muon1_neg->p4().Py();
	  m_negative_pz = muon1_neg->p4().Pz();
	  m_negative_d0 = muon1_neg->d0();
	  m_negative_z0 = muon1_neg->z0();
	  m_negative_d0_err = muon1_neg->definingParametersCovMatrix()(0,0);
	  m_negative_z0_err = muon1_neg->definingParametersCovMatrix()(1,1);
	}
	if (muon2_pos) {
	  m_positive_2_px = muon2_pos->p4().Px();
	  m_positive_2_py = muon2_pos->p4().Py();
	  m_positive_2_pz = muon2_pos->p4().Pz();
	  m_positive_2_d0 = muon2_pos->d0();
	  m_positive_2_z0 = muon2_pos->z0();
	  m_positive_2_d0_err = muon2_pos->definingParametersCovMatrix()(0,0);
	  m_positive_2_z0_err = muon2_pos->definingParametersCovMatrix()(1,1);
	}
	if (muon2_neg) {
	  m_negative_2_px = muon2_neg->p4().Px();
	  m_negative_2_py = muon2_neg->p4().Py();
	  m_negative_2_pz = muon2_neg->p4().Pz();
	  m_negative_2_d0 = muon2_neg->d0();
	  m_negative_2_z0 = muon2_neg->z0();
	  m_negative_2_d0_err = muon2_neg->definingParametersCovMatrix()(0,0);
	  m_negative_2_z0_err = muon2_neg->definingParametersCovMatrix()(1,1);
	}
	
	// electrons
	const xAOD::TrackParticle* elec1_neg = m_4mu.getELTrack(0);
	const xAOD::TrackParticle* elec2_neg = m_4mu.getELTrack(1);
	const xAOD::TrackParticle* elec1_pos = m_4mu.getELTrack(2);
	const xAOD::TrackParticle* elec2_pos = m_4mu.getELTrack(3);
	if (elec1_neg) {
	  m_el_negative1_px = elec1_neg->p4().Px();
	  m_el_negative1_py = elec1_neg->p4().Py();
	  m_el_negative1_pz = elec1_neg->p4().Pz();
	  m_el_negative1_d0 = elec1_neg->d0();
	  m_el_negative1_z0 = elec1_neg->z0();
	  m_el_negative1_d0_err = elec1_neg->definingParametersCovMatrix()(0,0);
	  m_el_negative1_z0_err = elec1_neg->definingParametersCovMatrix()(1,1);
	}
	if (elec2_neg) {
	  m_el_negative2_px = elec2_neg->p4().Px();
	  m_el_negative2_py = elec2_neg->p4().Py();
	  m_el_negative2_pz = elec2_neg->p4().Pz();
	  m_el_negative2_d0 = elec2_neg->d0();
	  m_el_negative2_z0 = elec2_neg->z0();
	  m_el_negative2_d0_err = elec2_neg->definingParametersCovMatrix()(0,0);
	  m_el_negative2_z0_err = elec2_neg->definingParametersCovMatrix()(1,1);
	}
	if (elec1_pos) {
	  m_el_positive1_px = elec1_pos->p4().Px();
	  m_el_positive1_py = elec1_pos->p4().Py();
	  m_el_positive1_pz = elec1_pos->p4().Pz();
	  m_el_positive1_d0 = elec1_pos->d0();
	  m_el_positive1_z0 = elec1_pos->z0();
	  m_el_positive1_d0_err = elec1_pos->definingParametersCovMatrix()(0,0);
	  m_el_positive1_z0_err = elec1_pos->definingParametersCovMatrix()(1,1);
	}
	if (elec2_pos) {
	  m_el_positive2_px = elec2_pos->p4().Px();
	  m_el_positive2_py = elec2_pos->p4().Py();
	  m_el_positive2_pz = elec2_pos->p4().Pz();
	  m_el_positive2_d0 = elec2_pos->d0();
	  m_el_positive2_z0 = elec2_pos->z0();
	  m_el_positive2_d0_err = elec2_pos->definingParametersCovMatrix()(0,0);
	  m_el_positive2_z0_err = elec2_pos->definingParametersCovMatrix()(1,1);
	}
	
	// other quantities
	m_nVertex =        m_4mu.GetNVertex ();
	m_negative_1_vtx = m_4mu.GetVertexMuNeg1();
	m_negative_2_vtx = m_4mu.GetVertexMuNeg2();
	m_positive_1_vtx = m_4mu.GetVertexMuPos1();
	m_positive_2_vtx = m_4mu.GetVertexMuPos2();

	m_el_negative1_vtx = m_4mu.GetVertexElec(0);
	m_el_negative2_vtx = m_4mu.GetVertexElec(1);
	m_el_positive1_vtx = m_4mu.GetVertexElec(2);
	m_el_positive2_vtx = m_4mu.GetVertexElec(3);

	m_pv_x = 0; m_pv_y = 0; m_pv_z = 0;

	m_4mu_minv = m_4mu.GetInvMass();
	
	// Obtain MET 
	std::string metName = "MET_Reference_AntiKt4LCTopo";
	std::string metRefFinalName = "FinalClus";
	const xAOD::MissingETContainer* final_met = nullptr;

	m_met = -1; // default value
	m_metphi = -1;

	if (!evtStore()->contains<xAOD::MissingETContainer>(metName)) {
	  ATH_MSG_WARNING ( "No Collection with name " << metName << " found in StoreGate");
	  // return StatusCode::SUCCESS;
	}
	else {
	  StatusCode sc = evtStore()->retrieve(final_met, metName);
	  if (sc.isFailure()) {
	    ATH_MSG_DEBUG ( "Could not retrieve Collection " << metName << " from StoreGate");
	    // return StatusCode::SUCCESS;
	  }
	}
	const xAOD::MissingET *met = nullptr;
	if (final_met) met = (*final_met)[metRefFinalName];
	if (met) { // load MET values
	  m_met = met->met();
	  m_metphi = met->phi();
	}
	ATH_MSG_DEBUG (" Zmumu event with MET = " << m_met);
	
	ATH_MSG_INFO (" -- IDPerfMonZmumu::execute -- Accepted event " << m_4mu.getAcceptedEvents() << " with m_4mu.GetInvMass= " << m_4mu_minv);
	ATH_MSG_DEBUG ("**** Filling m_FourMuTree ntuple " << m_FourMuTree->GetName() << " entry " << m_FourMuTree->GetEntries()
		       << " for run: " << m_runNumber 
		       << "  event: " << m_evtNumber 
		       << "  Lumiblock: " << m_lumi_block 
		       << "  Invariant mass = " << m_4mu_minv << " GeV ");
	if (muon1_pos != nullptr) ATH_MSG_DEBUG("mu1+  --> pxyz " << muon1_pos->p4().Px() 
						<< ", " << muon1_pos->p4().Py()
						<< ", " << muon1_pos->p4().Pz()
						<< "  pt: " << muon1_pos->pt()
						<< "  d0: " << muon1_pos->d0() 
						<< "  z0: " << muon1_pos->z0()
						<< "  d0unc: " << muon1_pos->definingParametersCovMatrixVec()[0]
						);
	if (muon1_neg != nullptr) ATH_MSG_DEBUG("mu1-  --> pxyz " << muon1_neg->p4().Px() 
						<< ", " << muon1_neg->p4().Py()
						<< ", " << muon1_neg->p4().Pz()
						<< "  pt: " << muon1_neg->pt()
						<< "  d0: " << muon1_neg->d0() 
						<< "  z0: " << muon1_neg->z0()
						<< "  d0unc: " << muon1_neg->definingParametersCovMatrixVec()[0]
						);
	if (elec1_pos != nullptr) ATH_MSG_DEBUG("el1+  --> pxyz " << elec1_pos->p4().Px() 
						<< ", " << elec1_pos->p4().Py()
						<< ", " << elec1_pos->p4().Pz()
						<< "  pt: " << elec1_pos->pt()
						<< "  d0: " << elec1_pos->d0() 
						<< "  z0: " << elec1_pos->z0()
						<< "  d0unc: " << elec1_pos->definingParametersCovMatrixVec()[0]
						);
	  // ntuple variables have been filled in FillRecParametersTP
	m_FourMuTree->Fill();
      }
    } // succesful 4mu reco

  } // end of fourMuon Analysis

  return thisStatus;
}

//==================================================================================
void IDPerfMonZmumu::ExtractIDHitsInformation(const xAOD::Muon* muon_pos, const xAOD::Muon* muon_neg)
{
  // hits info
  // positive Muon
  const xAOD::TrackParticle* IDTrkMuPos = muon_pos->trackParticle(xAOD::Muon::InnerDetectorTrackParticle);
  const xAOD::TrackParticle* IDTrkMuNeg = muon_neg->trackParticle(xAOD::Muon::InnerDetectorTrackParticle);

  int nBLhits, nhitsPIX, nhitsSCT, nhitsTRT, nContribPixLayers;
  int nPIXholes, nSCTholes;
  if (IDTrkMuPos && IDTrkMuNeg) { // make sure both summaries are available
    uint8_t dummy(-1);
    nBLhits  = IDTrkMuPos->summaryValue( dummy, xAOD::numberOfBLayerHits ) ? dummy :-1;
    nhitsPIX = IDTrkMuPos->summaryValue( dummy, xAOD::numberOfPixelHits ) ? dummy :-1;
    nhitsSCT = IDTrkMuPos->summaryValue( dummy, xAOD::numberOfSCTHits ) ? dummy :-1;
    nhitsTRT = IDTrkMuPos->summaryValue( dummy, xAOD::numberOfTRTHits ) ? dummy :-1;
    nContribPixLayers = IDTrkMuPos->summaryValue( dummy, xAOD::numberOfContribPixelLayers )? dummy :-1;

    nPIXholes = IDTrkMuPos->summaryValue( dummy, xAOD::numberOfPixelHoles )? dummy :-1;
    nSCTholes = IDTrkMuPos->summaryValue( dummy, xAOD::numberOfSCTHoles )? dummy :-1;


    m_nBLhits.push_back(nBLhits);
    m_nPIXhits.push_back(nhitsPIX);
    m_nSCThits.push_back(nhitsSCT);
    m_nTRThits.push_back(nhitsTRT);

    ATH_MSG_DEBUG (" -- mu_pos -- HITS --"
		   << "  nBLhits:  " << nBLhits
		   << "  nhitsPIX: " << nhitsPIX
		   << "  nPIXLayers: " << nContribPixLayers
		   << "  nhitsSCT: " << nhitsSCT
		   << "  Silicon holes: " << nPIXholes + nSCTholes
		   << "    nhitsTRT: " << nhitsTRT);

    nBLhits  = IDTrkMuNeg->summaryValue( dummy, xAOD::numberOfBLayerHits ) ? dummy :-1;
    nhitsPIX = IDTrkMuNeg->summaryValue( dummy, xAOD::numberOfPixelHits ) ? dummy :-1;
    nhitsSCT = IDTrkMuNeg->summaryValue( dummy, xAOD::numberOfSCTHits ) ? dummy :-1;
    nhitsTRT = IDTrkMuNeg->summaryValue( dummy, xAOD::numberOfTRTHits ) ? dummy :-1;
    nContribPixLayers = IDTrkMuNeg->summaryValue( dummy, xAOD::numberOfContribPixelLayers )? dummy :-1;
    nPIXholes = IDTrkMuNeg->summaryValue( dummy, xAOD::numberOfPixelHoles )? dummy :-1;
    nSCTholes = IDTrkMuNeg->summaryValue( dummy, xAOD::numberOfSCTHoles )? dummy :-1;

    m_nBLhits.push_back(nBLhits);
    m_nPIXhits.push_back(nhitsPIX);
    m_nSCThits.push_back(nhitsSCT);
    m_nTRThits.push_back(nhitsTRT);

    ATH_MSG_DEBUG (" -- mu_neg -- HITS --"
		   << "  nBLhits:  " << nBLhits
		   << "  nhitsPIX: " << nhitsPIX
		   << "  nPIXLayers: " << nContribPixLayers
		   << "  nhitsSCT: " << nhitsSCT
		   << "  Silicon holes: " << nPIXholes + nSCTholes
		   << "    nhitsTRT: " << nhitsTRT);

  }
  return;
}
