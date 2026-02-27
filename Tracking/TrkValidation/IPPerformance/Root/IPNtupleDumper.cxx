
#include "IPPerformance/EventSelectorAlg.h"
#include "IPPerformance/ETAlgorithm.h"
#include "IPPerformance/ReturnCheck.h"
#include "IPPerformance/IPNtupleDumper.h"
#include "AthContainers/ConstDataVector.h"

#include "xAODCutFlow/CutBookkeeperContainer.h" // MVGR

//xAOD
#include "xAODRootAccess/tools/TFileAccessTracer.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODJet/JetContainer.h"

#include "TSystem.h"
#include "TEnv.h"
#include "TProfile.h"
#include <iostream>
#include <bitset>

// Local include(s):
#include "TrackVertexAssociationTool/TrackVertexAssociationTool.h"


IPNtupleDumper::IPNtupleDumper(const std::string& name, ISvcLocator* pSvcLocator) 
: ETAlgorithm(name, pSvcLocator)
{
  m_name = name;
  m_histOutput         = "IPNtupleDumper";
  m_inJetContainer     = "AntiKt4EMTopoJets_Selected";
  //m_inJetContainer     = "AntiKt4TruthJets_Selected";
  m_TruthPtCut         = 500.;
  m_TruthEtaCut        = 2.5;
  m_TruthMatchProb     = 0.5;
  m_doTightTruthMatch  = true;
  m_doGhostAssociation = true;
  m_inRecTrkContainer  = "InDetTrackParticles";
  m_vtxContainer       = "PrimaryVertices";
  derivationName     = ""; // MVGR: Due to a mismatch between the derivation name when using or not the CA (IDTIDE vs. IDTIDE1), the derivation name is now obtained automatically using the function "GetDerivationName" 
  m_deltaRCut          = 0.4;
  m_applyDeltaRCut     = true;
  m_skipPVfailures     = true;
  m_TTVA_WP            = "Prompt_MaxWeight";
  m_noTTVAonInput      = true;
  m_debug              = false; 
  m_IPhistos           = nullptr;

}

IPNtupleDumper ::~IPNtupleDumper() {}


StatusCode IPNtupleDumper :: initialize ()
{

  // Here you do everything that you need to do after the first input
  // file has been connected and before the first event is processed,
  // e.g. create additional histograms based on which variables are
  // available in the input files. You can also create all of your
  // histograms and trees in here, but be aware that this method
  // doesn't get called if no events are processed. So any objects
  // you create here won't be available in the output if you have no
  // input events.

  Info("initialize()", "Initializing IPNtupleDumper..." );
  setConfig(m_configFileName);  
  Info("IPNtupleDumper::configure()", "Calling configure");
  //https://its.cern.ch/jira/browse/ATLASG-809
  xAOD::TFileAccessTracer::enableDataSubmission( false );

  if (!getConfig().empty()) {
    Info("configure()", "Configuring IPNtupleDumper interface. User configuration read from : %s ", getConfig().c_str());
    TEnv* config         = new TEnv(getConfig(true).c_str());
    m_inJetContainer     = config->GetValue("InputJetContainer", m_inJetContainer.c_str());
    m_TruthPtCut         = config->GetValue("TruthPtCut", m_TruthPtCut);
    m_TruthEtaCut        = config->GetValue("TruthEtaCut", m_TruthEtaCut);
    m_TruthMatchProb     = config->GetValue("TruthMatchProb", m_TruthMatchProb);
    m_doTightTruthMatch  = config->GetValue("doTightTruthMatch", m_doTightTruthMatch);
    m_inRecTrkContainer  = config->GetValue("InputTrackContainer", m_inRecTrkContainer.c_str());
    m_vtxContainer       = config->GetValue("InputVertexContainer", m_vtxContainer.c_str());
    m_doGhostAssociation = config->GetValue("DoGhostAssociation", m_doGhostAssociation);
    m_deltaRCut          = config->GetValue("DeltaRCut", m_deltaRCut);
    m_applyDeltaRCut     = config->GetValue("applyDeltaRCut", m_applyDeltaRCut);
    m_TTVA_WP            = config->GetValue("TTVA_WP", m_TTVA_WP.c_str());
    m_noTTVAonInput      = config->GetValue("noTTVAonInput", m_noTTVAonInput);
    m_debug              = config->GetValue("debug", m_debug);

    config->Print();
    delete config;
  }

  if (m_inJetContainer.empty()) {
    Info("IPNtupleDumper::configure()","InputJetContainer is empty!");
    return StatusCode::FAILURE;
  }
  Info("IPNtupleDumper::configure()","InputJetContainer: %s", m_inJetContainer.c_str());

  if (m_inRecTrkContainer.empty()) {
    Info("IPNtupleDumper::configure()","InputTrackContainer is empty!");
    return StatusCode::FAILURE;
  }
  Info("IPNtupleDumper::configure()","InputTrackContainer: %s", m_inRecTrkContainer.c_str());

  if (m_vtxContainer.empty()) {
    Info("IPNtupleDumper::configure()","InputVertexContainer is empty!");
    return StatusCode::FAILURE;
  }
  Info("IPNtupleDumper::configure()","InputVertexContainer: %s", m_vtxContainer.c_str());


  //Track to vertex tool
  ANA_CHECK(m_trktovxtool.retrieve());

  //LoosePrimary Track Selection
  ANA_CHECK(m_LoosePrimary_selTool.retrieve());
  m_trackselectionTools.push_back(m_LoosePrimary_selTool.get());
  
  //TightPrimary Track Selection
  ANA_CHECK(m_TightPrimary_selTool.retrieve());
  m_trackselectionTools.push_back(m_TightPrimary_selTool.get());
  
  //TrackTruthHelper
  ANA_MSG_INFO("initialize(),Initialising TrackTruthHelpers...");
  ANA_MSG_INFO( "   pt cut: " << m_TruthPtCut );
  ANA_MSG_INFO( "   eta cut: " << m_TruthEtaCut );
  ANA_MSG_INFO( "   truth matching probability: " << m_TruthMatchProb );
  m_truthhelper = new TrackTruthHelpers(m_TruthPtCut,m_TruthEtaCut,m_TruthMatchProb);

  /*const xAOD::EventInfo* evtInfo = 0;
  //if (!m_event->retrieve(evtInfo,"EventInfo").isSuccess()) {
  if (!evtStore()->retrieve(evtInfo,"EventInfo").isSuccess()) {
    Error("execute()","Failed to retrieve event info. Exiting.");
    return StatusCode::FAILURE;
  }
  m_isMC = evtInfo->eventType( xAOD::EventInfo::IS_SIMULATION ); //it can modified in configFile.
  runN   = evtInfo->runNumber();*/ //has been in execute()!
 
  
  ATH_CHECK(m_trigDecKey.initialize());
  if(!m_isMC){ 
    // trigger initialisation if running on data 
    Info("initialize()", "Trigger");
    ANA_CHECK(m_trigConfTool.retrieve());
    ToolHandle< TrigConf::ITrigConfigTool > configHandle( m_trigConfTool.get() );
    ANA_CHECK( m_trigDecTool.retrieve());
    ANA_CHECK( m_trigDecTool->setProperty( "ConfigTool", configHandle ));
    ANA_CHECK( m_trigDecTool->setProperty( "TrigDecisionKey", "xTrigDecision" ));
    ANA_CHECK( m_trigDecTool->setProperty( "OutputLevel", MSG::ERROR));
    //RETURN_CHECK("IPNtupleDumper::initialize()", m_trigDecTool->initialize(), "Failed to initialise TrigDecisionTool!");
  }


  if (!ipSaveHistosOnly) {
    t1 = new TTree("IPtree", "IPtree");
    SetBranches(t1);
    ANA_CHECK( histSvc()->regTree("/MYSTREAM/IPtree",t1));

    h_SumOfEventWeights = new TH1D("h_SumOfEventWeights", "", 2, 0.5, 1.5);
    ANA_CHECK( histSvc()->regHist("/MYSTREAM/h_SumOfEventWeights",h_SumOfEventWeights));
  }
  else{ // Set up all the tools needed for IP studies

    Info("histInitialize()", "Initializing IPhistos class");
    m_IPhistos = std::make_unique<IPhistos>("default_");
    Info("histInitialize()", "Saving additonal IP histograms? %s", std::to_string(ipSaveAdditionalHistos).c_str());
    m_IPhistos->SaveAdditionalHistos(ipSaveAdditionalHistos);
    Info("histInitialize()", "Defining 3D histograms");
    m_IPhistos->Define3DHistos();
    for (const auto& histEntry : m_IPhistos->get3DHistos()) {
        const std::string& histName = histEntry.first;
        TH3D* hist = histEntry.second;
	ANA_CHECK( histSvc()->regHist("/MYSTREAM/"+histName,hist));
    }
    Info("histInitialize()", "Defining 2D histograms");
    m_IPhistos->Define2DHistos();
    for (const auto& histEntry : m_IPhistos->get2DHistos()) {
        const std::string& histName = histEntry.first;
        TH2D* hist = histEntry.second;
        ANA_CHECK( histSvc()->regHist("/MYSTREAM/"+histName,hist));
    }

    Info("histInitialize()", "Booking histograms");
    m_IPhistos->BookHistograms();
    for(const auto& histvector : m_IPhistos->get1Dvector()) {
       for(const auto& hist: histvector) {
          const std::string& histName = hist->GetName();
          ANA_CHECK( histSvc()->regHist("/MYSTREAM/"+histName,hist));
       }
    }

    Info("histInitialize()", "Histograms booked successfully!");

  }

  return StatusCode::SUCCESS;

}


StatusCode IPNtupleDumper :: finalize ()
{

  // This method is the mirror image of initialize(), meaning it gets
  // called after the last event has been processed on the worker node
  // and allows you to finish up any objects you created in
  // initialize() before they are written to disk. This is actually
  // fairly rare, since this happens separately for each worker node.
  // Most of the time you want to do your post-processing on the
  // submission node after all your histogram outputs have been
  // merged. This is different from histFinalize() in that it only
  // gets called on worker nodes that processed input events.

  Info("finalize()", "Deleting tool instances...");

  if (m_truthhelper) delete m_truthhelper;
  m_trackselectionTools.clear();

  return StatusCode::SUCCESS;

}

StatusCode IPNtupleDumper :: execute ()
{

  //Retrieve event Info
  const xAOD::EventInfo* evtInfo = 0;
  if (!evtStore()->retrieve(evtInfo,"EventInfo").isSuccess()) {
    Error("execute()","Failed to retrieve event info. Exiting.");
    return StatusCode::FAILURE;
  }
  
  // Check if we are running on MC
  m_isMC = evtInfo->eventType( xAOD::EventInfo::IS_SIMULATION );
  if ( m_debug ) { Info("initialize()", "Is MC? %i", static_cast<int>(m_isMC) ); }

  // Set event weight to MC event weight on MC
  if( m_isMC ) evtW = evtInfo->mcEventWeight();
  else evtW = 1;
  
  // Retrieve vertices 
  const xAOD::VertexContainer* vtxContainer = 0;
  if (!evtStore()->retrieve(vtxContainer,m_vtxContainer).isSuccess()) {
    Error("execute()","Failed to retrieve vertex container. Exiting.");
    return StatusCode::FAILURE;
  }
  // Sanity check
  if( ! vtxContainer->size() ) {
    Warning("execute()", "Event with no vertex found!" );
    return StatusCode::SUCCESS;
   }

  // Get Primary Vertex
  const xAOD::Vertex* primVtx = 0;
  for (auto vtx : *vtxContainer) {
    if (vtx->vertexType() == xAOD::VxType::PriVtx)
      primVtx = vtx;
  }
  
  if (!primVtx) {
    Warning("execute()","Failed finding primary vertex. Exiting.");
    return StatusCode::SUCCESS;
  }
  
  // Retrieve Jets
  const xAOD::JetContainer* inJets_Selected = 0; 
  if (!evtStore()->retrieve(inJets_Selected, m_inJetContainer).isSuccess()){ //retrieve the jets selected and calibrated
    Error("execute()", "Failed to retrieve Selected Reconstructed Jets Container. Exiting.");
    return StatusCode::FAILURE;
  }
  
  // Retrieve trackParticles
  const xAOD::TrackParticleContainer* recoTracksSelected = 0;
  if (!evtStore()->retrieve (recoTracksSelected, m_inRecTrkContainer).isSuccess() ) {
    Error("execute()","Failed to retrieve track container. Exiting.");
    return StatusCode::FAILURE;
  }

  // MVGR : Retrieve AntiKt4EMPFlowJets. These are needed for weight corrections when using Run-3 data, as skimming in Run-3 no longer relies in EMTopoJets
  const xAOD::JetContainer* inAntiKt4EMPFlowJets = 0; 
  if (!evtStore()->retrieve(inAntiKt4EMPFlowJets, "AntiKt4EMPFlowJets").isSuccess()){ //retrieve the jets
    Error("execute()", "Failed to retrieve AntiKt4EMPFlowJets Container. Exiting.");
    return StatusCode::FAILURE;
  }

  // MVGR: Get the Derivation Name
  derivationName = GetDerivationName(recoTracksSelected);
  if (derivationName == ""){
    Error("execute()", "Could not find the derivation name from the output. Exiting.");
    return StatusCode::FAILURE;
  }
  
  // Check for the decorations. If not available exit. 
  if (!CheckForAvailableDecorations(recoTracksSelected, derivationName)) {
    Error("execute()","IP decorations not available. Exiting.");
    return StatusCode::FAILURE;
  }

  // General events quantities
  lb       = evtInfo->lumiBlock();
  runN     = evtInfo->runNumber();
  evtN     = evtInfo->eventNumber();
  mu       = evtInfo->averageInteractionsPerCrossing();
  pvx      = primVtx->x();
  pvy      = primVtx->y();
  pvz      = primVtx->z();
  pvN      = primVtx->nTrackParticles();
  // MC16 bs should be x,y,z (-0.5,-0.5,0)
  // Sigmas: (10um,10um, 42mm)
  bsx      = evtInfo->beamPosX();
  bsy      = evtInfo->beamPosY();
  bsz      = evtInfo->beamPosZ();
  bsSigmax = evtInfo->beamPosSigmaX();
  bsSigmay = evtInfo->beamPosSigmaY();
  bsSigmaz = evtInfo->beamPosSigmaZ();
  
  // Retrieve uncalibrated jet pt to correct for prescales on data
  // https://acode-browser.usatlas.bnl.gov/lxr/source/athena/PhysicsAnalysis/DerivationFramework/DerivationFrameworkInDet/share/IDTIDE1.py
  if( !m_isMC ){ 

    // MVGR ========= Applying weight correction in data due to the Skimming prescales in IDTIDE
    float factor_prescale = 1.0;

    if( runN >= 423433 ){  // For Run-3 we should use AntiKt4EMPFlowJets

      float max_constit_mom = 0.;
      for (auto jet : *inAntiKt4EMPFlowJets) {
	static const SG::Accessor<float> mAcc_jet_constit_scale_pt("JetConstitScaleMomentum_pt");
	float jet_constit_scale_pt = mAcc_jet_constit_scale_pt(*jet);
        if (jet_constit_scale_pt > max_constit_mom) max_constit_mom = jet_constit_scale_pt;   
      }

      if (max_constit_mom*0.001 < 600){
        // Checking the triggers and applying prescale corrections
        if(!ipHLTcorrection){
          factor_prescale = 20;
        }
        else{
          auto printingTriggerChainGroup = m_trigDecTool->getChainGroup("HLT_j[0-9]*_pf_ftf_preselj[0-9]*_L1J[0-9]*");
          for(auto &trig : printingTriggerChainGroup->getListOfTriggers()) {
            // The list actually comes from lower to higher jet scales.
            // This means in the end the code will get the factor_prescale from the fired trigger with highest jetPt
            auto cg = m_trigDecTool->getChainGroup(trig);
            auto trig_prescale = m_trigDecTool->getPrescale(trig); // Getting intrinsic trigger prescale (not related to IDTIDE)
            std::string trigger_name = trig.c_str();
            
            if (trigger_name == "HLT_j110_pf_ftf_preselj80_L1J30" || trigger_name == "HLT_j175_pf_ftf_preselj140_L1J50" || trigger_name == "HLT_j260_pf_ftf_preselj200_L1J75"){
              if( cg->isPassed()) factor_prescale = 20 * trig_prescale;
            }

            if (trigger_name == "HLT_j360_pf_ftf_preselj225_L1J100"){
              if( cg->isPassed()) factor_prescale = 40 * trig_prescale;
            } 
            
            if (trigger_name == "HLT_j420_pf_ftf_preselj225_L1J100" ){
              if ( cg->isPassed()) factor_prescale = 30 * trig_prescale;
            }
            if (trigger_name == "HLT_j460_pf_ftf_preselj225_L1J100" ){
              if ( cg->isPassed()) factor_prescale = 20 * trig_prescale;
            }
          }
        }
      }
      else if (600 <= max_constit_mom*0.001 && max_constit_mom*0.001 < 800) factor_prescale=10;
      else if (800 <= max_constit_mom*0.001 && max_constit_mom*0.001 < 1000) factor_prescale=5;
    }

    else{  // For Run-2 we should use AntiKt4EMTopoJets
      
      float max_constit_mom = 0.;
      for (auto jet : *inJets_Selected) {
	static const SG::Accessor<float> mAcc_jet_constit_scale_pt("JetConstitScaleMomentum_pt");
	float jet_constit_scale_pt = mAcc_jet_constit_scale_pt(*jet);
        if (jet_constit_scale_pt > max_constit_mom) max_constit_mom = jet_constit_scale_pt;   
      }
      if (max_constit_mom*0.001 < 600){
        // Checking the triggers and applying prescale corrections
        if(!ipHLTcorrection){
          factor_prescale = 20;
        }
        else{
          auto printingTriggerChainGroup = m_trigDecTool->getChainGroup("HLT_j[0-9]*");
          for(auto &trig : printingTriggerChainGroup->getListOfTriggers()) {
            // The list actually comes from lower to higher jet scales.
            // This means in the end the code will get the factor_prescale from the fired trigger with highest jetPt
            auto cg = m_trigDecTool->getChainGroup(trig);
            auto trig_prescale = m_trigDecTool->getPrescale(trig); // Getting intrinsic trigger prescale (not related to IDTIDE)
            std::string trigger_name = trig.c_str();
            
            if (trigger_name == "HLT_j110" || trigger_name == "HLT_j175" || trigger_name == "HLT_j260"){
              if( cg->isPassed()) factor_prescale = 20 * trig_prescale;
            }

            if (trigger_name == "HLT_j360" || trigger_name == "HLT_j380" || trigger_name == "HLT_j400"){
              if( cg->isPassed()) factor_prescale = 40 * trig_prescale;
            } 
            
            if (trigger_name == "HLT_j420" ){
              if ( cg->isPassed()) factor_prescale = 30 * trig_prescale;
            }
            if (trigger_name == "HLT_j460" ){
              if ( cg->isPassed()) factor_prescale = 20 * trig_prescale;
            }
          }
        }
      }
      else if (600 <= max_constit_mom*0.001 && max_constit_mom*0.001 < 800) factor_prescale = 10;
      else if (800 <= max_constit_mom*0.001 && max_constit_mom*0.001 < 1000) factor_prescale = 5;
    }

    evtW *= factor_prescale;
  }

  // ============

  // If doing GhostAssociation:
  // 1) Loop on selected jets
  // 2) Get ghost-associated tracks
  // 3) Loop on tracks
  // 4) DeltaR cut (if requested)
  // 5) Fill tree variables
  
  if (m_doGhostAssociation) {//m_doGhostAssociation=true
    for (auto jet : *inJets_Selected) {
      std::vector<const xAOD::TrackParticle*> jetTracks; 
      bool haveJetTracks = jet->getAssociatedObjects(xAOD::JetAttribute::GhostTrack, jetTracks);
      if (!haveJetTracks) {
        Warning("execute()","No ghost-associated tracks");
        continue;
      }
      ntracks = jetTracks.size();
      for (auto *trk : jetTracks)
        ProcessTrack(trk, jet, vtxContainer);
    }
  }
  else {
    // If checking all tracks
    // 1) Loop on tracks
    // 2) Match the tracks to selected jets via DeltaR
    // 3) Apply DeltaR cut (if requested)
    // 4) Fill tree variables
    
    for (auto track : *recoTracksSelected) {  
      ntracks = recoTracksSelected->size();
      ProcessTrack(track, inJets_Selected, vtxContainer);
    }       
  } 
  
  // MVGR ==========
  const xAOD::JetContainer* inAntiKt4EMTopoJets = 0;
  if (!evtStore()->retrieve(inAntiKt4EMTopoJets, "AntiKt4EMTopoJets").isSuccess()){ // Retrieve AntiKtEMTopoJets directly from input
    Error("execute()", "Failed to retrieve AntiKt4EMTopoJets Container. Exiting.");
    return StatusCode::FAILURE;
  }
  for (auto jet : *inAntiKt4EMTopoJets) {
    static const SG::Accessor<float> mAcc_jet_constit_scale_pt("JetConstitScaleMomentum_pt");
    float jet_constit_scale_pt = mAcc_jet_constit_scale_pt(*jet);
    jetPt_Uncalibrated.push_back(jet_constit_scale_pt*0.001); // Store the uncalibrated jets for comparisons
  }
  // MVGR ==============

  if (!ipSaveHistosOnly) {  
    t1->Fill();
    h_SumOfEventWeights->Fill(1.0, evtW);
  }
  else {
    if (FillIPHistograms() != StatusCode::SUCCESS) {
      Error("execute()", "Failed to fill IP histograms. Exiting.");
      return StatusCode::FAILURE;
    };
  }

  ResetVars();
  
  return StatusCode::SUCCESS;

}



void IPNtupleDumper::SetBranches(TTree* t) 
{

  t->Branch("NTracks",        &ntracks,  "ntracks/I");
  t->Branch("lb",             &lb,       "lb/I");
  t->Branch("runN",           &runN,     "runN/I");
  t->Branch("evtW",           &evtW,     "evtW/F");
  t->Branch("mu",             &mu,       "mu/F");
  t->Branch("bsx",            &bsx,      "bsx/F");
  t->Branch("bsy",            &bsy,      "bsy/F");
  t->Branch("bsz",            &bsz,      "bsz/F");
  t->Branch("bsSigmax",       &bsSigmax, "bsSigmax/F");
  t->Branch("bsSigmay",       &bsSigmay, "bsSigmay/F");
  t->Branch("bsSigmaz",       &bsSigmaz, "bsSigmaz/F");
  t->Branch("pvx",            &pvx,      "pvx/F");
  t->Branch("pvy",            &pvy,      "pvy/F");
  t->Branch("pvz",            &pvz,      "pvz/F");
  t->Branch("pvN",            &pvN,      "pvN/I");

  t->Branch("selectionBits",  &selectionBits); 
  t->Branch("d0_pv",          &d0_pv);
  t->Branch("z0_pv",          &z0_pv);
  t->Branch("d0Sigma_pv",     &d0Sigma_pv);
  t->Branch("z0Sigma_pv",     &z0Sigma_pv);
  t->Branch("d0PVSigma_pv",   &d0PVSigma_pv);
  t->Branch("z0PVSigma_pv",   &z0PVSigma_pv);
  t->Branch("trk_pt",         &trk_pt);
  t->Branch("trk_eta",        &trk_eta);
  t->Branch("trk_theta",      &trk_theta);
  t->Branch("trk_q",          &trk_q);
  t->Branch("trk_hitPattern", &trk_hitPattern);
  t->Branch("trk_phi",        &trk_phi);
  t->Branch("trk_chi2",       &trk_chi2);
  t->Branch("trk_d0",         &trk_d0);
  t->Branch("trk_z0",         &trk_z0);
  t->Branch("trk_d0Sigma",    &trk_d0Sigma);
  t->Branch("trk_z0Sigma",    &trk_z0Sigma);

  t->Branch("trk_nInnermostPixelLayerHits",             &trk_nInnermostPixelLayerHits);
  t->Branch("trk_nInnermostPixelLayerOutliers",         &trk_nInnermostPixelLayerOutliers);
  t->Branch("trk_nInnermostPixelLayerSharedHits",       &trk_nInnermostPixelLayerSharedHits);
  t->Branch("trk_nInnermostPixelLayerSplitHits",        &trk_nInnermostPixelLayerSplitHits);
  t->Branch("trk_expectInnermostPixelLayerHit",         &trk_expectInnermostPixelLayerHit);
  t->Branch("trk_nNextToInnermostPixelLayerHits",       &trk_nNextToInnermostPixelLayerHits);
  t->Branch("trk_nNextToInnermostPixelLayerOutliers",   &trk_nNextToInnermostPixelLayerOutliers);
  t->Branch("trk_nNextToInnermostPixelLayerSharedHits", &trk_nNextToInnermostPixelLayerSharedHits);
  t->Branch("trk_nNextToInnermostPixelLayerSplitHits",  &trk_nNextToInnermostPixelLayerSplitHits);
  t->Branch("trk_expectNextToInnermostPixelLayerHit",   &trk_expectNextToInnermostPixelLayerHit);
  
  t->Branch("deltaRtrkJet", &deltaRtrkJet);
  t->Branch("jetPt",        &jetPt);
  t->Branch("jetEta",       &jetEta);
  t->Branch("jetJVT",       &jetJVT);
  
  //Would like to fill this only if this is MC...but okay. 
  t->Branch("trk_truth_pt",    &trk_truth_pt);
  t->Branch("trk_truth_d0",    &trk_truth_d0);
  t->Branch("trk_truth_z0",    &trk_truth_z0);
  t->Branch("trk_truth_prodR", &trk_truth_prodR);
  t->Branch("trk_truth_phi",   &trk_truth_phi);
  t->Branch("trk_barcode",     &trk_barcode);
  t->Branch("trk_pdgId",       &trk_pdgId);

  // MVGR
  t->Branch("jetPt_Uncalibrated",      &jetPt_Uncalibrated);
  t->Branch("numberOfPixelHits",       &numberOfPixelHits);
  t->Branch("numberOfPixelHoles",      &numberOfPixelHoles);
  t->Branch("numberOfPixelOutliers",   &numberOfPixelOutliers);
  t->Branch("numberOfPixelSharedHits", &numberOfPixelSharedHits);
  t->Branch("numberOfPixelSplitHits",  &numberOfPixelSplitHits);
  t->Branch("numberOfSCTHits",         &numberOfSCTHits);
  t->Branch("numberOfSCTHoles",        &numberOfSCTHoles);
  t->Branch("numberOfSCTOutliers",     &numberOfSCTOutliers);
  t->Branch("numberOfSCTSharedHits",   &numberOfSCTSharedHits);
   
}

void IPNtupleDumper::ResetVars() 
{ 
    
    runN = -999;
    lb   = -999;
    evtN = -999;
    evtW = -999;
    bsx  = -999;
    bsy  = -999;
    bsz  = -999;
    bsSigmax = -999;
    bsSigmay = -999;
    bsSigmaz = -999;
    pvx  = -999;
    pvy  = -999;
    pvz  = -999;
    pvN  = -999;
    
    selectionBits  .clear();
    d0_pv          .clear(); 
    z0_pv          .clear(); 
    d0Sigma_pv     .clear(); 
    z0Sigma_pv     .clear(); 
    d0PVSigma_pv   .clear(); 
    z0PVSigma_pv   .clear(); 
    trk_pt         .clear();    
    trk_eta        .clear();
    trk_theta      .clear();
    trk_phi        .clear();
    trk_q          .clear();
    trk_hitPattern .clear();
    trk_chi2       .clear();
    deltaRtrkJet   .clear();
    jetPt          .clear();
    jetEta         .clear();
    jetJVT         .clear();
    trk_d0         .clear();
    trk_z0         .clear();
    trk_d0Sigma    .clear();
    trk_z0Sigma    .clear();
    trk_truth_pt   .clear();
    trk_truth_d0   .clear();
    trk_truth_z0   .clear();
    trk_truth_prodR.clear();
    trk_truth_phi  .clear();
    trk_barcode    .clear();
    trk_pdgId      .clear();
    
    trk_nInnermostPixelLayerHits            .clear();
    trk_nInnermostPixelLayerOutliers        .clear();
    trk_nInnermostPixelLayerSharedHits      .clear();
    trk_nInnermostPixelLayerSplitHits       .clear();
    trk_expectInnermostPixelLayerHit        .clear();
    trk_nNextToInnermostPixelLayerHits      .clear();
    trk_nNextToInnermostPixelLayerOutliers  .clear();
    trk_nNextToInnermostPixelLayerSharedHits.clear();
    trk_nNextToInnermostPixelLayerSplitHits .clear();
    trk_expectNextToInnermostPixelLayerHit  .clear();
    trk_nPixelSharedHits                    .clear();
    trk_nPixelSplitHits                     .clear();
    trk_nSCTSharedHits                      .clear();

    // MVGR
    jetPt_Uncalibrated.clear();
    numberOfPixelHits.clear();
    numberOfPixelHoles.clear();
    numberOfPixelOutliers.clear();
    numberOfPixelSharedHits.clear();
    numberOfPixelSplitHits.clear();
    numberOfSCTHits.clear();
    numberOfSCTHoles.clear();
    numberOfSCTOutliers.clear();
    numberOfSCTSharedHits.clear();
    
}

bool IPNtupleDumper::CheckForAvailableDecorations(const xAOD::TrackParticleContainer* trkC, const std::string& derivationName ) 
{

  bool available = false;
  
  xAOD::TrackParticleContainer::const_iterator trk = trkC->begin(); 
  static const SG::Accessor<float> mAcc_unbias_d0(derivationName + "_unbiased_d0");
  static const SG::Accessor<float> mAcc_unbias_z0(derivationName + "_unbiased_z0");
  static const SG::Accessor<float> mAcc_unbias_d0Sigma(derivationName + "_unbiased_d0Sigma");
  static const SG::Accessor<float> mAcc_unbias_z0Sigma(derivationName + "_unbiased_z0Sigma");
  static const SG::Accessor<float> mAcc_unbias_PVd0Sigma(derivationName + "_unbiased_PVd0Sigma");
  static const SG::Accessor<float> mAcc_unbias_PVz0Sigma(derivationName + "_unbiased_PVz0Sigma");

  if (mAcc_unbias_d0.isAvailable(**trk) &&
      mAcc_unbias_z0.isAvailable(**trk) &&
      mAcc_unbias_d0Sigma.isAvailable(**trk) &&
      mAcc_unbias_z0Sigma.isAvailable(**trk) &&
      mAcc_unbias_PVd0Sigma.isAvailable(**trk) &&
      mAcc_unbias_PVz0Sigma.isAvailable(**trk) )
    available = true;

    
  return available;
   
}

std::string IPNtupleDumper::GetDerivationName(const xAOD::TrackParticleContainer* trkC) // We will get the derivation name from the variables in the output
{ 
  
  xAOD::TrackParticleContainer::const_iterator trk = trkC->begin();
  static const SG::Accessor<float> mAcc_IDTIDE("IDTIDE_unbiased_d0");
  static const SG::Accessor<float> mAcc_IDTIDE1("IDTIDE1_unbiased_d0");

  //if( (*trk)->isAvailable<float>("IDTIDE_unbiased_d0") ){
  if(mAcc_IDTIDE.isAvailable(**trk)){
    return "IDTIDE";
  }
  //else if( (*trk)->isAvailable<float>("IDTIDE1_unbiased_d0") ){
  else if(mAcc_IDTIDE1.isAvailable(**trk)){
    return "IDTIDE1";
  }
  else{
    return "";
  }

}

void IPNtupleDumper::TrackToJetDeltaRAssociation(const xAOD::TrackParticle* trk, const xAOD::JetContainer* jets, float dRcut,float& deltaR, float& pT, float& Etajet, int &JVTjet) 
{

  bool nearjet = false;
  float best_deltaR = dRcut;
  float best_pT     = -1;
  float best_eta     = -1;
  int   best_JVTpass = -1;
  float jetpT = -1;
  float jeteta = -1;

  for (xAOD::JetContainer::const_iterator jet_itr=jets->begin(); jet_itr!=jets->end(); ++jet_itr) {
    jetpT = ((*jet_itr)->pt())*0.001;
    jeteta = (*jet_itr)->eta();
    
    bool pass = false;
    static const SG::Accessor<char> mAcc_passSel("passSel");
    if (!mAcc_passSel.isAvailable(**jet_itr))
      pass = true;
    else {
      if (mAcc_passSel(**jet_itr)==1)
      pass=true;
    }
    if (!pass)
      continue;
    float deltaR = (*jet_itr)->p4().DeltaR( (trk)->p4() );
    if(deltaR <best_deltaR) {
      nearjet=true;
      best_deltaR = deltaR;
      best_pT = jetpT;
      best_eta = jeteta;
      best_JVTpass = PassJVTCut(*jet_itr);
    } // match
  } // jet loop
 
  if (nearjet) {
    deltaR = best_deltaR;
    pT     = best_pT;
    Etajet = best_eta;
    JVTjet = best_JVTpass;
  }
  else {
    deltaR = -1;
    pT = -1;
    Etajet = -1;
    JVTjet = -1;
  }

}

bool IPNtupleDumper::ProcessTrack(const xAOD::TrackParticle* track, const  xAOD::JetContainer* jets, const xAOD::VertexContainer* vtxCont) 
{

  // Track to jet deltaR association => bit of duplicated code. Should move to some helpers.
  // Could even move this to a different algorithm and add more info on jet flavour!
  float deltaR_trk_jet = -1;
  float pTjet          = -1;
  float Etajet         = -1;
  int   JVTjet         = -1;
  
  //Remove nullptrs
  if (!track )
    return false;

  // Option to allow processing of samples without TTVA information
  // Consider implementing track-vertex cut by hand as temporary fix
  if( m_noTTVAonInput ) Warning("IPNtupleDumper::ProcessTrack", "Skipping TTVA for now.");
  else{
  
  // Check if track has at least one associated primary vertex
  bool hasMatch = false;
  bool hasMatchFirst = false;
  int count_vtx = 0;

  for (xAOD::VertexContainer::const_iterator vtx_itr = vtxCont->begin(); vtx_itr != vtxCont->end(); ++vtx_itr) {

    const xAOD::Vertex * vtx = dynamic_cast<const xAOD::Vertex *>(*vtx_itr);
    count_vtx++;

    hasMatch = m_trktovxtool->isCompatible(*(track), *(vtx));
    if( hasMatch && count_vtx == 1 ) hasMatchFirst = true;
    if( hasMatch ) break;
  }

  // Remove tracks without associated vertices
  if( !hasMatch ) {
    if( m_debug ) Warning("IPNtupleDumper::ProcessTrack", "No vertex found for track.");
    return false;
  }

  // Remove tracks not associated to first primary vertex
  // KB: assume this is the vertex of the jet --> is there a way of checking this on the fly?
  if ( !(hasMatchFirst) ) {
    if( m_debug ) Warning("IPNtupleDumper::ProcessTrack", "Track not associated to primary vertex.");
    return false;
  }

  } // End of m_noTTVAonInput
  
  // Associate tracks to jets and fill jet variables
  TrackToJetDeltaRAssociation(track, jets, m_deltaRCut, deltaR_trk_jet, pTjet, Etajet, JVTjet);
  
  // deltaR_trk>0 only if the track is associated to a jet
  if (m_applyDeltaRCut && deltaR_trk_jet < 0)
    return false;
  FillTreeVariables(track, deltaR_trk_jet, pTjet, Etajet, JVTjet);
  
  return true;

}

bool IPNtupleDumper::ProcessTrack(const xAOD::TrackParticle* track, const xAOD::Jet* jet, const xAOD::VertexContainer* vtxCont) 
{
  // Remove nullptrs
  if (!track )
    return false;
  // Option to allow processing of samples without TTVA information
  // Consider implementing track-vertex cut by hand as temporary fix
  if( m_noTTVAonInput ) Warning("IPNtupleDumper::ProcessTrack", "Skipping TTVA for now.");//"m_noTTVAonInput is false"
  else{

    //Check if track has at least one associated primary vertex
    bool hasMatch = false;
    bool hasMatchFirst = false;
    int count_vtx = 0;
    for (xAOD::VertexContainer::const_iterator vtx_itr = vtxCont->begin(); vtx_itr != vtxCont->end(); ++vtx_itr) {

      const xAOD::Vertex * vtx = dynamic_cast<const xAOD::Vertex *>(*vtx_itr);
      count_vtx++;

      hasMatch = m_trktovxtool->isCompatible(*(track), *(vtx));
      if( hasMatch && count_vtx == 1 ) hasMatchFirst = true;
      if( hasMatch ) break;
    }
    // Remove tracks without associated vertices
    if( !hasMatch ) {
      if( m_debug )  Warning("IPNtupleDumper::ProcessTrack", "No vertex found for track.");
      return false;
    }

    // Remove tracks not associated to first primary vertex
    // KB: assume this is the vertex of the jet --> is there a way of checking this on the fly?
    if ( !(hasMatchFirst) ){
      if( m_debug ) Warning("IPNtupleDumper::ProcessTrack", "Track not associated to primary vertex.");
      return false;
    }

  } // End of m_noTTVAonInput

  float deltaR_trk_jet = jet->p4().DeltaR(track->p4());
  if (m_applyDeltaRCut && deltaR_trk_jet > m_deltaRCut) //"m_applyDeltaRCut is false"
    return false;
  float pTjet  = jet->pt() *  0.001;
  float Etajet = jet->eta();
  int   JVTjet = PassJVTCut(jet);
 
  FillTreeVariables(track, deltaR_trk_jet, pTjet, Etajet, JVTjet);
  
  return true;

}

void IPNtupleDumper::FillTreeVariables(const xAOD::TrackParticle* track,float deltaR_trk_jet,float pTjet,float Etajet,int JVTjet) 
{
          
  uint8_t selBits = 0x0;
 
  static const SG::Accessor<float> mAcc_d0PV(derivationName + "_unbiased_d0");   
  static const SG::Accessor<float> mAcc_z0PV(derivationName + "_unbiased_z0");   
  static const SG::Accessor<float> mAcc_d0SigmaPV(derivationName + "_unbiased_d0Sigma");   
  static const SG::Accessor<float> mAcc_z0SigmaPV(derivationName + "_unbiased_z0Sigma");   
  static const SG::Accessor<float> mAcc_d0PVSigmaPV(derivationName + "_unbiased_PVd0Sigma");   
  static const SG::Accessor<float> mAcc_z0PVSigmaPV(derivationName + "_unbiased_PVz0Sigma");   
    
  float d0PV        = mAcc_d0PV(*track); 
  float z0PV        = mAcc_z0PV(*track); 
  float d0SigmaPV   = mAcc_d0SigmaPV(*track); 
  float z0SigmaPV   = mAcc_z0SigmaPV(*track); 
  float d0PVSigmaPV = mAcc_d0PVSigmaPV(*track); 
  float z0PVSigmaPV = mAcc_z0PVSigmaPV(*track);
 
  
  if (m_skipPVfailures && (d0PV==999 || z0PV==999 || d0SigmaPV==999 || z0SigmaPV==999 || d0PVSigmaPV==999 || z0PVSigmaPV==999))
    return;
  
  for (unsigned int i_selTool = 0; i_selTool<m_trackselectionTools.size(); i_selTool++) {
    if (m_trackselectionTools[i_selTool]->accept(track))
      selBits |= 0x1<<i_selTool;
  }
  
  selectionBits.push_back(selBits);
  trk_pt.push_back(track->pt());

  if (m_isMC) {
    
    const xAOD::TruthParticle* truth_p = nullptr;
    if (m_doTightTruthMatch){
      if (m_debug) Info("IPNtupleDumper::FillTreeVariables", "Running tight truth matching.");
      truth_p = getAssociatedPrimaryTruth(track);
    }
    else truth_p = getTrackTruthLink(track);

    if (truth_p){ 
      trk_truth_pt.push_back(truth_p->pt());
      trk_barcode.push_back(truth_p->barcode());
      trk_pdgId.push_back(truth_p->pdgId());
      static const SG::Accessor<float> mAcc_d0("d0");
      static const SG::Accessor<float> mAcc_z0("z0");
      static const SG::Accessor<float> mAcc_prodR("prodR");
      static const SG::Accessor<float> mAcc_phi("phi");
      float true_d0 = mAcc_d0.isAvailable(*truth_p) ? mAcc_d0(*truth_p) : -999.;
      float true_z0 = mAcc_z0.isAvailable(*truth_p) ? mAcc_z0(*truth_p) : -999.;
      float true_prodR = mAcc_prodR.isAvailable(*truth_p) ? mAcc_prodR(*truth_p) : -999.;
      float true_phi = mAcc_phi.isAvailable(*truth_p) ? mAcc_phi(*truth_p) : -999.;

      trk_truth_d0.push_back(true_d0);
      trk_truth_z0.push_back(true_z0);
      trk_truth_prodR.push_back(true_prodR);
      trk_truth_phi.push_back(true_phi);
    }
    else{
      trk_truth_pt.push_back(-999);
      trk_truth_pt.push_back(-999);
      trk_barcode.push_back(-999);
      trk_pdgId.push_back(-999);
      trk_truth_prodR.push_back(-999.);
      trk_truth_d0.push_back(-999.);
      trk_truth_z0.push_back(-999.);
      trk_truth_phi.push_back(-999.);
    }
  }
  else{
    trk_truth_pt.push_back(-999);
    trk_truth_pt.push_back(-999);
    trk_barcode.push_back(-999);
    trk_pdgId.push_back(-999);
    trk_truth_prodR.push_back(-999.);
    trk_truth_d0.push_back(-999.);
    trk_truth_z0.push_back(-999.);
    trk_truth_phi.push_back(-999.);
  }
  trk_eta.push_back(track->eta());
  trk_theta.push_back(track->theta());
  trk_phi.push_back(track->phi());
  trk_d0.push_back(track->d0());
  trk_z0.push_back(track->z0());
  trk_q.push_back(track->charge());
  trk_hitPattern.push_back(track->hitPattern()); // KB: adding hit pattern
  trk_chi2.push_back(track->chiSquared()); // KB: adding chi2 
  d0_pv.push_back( d0PV );
  z0_pv.push_back( z0PV );
  d0Sigma_pv.push_back( d0SigmaPV );
  z0Sigma_pv.push_back( z0SigmaPV );
  d0PVSigma_pv.push_back( d0PVSigmaPV );
  z0PVSigma_pv.push_back( z0PVSigmaPV );
  jetPt.push_back(pTjet);
  jetEta.push_back(Etajet);
  jetJVT.push_back(JVTjet);
  deltaRtrkJet.push_back(deltaR_trk_jet);
  
  // Elements in definingParametersCovMatrixDiag should be: sigma_d0^2, sigma_z0^2, sigma_phi^2, sigma_th^2, sigma_qp^2
  float tmp = track->definingParametersCovMatrixDiagVec()[0];
  if (tmp>0){
    tmp=sqrt(tmp);
  } else {tmp=0.;};
  trk_d0Sigma.push_back(tmp);
  float tmp2 = track->definingParametersCovMatrixDiagVec()[1];
  if (tmp2>0){
    tmp2=sqrt(tmp2);
  } else {tmp2=0;};
  trk_z0Sigma.push_back(tmp2);
  
  static const SG::Accessor<unsigned char> mAcc_nInnermostPixelLayerHit ("numberOfInnermostPixelLayerHits");
  static const SG::Accessor<unsigned char> mAcc_nInnermostPixelLayerOutliers ("numberOfInnermostPixelLayerOutliers");
  static const SG::Accessor<unsigned char> mAcc_nInnermostPixelLayerSharedHits ("numberOfInnermostPixelLayerSharedHits");
  static const SG::Accessor<unsigned char> mAcc_nInnermostPixelLayerSplitHits ("numberOfInnermostPixelLayerSplitHits");
  static const SG::Accessor<unsigned char> mAcc_expectInnermostPixelLayerHit ("expectInnermostPixelLayerHit");
  static const SG::Accessor<unsigned char> mAcc_nNextToInnermostPixelLayerHits ("numberOfNextToInnermostPixelLayerHits");
  static const SG::Accessor<unsigned char> mAcc_nNextToInnermostPixelLayerOutliers ("numberOfNextToInnermostPixelLayerOutliers");
  static const SG::Accessor<unsigned char> mAcc_nNextToInnermostPixelLayerSharedHits ("numberOfNextToInnermostPixelLayerSharedHits");
  static const SG::Accessor<unsigned char> mAcc_nNextToInnermostPixelLayerSplitHits ("numberOfNextToInnermostPixelLayerSplitHits");
  static const SG::Accessor<unsigned char> mAcc_expectNextToInnermostPixelLayerHit ("expectNextToInnermostPixelLayerHit");
  static const SG::Accessor<unsigned char> mAcc_nPixelSharedHits ("numberOfPixelSharedHits");
  static const SG::Accessor<unsigned char> mAcc_nPixelSplitHits ("numberOfPixelSplitHits");
  static const SG::Accessor<unsigned char> mAcc_nSCTSharedHits ("numberOfSCTSharedHits");

  unsigned char nInnermostPixelLayerHit = mAcc_nInnermostPixelLayerHit(*track);
  unsigned char nInnermostPixelLayerOutliers = mAcc_nInnermostPixelLayerOutliers(*track);
  unsigned char nInnermostPixelLayerSharedHits= mAcc_nInnermostPixelLayerSharedHits(*track);
  unsigned char nInnermostPixelLayerSplitHits = mAcc_nInnermostPixelLayerSplitHits(*track);
  unsigned char expectInnermostPixelLayerHit = mAcc_expectInnermostPixelLayerHit(*track);
  unsigned char nNextToInnermostPixelLayerHits =  mAcc_nNextToInnermostPixelLayerHits(*track);
  unsigned char nNextToInnermostPixelLayerOutliers = mAcc_nNextToInnermostPixelLayerOutliers(*track);
  unsigned char nNextToInnermostPixelLayerSharedHits = mAcc_nNextToInnermostPixelLayerSharedHits(*track);
  unsigned char nNextToInnermostPixelLayerSplitHits = mAcc_nNextToInnermostPixelLayerSplitHits(*track);
  unsigned char expectNextToInnermostPixelLayerHit =  mAcc_expectNextToInnermostPixelLayerHit(*track);
  unsigned char nPixelSharedHits = mAcc_nPixelSharedHits(*track);
  unsigned char nPixelSplitHits =  mAcc_nPixelSplitHits(*track);
  unsigned char nSCTSharedHits = mAcc_nSCTSharedHits(*track);   


  trk_nInnermostPixelLayerHits.push_back( nInnermostPixelLayerHit );
  trk_nInnermostPixelLayerOutliers.push_back( nInnermostPixelLayerOutliers );
  trk_nInnermostPixelLayerSharedHits.push_back( nInnermostPixelLayerSharedHits );
  trk_nInnermostPixelLayerSplitHits.push_back( nInnermostPixelLayerSplitHits );
  trk_expectInnermostPixelLayerHit.push_back( expectInnermostPixelLayerHit );
  trk_nNextToInnermostPixelLayerHits.push_back( nNextToInnermostPixelLayerHits );
  trk_nNextToInnermostPixelLayerOutliers.push_back( nNextToInnermostPixelLayerOutliers );
  trk_nNextToInnermostPixelLayerSharedHits.push_back( nNextToInnermostPixelLayerSharedHits );
  trk_nNextToInnermostPixelLayerSplitHits.push_back( nNextToInnermostPixelLayerSplitHits );
  trk_expectNextToInnermostPixelLayerHit.push_back( expectNextToInnermostPixelLayerHit );
  trk_nPixelSharedHits.push_back( nPixelSharedHits );
  trk_nPixelSplitHits.push_back( nPixelSplitHits );
  trk_nSCTSharedHits.push_back( nSCTSharedHits );
  
  // MVGR
  static const SG::Accessor<unsigned char> mAcc_numberOfPixelHits ("numberOfPixelHits");
  static const SG::Accessor<unsigned char> mAcc_numberOfPixelHoles ("numberOfPixelHoles");
  static const SG::Accessor<unsigned char> mAcc_numberOfPixelOutliers ("numberOfPixelOutliers");
  static const SG::Accessor<unsigned char> mAcc_numberOfPixelSharedHits ("numberOfPixelOutliers");
  static const SG::Accessor<unsigned char> mAcc_numberOfPixelSplitHits ("numberOfPixelSplitHits");
  static const SG::Accessor<unsigned char> mAcc_numberOfSCTHits ("numberOfSCTHits");
  static const SG::Accessor<unsigned char> mAcc_numberOfSCTHoles ("numberOfSCTHoles");
  static const SG::Accessor<unsigned char> mAcc_numberOfSCTOutliers ("numberOfSCTOutliers");
  static const SG::Accessor<unsigned char> mAcc_numberOfSCTSharedHits ("numberOfSCTSharedHits");
 
  numberOfPixelHits      .push_back( mAcc_numberOfPixelHits(*track)); 
  numberOfPixelHoles     .push_back( mAcc_numberOfPixelHoles(*track));
  numberOfPixelOutliers  .push_back( mAcc_numberOfPixelOutliers(*track));
  numberOfPixelSharedHits.push_back( mAcc_numberOfPixelSharedHits(*track));
  numberOfPixelSplitHits.push_back(mAcc_numberOfPixelSplitHits(*track));
  numberOfSCTHits        .push_back(mAcc_numberOfSCTHits(*track));
  numberOfSCTHoles       .push_back(mAcc_numberOfSCTHoles(*track));
  numberOfSCTOutliers    .push_back(mAcc_numberOfSCTOutliers(*track));
  numberOfSCTSharedHits  .push_back(mAcc_numberOfSCTSharedHits(*track));  
 

}

// This should eventually return a bool..
// TODO:Remove hardcoding
// from: https://acode-browser2.usatlas.bnl.gov/lxr/source/r21/atlas/Reconstruction/MET/METUtilities/Root/METMaker.cxx
// cuts from:
// https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/BTaggingBenchmarksRelease21
int IPNtupleDumper::PassJVTCut(const xAOD::Jet* jet) 
{
  if (jet->pt() < 60000 && fabs(jet->eta() < 2.4)) {
    float jvt = -999;
    bool gotJVT = jet->getAttribute<float>("Jvt",jvt);
    if (gotJVT) {
      if (jvt<0.59)
        return 0;
      else
        return 1;
    }
    //else
    //Warning("PassJVTCut()","Couldn't retrieve Jvt");
  }
  //If the jet is not decorated with jvt, or jetpT>60 GeV
  return -1;

}
 
// Original truth-truth matching, based only on element link
// No selection on truth particle of truth matching probability
const xAOD::TruthParticle* IPNtupleDumper::getTrackTruthLink(const xAOD::TrackParticle* track) const 
{

  const xAOD::TruthParticle* truth = nullptr;
  static SG::AuxElement::ConstAccessor<Link_t> linkAcc("truthParticleLink");
  if (linkAcc.isAvailable(*track)){
    const Link_t& link = linkAcc(*track);
    if (link.isValid()){
      truth=*link;
    }
  }
  return truth;

}

// Alternative track-truth matching based on TrackTruthHelper
// See the following talk for different track-truh-matching approaches
// https://indico.cern.ch/event/795039/contributions/3391771/attachments/1857138/3050771/TruthTrackFTAGWS.pdf
const xAOD::TruthParticle* IPNtupleDumper::getAssociatedPrimaryTruth(const xAOD::TrackParticle* track) const
{

  const xAOD::TruthParticle* truth = nullptr;
  
  // Check if the track particle originated from a primary charged particle 
  // within detector acceptance and with good truth match probability
  if (m_truthhelper->isPrimary( track )){
  //@TODO for just test 
  //if (true){
    truth = m_truthhelper->truthParticle( track );
  }

  return truth;

}

StatusCode IPNtupleDumper::FillIPHistograms() 
{

  if(trk_pt.size() == 0) return StatusCode::SUCCESS;

  // Checking which classes do this track (with index i) lie in
  std::vector<std::vector<int>> class_satisfied; 
  for (unsigned int i = 0; i < trk_pt.size(); ++i)
    class_satisfied.push_back(Classify(i));
      
  for (unsigned int i = 0; i < trk_pt.size(); ++i) {
    std::vector<float> weights(15, 1); // for same track in different classes
    
    // Selection on jetPt
    if (jetPt.at(i) > 300 && selectionBits.at(i) > 1) { // Tight selection

      for (int class_index = 0; class_index < int(class_satisfied.at(i).size()); class_index++) {
        if (!m_isMC)
          weights.at(class_index) = 1;
        else if (class_satisfied.at(i)[class_index]) {
          float net_weight = evtW; //mc_weight * evtW * mc_reweight;
          weights.at(class_index) = net_weight;
        }
      }

      // To find out deltaR to the nearest track
      double deltaR_trk12 = 9999;
      for (unsigned int i2 = 0; i2 < trk_pt.size(); ++i2) {
        if (jetPt.at(i) == jetPt.at(i2)) { 
          // Both tracks in same jet (To find: closest track in same jet)
          double deltaR = TMath::Sqrt(TMath::Power((trk_eta.at(i) - trk_eta.at(i2)), 2) + TMath::Power((trk_phi.at(i) - trk_phi.at(i2)), 2));
          if (deltaR < deltaR_trk12 && deltaR != 0)
            deltaR_trk12 = deltaR;
        }
      }

	    float intLumi = -100.;
      if (GetIntLumi(runN, intLumi) != StatusCode::SUCCESS)
        return StatusCode::FAILURE;

      double bsWidth = (bsSigmax + bsSigmay) / 2.;
      m_IPhistos->FillHistograms(trk_d0.at(i) * 1000., (trk_z0.at(i) - pvz + bsz) * 1000, trk_pt.at(i) * 0.001, trk_eta.at(i), trk_phi.at(i),
				                           runN, mu, jetPt.at(i), weights, bsWidth, deltaR_trk12, class_satisfied.at(i), intLumi);
      
    } // jetpT selection
  
  } // loop on tracks

  return StatusCode::SUCCESS;

}

std::vector<int> IPNtupleDumper::Classify(unsigned int i)
{

  // init vector of 0 (false)
  std::vector<int> output(15, 0);

  if (trk_nInnermostPixelLayerHits.at(i) == 0 && trk_nNextToInnermostPixelLayerHits.at(i) == 0 && trk_expectInnermostPixelLayerHit.at(i) >= 1 && trk_expectNextToInnermostPixelLayerHit.at(i) >= 1)
    output.at(0) = 1; //!
  if (trk_nInnermostPixelLayerHits.at(i) == 0 && trk_nNextToInnermostPixelLayerHits.at(i) == 0 && trk_expectInnermostPixelLayerHit.at(i) >= 1 && trk_expectNextToInnermostPixelLayerHit.at(i) == 0)
    output.at(1) = 1;
  if (trk_nInnermostPixelLayerHits.at(i) == 0 && trk_nNextToInnermostPixelLayerHits.at(i) == 0 && trk_expectInnermostPixelLayerHit.at(i) == 0 && trk_expectNextToInnermostPixelLayerHit.at(i) >= 1)
    output.at(2) = 1;
  if (trk_nInnermostPixelLayerHits.at(i) == 0 && trk_nNextToInnermostPixelLayerHits.at(i) == 0 && trk_expectInnermostPixelLayerHit.at(i) == 0 && trk_expectNextToInnermostPixelLayerHit.at(i) == 0)
    output.at(3) = 1;
  if (trk_nInnermostPixelLayerHits.at(i) == 0 && trk_expectInnermostPixelLayerHit.at(i) >= 1)
    output.at(4) = 1;
  if (trk_nInnermostPixelLayerHits.at(i) == 0 && trk_expectInnermostPixelLayerHit.at(i) == 0)
    output.at(5) = 1;
  if (trk_nNextToInnermostPixelLayerHits.at(i) == 0 && trk_expectNextToInnermostPixelLayerHit.at(i) >= 1)
    output.at(6) = 1;
  if (trk_nNextToInnermostPixelLayerHits.at(i) == 0 && trk_expectNextToInnermostPixelLayerHit.at(i) == 0)
    output.at(7) = 1;
  if (trk_nInnermostPixelLayerSharedHits.at(i) >= 1 && trk_nNextToInnermostPixelLayerSharedHits.at(i) >= 1)
    output.at(8) = 1; //TMP not using now;
  if (trk_nPixelSharedHits.at(i) >= 1)
    output.at(9) = 1;
  if (trk_nSCTSharedHits.at(i) >= 2)
    output.at(10) = 1;
  if (trk_nInnermostPixelLayerSplitHits.at(i) >= 1 && trk_nNextToInnermostPixelLayerSplitHits.at(i) >= 1)
    output.at(11) = 1;
  if (trk_nPixelSplitHits.at(i) >= 1)
    output.at(12) = 1;
  if (!(trk_nInnermostPixelLayerHits.at(i) == 0 && trk_nNextToInnermostPixelLayerHits.at(i) == 0) && !(trk_nInnermostPixelLayerHits.at(i) == 0) && !(trk_nNextToInnermostPixelLayerHits.at(i) == 0) && !(trk_nInnermostPixelLayerSharedHits.at(i) >= 1 && trk_nNextToInnermostPixelLayerSharedHits.at(i) >= 1) && !(trk_nPixelSharedHits.at(i) >= 1) && !(trk_nSCTSharedHits.at(i) >= 2) && !(trk_nInnermostPixelLayerSplitHits.at(i) >= 1 && trk_nNextToInnermostPixelLayerSplitHits.at(i) >= 1) && !(trk_nPixelSplitHits.at(i) >= 1))
    output.at(13) = 1;
  output.at(14) = 1;

  return output;

}

StatusCode IPNtupleDumper::GetIntLumi(Int_t runN, float& int_lumi)
{

  const std::string fname = "/afs/cern.ch/work/x/xai/public/ATLAS/CTIDE/src/CTIDEefficiency/macros/IPanalysis_vRun3/util/runToLumi.txt";
  std::ifstream inFile(fname);
  if (!inFile) {
    Error("GetIntLumi", "Unable to open x-section file");
    return StatusCode::FAILURE; // terminate with error
  }
  
  std::vector<std::string> lines;
  std::string line;

  int i=0;
  while(!inFile.eof()){
    getline(inFile, line);

    if( !(line.length() == 0 || line[0] == '#') )
      lines.push_back(line);
    ++i;
  }
  inFile.close();

  int found = -1, flag2= -1, dsid = runN; 
  for(unsigned int k = 0; k < lines.size(); k++){
    std::stringstream ll(lines[k]);
    std::string substr;
    int ind = 0;
    while (ll >> substr){
      //Run Number 
      if (ind == 0){
        if( std::stof(substr) == dsid ){
          found = 1;
        }
      }
      //Integrated Luminosity
      if (ind == 5){
	      if (found == 1){
          flag2 = 1;
	        int_lumi = std::stof(substr);
        }
      }
      ind++;
    }
    if (flag2 == 1)
      break;
  }

  return StatusCode::SUCCESS;

}
