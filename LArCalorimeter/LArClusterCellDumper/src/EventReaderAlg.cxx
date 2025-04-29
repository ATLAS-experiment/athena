// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#include "LArClusterCellDumper/EventReaderAlg.h"

#include "CaloEvent/CaloCellContainer.h"
#include "CaloEvent/CaloClusterCellLinkContainer.h"
#include "CaloEvent/CaloCompactCellContainer.h"

#include "Identifier/Identifier.h"
#include "Identifier/HWIdentifier.h"

#include "CaloGeoHelpers/CaloSampling.h"

#include "xAODTruth/xAODTruthHelpers.h"
#include "xAODTracking/TrackParticlexAODHelpers.h"

#include "TruthUtils/MagicNumbers.h"

#include <bitset>
#include "CLHEP/Units/SystemOfUnits.h"
#include "TTree.h"
#include "xAODEgamma/EgammaxAODHelpers.h" //for getAssociatedTopoClusters


using CLHEP::twopi;
using CLHEP::GeV;
using CLHEP::pi;

EventReaderAlg::EventReaderAlg( const std::string& name, ISvcLocator* pSvcLocator ) : 
    EventReaderBaseAlg(name, pSvcLocator)
    , m_ntsvc("THistSvc/THistSvc", name)
    {}

StatusCode EventReaderAlg::initialize() {
  ATH_MSG_INFO ("Initializing " << name() << "...");
  ATH_MSG_INFO ("(EventReader) Initializing conditions...");

  // Conditions  
  ATH_CHECK(m_run2DSPThresholdsKey.initialize(!m_isMC));
  ATH_CHECK(m_EneRescalerFldr.initialize(!m_isMC));
  ATH_CHECK(m_noiseCDOKey.initialize());  //---- retrieve the noise CDO
  ATH_CHECK(m_lumiDataKey.initialize() );
  ATH_CHECK(m_pedestalKey.initialize());     
  ATH_CHECK(m_adc2MeVKey.initialize());  
  ATH_CHECK(m_ofcKey.initialize());  
  ATH_CHECK(m_shapeKey.initialize());
  ATH_CHECK(m_minBiasAvgKey.initialize());
  ATH_CHECK(m_fSamplKey.initialize());
  ATH_CHECK(m_offlineHVScaleCorrKey.initialize());
  ATH_MSG_DEBUG(" (EventReader) Condition Keys initialized!");

  // Containers
  ATH_CHECK(m_eventInfoSgKey.initialize());
  ATH_CHECK(m_primVertSgKey.initialize());
  ATH_CHECK(m_caloClusSgKey.initialize());
  ATH_CHECK(m_myElecSelectionSgKey.initialize());
  ATH_CHECK(m_truthParticleCntSgKey.initialize());
  ATH_CHECK(m_electronCntSgKey.initialize());
  ATH_CHECK(m_truthEventCntSgKey.initialize());
  ATH_CHECK(m_larEMBHitCntSgKey.initialize());
  ATH_CHECK(m_larDigitCntSgKey.initialize());
  ATH_CHECK(m_larRawChCntSgKey.initialize());
  ATH_CHECK(m_allCaloCellCntSgKey.initialize());
  ATH_MSG_DEBUG(" (EventReader) Container Keys initialized!");

  // -- Retrieve ID Helpers --
  ATH_CHECK( detStore()->retrieve(m_onlineLArID, "LArOnlineID") ); // get online ID info from LAr (online)  
  ATH_CHECK( detStore()->retrieve (m_calocell_id, "CaloCell_ID") );
  ATH_CHECK( m_larCablingKey.initialize());

  StatusCode sc = detStore()->retrieve(m_caloIdMgr);
  if (sc.isFailure()) {
    ATH_MSG_ERROR(" Unable to retrieve CaloIdManager from DetectoreStore");
    return StatusCode::FAILURE;
  }
  else{
    m_larem_id   = m_caloIdMgr->getEM_ID();
    m_larhec_id  = m_caloIdMgr->getHEC_ID();
    m_larfcal_id = m_caloIdMgr->getFCAL_ID();
  }
  // -------------------------
  
  // Book the variables to save in the *.root 
  m_Tree        = new TTree("dumpedData", "dumpedData");
  m_secondTree  = new TTree("lumiblockData", "lumiblockData");

  ATH_MSG_DEBUG("Booking branches...");
  bookBranches(m_Tree); // book TTree with branches
  bookDatabaseBranches(m_secondTree);

  if (!m_ntsvc->regTree("/rec/tree1", m_Tree).isSuccess()) {
      ATH_MSG_ERROR("could not register tree [dumpedData]");
      return StatusCode::FAILURE;
    }
  if (!m_ntsvc->regTree("/rec/tree2", m_secondTree).isSuccess()) {
      ATH_MSG_ERROR("could not register tree [database]");
      return StatusCode::FAILURE;
    }

  if      (m_doTagAndProbe && m_doElecSelectByTrackOnly)  ATH_MSG_INFO("Entering single_e mode...");
  else if (m_doTagAndProbe && !m_doElecSelectByTrackOnly) ATH_MSG_INFO("Entering Zee T&P mode...");
  else {
    ATH_MSG_ERROR("No valid electron chain selected!");
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

StatusCode EventReaderAlg::execute(){  
  ATH_MSG_DEBUG ("Executing " << name() << "...");
  ATH_MSG_DEBUG("Cleanning of event variables...");
  const EventContext& ctx = getContext();
  clear();  // clear all variables selected to dump to the output NTuple.

  // Conditions
  if (!m_isMC){
  
    if (!m_EneRescalerFldr.empty()){
      SG::ReadCondHandle<AthenaAttributeList> EneRescalerHdl  (m_EneRescalerFldr,ctx);
      const coral::Blob& blob = (**EneRescalerHdl)["CaloCondBlob16M"].data<coral::Blob>();
      if (blob.size()<3)  ATH_MSG_WARNING("Found empty blob, no correction needed");
      m_EneRescaler = CaloCondBlobFlt::getInstance(blob); // offline egamma cell correction (due from 5 to 4 downsampling rate)
      ATH_MSG_DEBUG("offlineEnergyRescaler is set!");
    }

    if (!m_run2DSPThresholdsKey.empty()) {
      SG::ReadCondHandle<AthenaAttributeList> dspThrshAttrHdl (m_run2DSPThresholdsKey,ctx);
      m_run2DSPThresh   = std::unique_ptr<LArDSPThresholdsFlat>(new LArDSPThresholdsFlat(*dspThrshAttrHdl));
      ATH_MSG_DEBUG("DSPThreshold has set!");
    }

    if (!m_lumiDataKey.empty()){
      SG::ReadCondHandle<LuminosityCondData>  lumiHdl  (m_lumiDataKey,ctx);
      m_lumis =*lumiHdl;
      ATH_MSG_DEBUG("LuminosityCondData is set!");
    }

    if (!m_offlineHVScaleCorrKey.empty()){
      SG::ReadCondHandle<LArHVCorr>  oflHVCorrHdl (m_offlineHVScaleCorrKey,ctx);
      m_oflHVCorr =*oflHVCorrHdl;
      ATH_MSG_DEBUG("OfflineHVCorrection is set!");
    }
  }
  else{
    if (!m_fSamplKey.empty()){ // MC
      SG::ReadCondHandle<ILArfSampl> fSamplHdl (m_fSamplKey,ctx);
      m_fSampl = *fSamplHdl;
      ATH_MSG_DEBUG("SamplingFraction is set!");
    }
  }  

  SG::ReadCondHandle<LArOnOffIdMapping>     larCablingHdl       (m_larCablingKey,ctx);
  SG::ReadCondHandle<CaloNoise>             noiseHdl            (m_noiseCDOKey,ctx);
  SG::ReadCondHandle<ILArPedestal>          pedHdl              (m_pedestalKey,ctx);
  SG::ReadCondHandle<LArADC2MeV>            adc2mevHdl          (m_adc2MeVKey,ctx);
  SG::ReadCondHandle<ILArOFC>               ofcHdl              (m_ofcKey,ctx);
  SG::ReadCondHandle<ILArShape>             shapeHdl            (m_shapeKey,ctx);
  SG::ReadCondHandle<ILArMinBiasAverage>    minBiasAvgHdl       (m_minBiasAvgKey,ctx);

  m_larCabling    = *larCablingHdl;
  m_noiseCDO      = *noiseHdl;
  m_peds          = *pedHdl;
  m_adc2MeVs      = *adc2mevHdl;
  m_ofcs          = *ofcHdl;
  m_shapes        = *shapeHdl;
  m_minBiasAvgs   = *minBiasAvgHdl;

  SG::ReadHandle<xAOD::EventInfo> ei = SG::makeHandle(m_eventInfoSgKey,ctx);
  if (!ei.isValid()) ATH_MSG_ERROR(" EventInfo container is not valid!");

  if (ei->errorState(xAOD::EventInfo::LAr) == xAOD::EventInfo::Error) {
    ATH_MSG_WARNING("Event not passing LAr! Skipping event.");
    return StatusCode::SUCCESS;
  }

  SG::ReadHandle<xAOD::VertexContainer> primVertexCnt = SG::makeHandle(m_primVertSgKey,ctx);
  if (!primVertexCnt.isValid()) ATH_MSG_ERROR(" Primary Vertex container is not valid!");  

  // *** Lumiblock data ***
  //    Verify if ei->lumiBlock() was dumped into lb_lumiblock. if not, add it.
  //     Each LB entry should be dumped just once, since their content doesnt change per event.
  if (std::count(m_lb_lumiblock->begin(), m_lb_lumiblock->end(), ei->lumiBlock()) == 0){
    if(!dumpLumiblockInfo(ei)) ATH_MSG_INFO("Lumiblock information cannot be collected!");
  }

  // EventInfo
  if(!dumpEventInfo(ei)) ATH_MSG_INFO("Event information cannot be collected!");
 
  // Clusters
  if (m_doClusterDump){

    SG::ReadHandle<xAOD::CaloClusterContainer> cls = SG::makeHandle(m_caloClusSgKey,ctx);

    if(!cls.isValid()){
      ATH_MSG_ERROR("CaloCluster container is not valid!");
      return StatusCode::FAILURE;
    } 

    for (const xAOD::CaloCluster *cl : *cls){
      ATH_MSG_DEBUG ("Cluster "<< m_c_clusterIndexCounter << ", Total size: " << cl->numberCells() << "/  "<< cl->clusterSize() << ", etaBE: " << cl->etaBE(2) << ", numberCellsInSampling: " << cl->numberCellsInSampling(CaloSampling::EMB2) <<", eta size: " << cl->etasize(CaloSampling::EMB2) << ", phi size: " << cl->phisize(CaloSampling::EMB2) );

      if (! dumpClusterCells(cl, m_c_clusterIndexCounter , ctx) )  ATH_MSG_WARNING ("CaloCluster Cells cannot be dumped for Zee.");

      (m_c_clusterIndexCounter)++;
    }
  }

  // Apply the Zee cut
  if (m_doTagAndProbe && !m_doClusterDump){

    if(!dumpZeeCut(ei,primVertexCnt, ctx)) ATH_MSG_DEBUG("Zee cut algorithm cannot be executed!");

    std::string eSelectionText = "";
    SG::ReadHandle<xAOD::ElectronContainer> myElectronsSelection;

    if (m_doElecSelectByTrackOnly) eSelectionText  = "Number of myElectronsSelection single electrons: ";
    else                           eSelectionText  = "Number of myElectronsSelection tagAndProbe Zee candidates: ";

    myElectronsSelection = SG::makeHandle(m_myElecSelectionSgKey,ctx);
    // Optional: Skip empty events.
    if (m_skipEmptyEvents && (myElectronsSelection->size() == 0)){
      ATH_MSG_INFO("This event has no selected electrons! Cleanning event variables and skipping writing to NTuple...");
      clear();  // clear all variables selected to dump to the output NTuple.
      return StatusCode::SUCCESS;
    }

    if(!FillNTupleWithSelectedElectrons(ei, primVertexCnt, myElectronsSelection, eSelectionText, ctx)) ATH_MSG_DEBUG("FillNTupleWithElectrons cannot be executed!");
    
    if (m_isMC){
      // Particle Truth
      if (m_doTruthPartDump){
        SG::ReadHandle<xAOD::TruthParticleContainer> truthParticleCnt = SG::makeHandle(m_truthParticleCntSgKey,ctx);
        if (!truthParticleCnt.isValid()) ATH_MSG_ERROR("TruthParticleContainer is not valid!");

        if(!dumpTruthParticle(myElectronsSelection, truthParticleCnt)) ATH_MSG_DEBUG("Truth Particle information cannot be collected!");
      }
    }
  } // end if-start TagAndProbe chain
  
  m_Tree->Fill();
  
  return StatusCode::SUCCESS;  
}

StatusCode EventReaderAlg::FillNTupleWithSelectedElectrons(SG::ReadHandle<xAOD::EventInfo> &ei, SG::ReadHandle<xAOD::VertexContainer> &primVertexCnt, SG::ReadHandle<xAOD::ElectronContainer> &elContainer, std::string& eSelectionText, const EventContext& ctx){

  ATH_MSG_INFO(eSelectionText << elContainer->size() );

  int electronIndex = 0;
  for (const xAOD::Electron *elec : *elContainer){

    const auto& clusLinks = elec->caloClusterLinks(); // get the cluster links for the electron
    
    // loop over clusters and dump cells
    for (const auto& clusLink : clusLinks){

      // get associated topoclusters from superclusters        
      if (m_getAssociatedTopoCluster){
        auto assocCls = xAOD::EgammaHelpers::getAssociatedTopoClusters( (*clusLink) );

        for ( auto assocCl : assocCls){
          ATH_MSG_DEBUG ("AssociatedTopoCluster " << m_c_clusterIndexCounter << ": e: "<< assocCl->e() << ", eta: " << assocCl->eta() << ", phi: " << assocCl->phi() <<". Total size: " << assocCl->numberCells() << "/  "<< assocCl->clusterSize() << ", etaBE: " << assocCl->etaBE(2) << ", numberCellsInSampling: " << assocCl->numberCellsInSampling(CaloSampling::EMB2) <<", eta size: " << assocCl->etasize(CaloSampling::EMB2) << ", phi size: " << assocCl->phisize(CaloSampling::EMB2) );
          if (! dumpClusterCells(assocCl, m_c_clusterIndexCounter, ctx ) )  ATH_MSG_WARNING ("CaloCluster Cells cannot be dumped for Zee.");
          
          m_c_clusterIndexCounter++;
        }
      }
      else{ // dump only the superclusters
        if (! dumpClusterCells((*clusLink), m_c_clusterIndexCounter, ctx ) )  ATH_MSG_WARNING ("CaloCluster Cells cannot be dumped for Zee.");
        m_c_clusterIndexCounter++;
      }
      m_c_electronIndex_clusterLvl->push_back(electronIndex); // electron indexes for each cluster
    } //end-for loop in clusLinks

    // dump tag and probe electrons
    if (!dumpElectrons(elec)) ATH_MSG_WARNING ("Cannot dump ordinary electrons...");
    if (!dumpOfflineSS(elec)) ATH_MSG_WARNING ("Cannot dump offline shower shapes for electrons...");
    if (!dumpPrimVertexAssocToElectron(elec, primVertexCnt, ei)) ATH_MSG_WARNING ("Cannot dump primary vertexes info associated to electrons...");
    m_el_index->push_back(electronIndex);

    electronIndex++;

  } // end-for loop in m_electronsSelectedByTrack electrons

  return StatusCode::SUCCESS;
}

StatusCode EventReaderAlg::dumpOfflineSS(const xAOD::Electron *electron){
    m_el_f1->push_back(electron->showerShapeValue(xAOD::EgammaParameters::f1));
    m_el_f3->push_back(electron->showerShapeValue(xAOD::EgammaParameters::f3));
    m_el_eratio->push_back(electron->showerShapeValue(xAOD::EgammaParameters::Eratio));
    m_el_weta1->push_back(electron->showerShapeValue(xAOD::EgammaParameters::weta1));
    m_el_weta2->push_back(electron->showerShapeValue(xAOD::EgammaParameters::weta2));
    m_el_fracs1->push_back(electron->showerShapeValue(xAOD::EgammaParameters::fracs1));
    m_el_wtots1->push_back(electron->showerShapeValue(xAOD::EgammaParameters::wtots1));
    m_el_e277->push_back(electron->showerShapeValue(xAOD::EgammaParameters::e277));
    m_el_reta->push_back(electron->showerShapeValue(xAOD::EgammaParameters::Reta));
    m_el_rphi->push_back(electron->showerShapeValue(xAOD::EgammaParameters::Rphi));
    m_el_deltae->push_back(electron->showerShapeValue(xAOD::EgammaParameters::DeltaE));
    m_el_rhad->push_back(electron->showerShapeValue(xAOD::EgammaParameters::Rhad));
    m_el_rhad1->push_back(electron->showerShapeValue(xAOD::EgammaParameters::Rhad1));
   
  return StatusCode::SUCCESS;
}

StatusCode  EventReaderAlg::dumpElectrons(const xAOD::Electron* electron){
  float electronEt = electron->e()/(cosh(electron->trackParticle()->eta()));
  float track_p = (electron->trackParticle())->pt()*std::cosh((electron->trackParticle())->eta());
  float eoverp  = (electron->caloCluster())->e()/track_p;
  m_el_eoverp->push_back(eoverp);
  m_el_Pt->push_back(electron->pt());
  m_el_et->push_back(electronEt);
  m_el_Eta->push_back(electron->eta());
  m_el_Phi->push_back(electron->phi());
  m_el_m->push_back(electron->m());

  return StatusCode::SUCCESS;
}

StatusCode EventReaderAlg::dumpPrimVertexAssocToElectron(const xAOD::Electron *el, SG::ReadHandle<xAOD::VertexContainer> &primVertexCnt, SG::ReadHandle<xAOD::EventInfo> & evtInfo){

  const xAOD::TrackParticle *trackElectron =  el->trackParticle();

  // check for primary vertex in the event
  // *************** Primary vertex check ***************
  //loop over vertices and look for good primary vertex
  float   bestDeltaZ0Sin  = 9999.0, bestZfromVertex = 9999.0, bestXFromVertex = 9999.0, bestYFromVertex = 9999.0, bestDeltaZ0 = 9999.0;
  double  bestD0Sig       = 9999.0;
  int     vtxCounter      = 0;//, bestVtxCounter = 0;
  bool    isAnyClosestVtx = false;
  for (const xAOD::Vertex *vxIter : *primVertexCnt) {

    // Select good primary vertex
    float zFromVertex   = vxIter->z();
    float delta_z0      = fabs(trackElectron->z0() + trackElectron->vz() - zFromVertex); //where trk.vz() represents the point of reference for the z0 calculation (in this case, the beamspot position along the z axis).
    float delta_z0_sin  = delta_z0 * sin(trackElectron->theta()); //where sin(trk.theta()) parameterises the uncertainty of the z0 measurement. 
    double d0sig = xAOD::TrackingHelpers::d0significance( trackElectron, evtInfo->beamPosSigmaX(), evtInfo->beamPosSigmaY(), evtInfo->beamPosSigmaXY() );


    if ((vxIter->vertexType() == xAOD::VxType::PriVtx) && ( fabs(delta_z0_sin) < m_z0Tag) && (fabs(d0sig) < m_d0TagSig) ){ //is this vertex passed into d0sig and dz0Sin conditions?
      isAnyClosestVtx = true;
      
      ATH_MSG_DEBUG ("(dumpPrimVertexAssocToElectron) Vertex "<< vtxCounter << ": This is a primary vertex. |delta_z0_sin| < 0.5 mm ("<< delta_z0_sin << "). |d0sig| < 5 (" << d0sig << ")");
      if ( (fabs(delta_z0_sin) < fabs(bestDeltaZ0Sin) ) &&  (fabs(d0sig) < fabs(bestD0Sig)) ) // is this vertex the closest one to that electron track?
      {
        // assign all variables to dump:
        ATH_MSG_DEBUG ("(dumpPrimVertexAssocToElectron) New best associated Vertex. Vertex "<< vtxCounter << ": |delta_z0_sin| < 0.5 mm ("<< delta_z0_sin << "). |d0sig| < 5 (" << d0sig << ")");

        bestDeltaZ0Sin      = delta_z0_sin;
        bestD0Sig           = d0sig;

        bestXFromVertex = vxIter->x();
        bestYFromVertex = vxIter->y();
        bestZfromVertex = zFromVertex;
        bestDeltaZ0     = delta_z0;
      }
    }
    vtxCounter++;
  }
  // --------- Dump here: ---------
  if (isAnyClosestVtx){
    m_vtx_x->push_back(bestXFromVertex);
    m_vtx_y->push_back(bestYFromVertex);
    m_vtx_z->push_back(bestZfromVertex);
    m_vtx_deltaZ0->push_back(bestDeltaZ0);
    m_vtx_delta_z0_sin->push_back(bestDeltaZ0Sin);
    m_vtx_d0sig->push_back(bestD0Sig);
  }
  else{
    ATH_MSG_ERROR("A pre-selected electron track has no closest vertex to dump! (weird?)");
  }
  // -------------------------------


  return StatusCode::SUCCESS;
}

StatusCode EventReaderAlg::dumpLumiblockInfo(SG::ReadHandle<xAOD::EventInfo> &ei){

  if (!m_isMC){
    m_lb_bcidLuminosity->push_back(m_lumis->lbLuminosityPerBCIDVector());
    ATH_MSG_INFO ("LB_data: lbLuminosityPerBCIDVector stored into ntuple." );
  }
  m_lb_lumiblock->push_back( ei->lumiBlock() );
  ATH_MSG_INFO ("LB_data: Lumiblock "<< ei->lumiBlock() << " stored into ntuple." );

  return StatusCode::SUCCESS;
}

StatusCode EventReaderAlg::dumpEventInfo(SG::ReadHandle<xAOD::EventInfo> &ei){
  m_e_runNumber       = ei->runNumber();
  m_e_eventNumber     = ei->eventNumber();
  m_e_bcid            = ei->bcid();
  m_e_lumiBlock       = ei->lumiBlock();
  m_e_inTimePileup    = ei->actualInteractionsPerCrossing();
  m_e_outOfTimePileUp = ei->averageInteractionsPerCrossing();

  ATH_MSG_INFO ("in execute, runNumber = " << m_e_runNumber << ", eventNumber = " << m_e_eventNumber << ", bcid: " << m_e_bcid );
  
  return StatusCode::SUCCESS;
}

StatusCode EventReaderAlg::dumpZeeCut(SG::ReadHandle<xAOD::EventInfo> &ei, SG::ReadHandle<xAOD::VertexContainer> &primVertexCnt, const EventContext& ctx){
  ATH_MSG_INFO("Dumping Zee cut region...");
  
  auto electronSelectionCnt             = std::make_unique<ConstDataVector<xAOD::ElectronContainer>> (SG::VIEW_ELEMENTS);
  auto TagAndProbeElectronSelectionCnt  = std::make_unique<ConstDataVector<xAOD::ElectronContainer>> (SG::VIEW_ELEMENTS);
  
  SG::ReadHandle<xAOD::ElectronContainer> electronsCnt = SG::makeHandle(m_electronCntSgKey,ctx);

  if (!electronsCnt.isValid()) {
    ATH_MSG_ERROR("Electron container is not valid!");
    return StatusCode::FAILURE;
  }
   
  // Pre-selection of electrons
  for (auto el : *electronsCnt){
    if (std::abs(el->eta()) > m_elecEtaCut){ // apply electron |eta| cut
      continue;
    }
    if (trackSelectionElectrons(el, primVertexCnt, ei) == true){
      if (eOverPElectron(el) == true){ // if 0.7 < E/p < 1.4
        electronSelectionCnt->push_back( el );
      }
    }
    else continue;
  }

  if (m_doElecSelectByTrackOnly){ // do not proceed to T&P
    ATH_CHECK (evtStore()->record( electronSelectionCnt.release(), "MySelectedElectrons"));
    return StatusCode::SUCCESS;
  }

  if (electronSelectionCnt->size() < 2){
    ATH_MSG_DEBUG("(TagAndProbe) Not enough Electrons for T&P (nElectrons="<<electronSelectionCnt->size() << "). Storing "<<electronSelectionCnt->size()<<" electron on MyTagAndProbeElectrons container. This will not be dumped!");
    ATH_CHECK (evtStore()->record( TagAndProbeElectronSelectionCnt.release(), "MyTagAndProbeElectrons"));
    return StatusCode::SUCCESS;
  }

  // ## Loop over electron container in search of Zee pairs ##
    for (const xAOD::Electron *elTag : *electronSelectionCnt){
      
    if (! isTagElectron(elTag)) continue;
    ATH_MSG_DEBUG("(TagAndProbe) Electron is a Tag!");

    for (const xAOD::Electron *elProbe : *electronSelectionCnt){
      if (elTag==elProbe) continue;

      //check for charge (opposite charges for Zee)
      if ( elTag->charge() == elProbe->charge() ) continue;      
      ATH_MSG_DEBUG ("(TagAndProbe) Electron Pair Charges are opposite! Good.");

      //check for Zee mass    
      TLorentzVector el1;
      TLorentzVector el2;
      el1.SetPtEtaPhiE(elTag->pt(), elTag->trackParticle()->eta(), elTag->trackParticle()->phi(), elTag->e());
      el2.SetPtEtaPhiE(elProbe->pt(), elProbe->trackParticle()->eta(), elProbe->trackParticle()->phi(), elProbe->e());

      float tpPairMass = (el1 + el2).M(); // mass of the electron pair
      ATH_MSG_DEBUG ("(TagAndProbe) Electron Pair mass: " << tpPairMass << ". Min: "<< m_minZeeMassTP*GeV << ". Max: " << m_maxZeeMassTP*GeV);
      if (!((tpPairMass > m_minZeeMassTP*GeV) && (tpPairMass < m_maxZeeMassTP*GeV))){
        ATH_MSG_DEBUG ("(TagAndProbe) Electron pair not in Z mass window.");
        continue;
      }
      else{
        ATH_MSG_INFO ("(TagAndProbe) Electron-positron pair are in Z mass window.");
        // test for goodProbeElectron
        if (!isGoodProbeElectron(elProbe)){
          ATH_MSG_DEBUG (" Probe electron not passed in Goodnes of Probe test.");
          continue;
        }
        ATH_MSG_INFO("(TagAndProbe) Electron is a good probe.");
        ATH_MSG_INFO ("Tag and probe pair found.");

        ATH_MSG_INFO ("TAG: pt="<< elTag->pt() << " e="<< elTag->e());
        ATH_MSG_INFO ("PRO: pt="<< elProbe->pt() << " e="<< elProbe->e());
        ATH_MSG_INFO ("Zee: M="<< tpPairMass << " E="<< (el1 + el2).E());
        m_zee_M->push_back((el1 + el2).M());
        m_zee_E->push_back((el1 + el2).E());
        m_zee_pt->push_back((el1 + el2).Pt());
        m_zee_px->push_back((el1 + el2).Px());
        m_zee_py->push_back((el1 + el2).Py());
        m_zee_pz->push_back((el1 + el2).Pz());
        m_zee_T->push_back((el1 + el2).T());
        m_zee_deltaR->push_back(el1.DeltaR(el2));

        // ****************************************************************
        // store electrons in a container to dump their clusters and cells.
        TagAndProbeElectronSelectionCnt->push_back( (elTag) );
        // ****************************************************************
      }      
    } //end-loop probe electrons    
  } //end-loop tag electrons

  ATH_CHECK (evtStore()->record( TagAndProbeElectronSelectionCnt.release(), "MyTagAndProbeElectrons"));
  return StatusCode::SUCCESS;
}

StatusCode EventReaderAlg::dumpTruthParticle(SG::ReadHandle<xAOD::ElectronContainer> &electronSelectionCnt, SG::ReadHandle<xAOD::TruthParticleContainer> &truthParticleCnt){
  ATH_MSG_INFO("Dumping Truth Particle...");

  auto elecCandidates = std::make_unique<ConstDataVector<xAOD::TruthParticleContainer>> (SG::VIEW_ELEMENTS);

  if (truthParticleCnt->size() == 0) ATH_MSG_INFO("truthParticleCnt is empty!");
  else{
    for (const xAOD::Electron *recElectron : *electronSelectionCnt){

      const xAOD::TruthParticle *truth = xAOD::TruthHelpers::getTruthParticle(*recElectron);
      elecCandidates->push_back( truth ); 

      ATH_MSG_INFO("(TruthParticle) Reco pt="<< recElectron->pt()<<" , Truth pt= "<< truth->pt());
      }
    }

    if (elecCandidates->size() > 0)  ATH_MSG_INFO("(TruthParticle) Size of elecCandidates is "<< elecCandidates->size()<<" !");

    for (const xAOD::TruthParticle *elecSelected : *elecCandidates){
      const xAOD::TruthVertex* vertex = elecSelected->prodVtx();

      TLorentzVector elecCandidateLorentz;
      elecCandidateLorentz.SetPtEtaPhiE(elecSelected->pt(),elecSelected->eta(),elecSelected->phi(),elecSelected->e()); 

      m_mc_vert_x->push_back(vertex->x());  // vertex displacement
      m_mc_vert_y->push_back(vertex->y());  // vertex displacement
      m_mc_vert_z->push_back(vertex->z());  // vertex displacement in beam direction
      m_mc_vert_time->push_back(vertex->t());  // vertex time
      m_mc_vert_perp->push_back(vertex->perp());  // Vertex transverse distance from the beam line
      m_mc_vert_eta->push_back(vertex->eta());  // Vertex pseudorapidity
      m_mc_vert_phi->push_back(vertex->phi());  // Vertex azimuthal angle
      m_mc_vert_barcode->push_back(HepMC::barcode(vertex));  // FIXME barcode-based
      m_mc_vert_status->push_back(HepMC::status(vertex));

      m_mc_part_energy->push_back(elecSelected->e());
      m_mc_part_pt->push_back(elecSelected->pt());
      m_mc_part_pdgId->push_back(elecSelected->pdgId());
      m_mc_part_m->push_back(elecSelected->m());
      m_mc_part_phi->push_back(elecSelected->phi());
      m_mc_part_eta->push_back(elecSelected->eta());
    }
  ATH_CHECK (evtStore()->record( elecCandidates.release(), "MyMatchedTruthElectrons"));
  return StatusCode::SUCCESS;
  
}

StatusCode EventReaderAlg::dumpClusterCells(const xAOD::CaloCluster *cl, int clusIndex, const EventContext& ctx){

  SG::ReadHandle<LArDigitContainer>       LarDigCnt  = SG::makeHandle(m_larDigitCntSgKey,ctx);
  if(!LarDigCnt.isValid()) ATH_MSG_ERROR("LArDigitContainer is not a valid container!");
  SG::ReadHandle<LArRawChannelContainer>  LArRawCnt  = SG::makeHandle(m_larRawChCntSgKey,ctx);
  if(!LArRawCnt.isValid()) ATH_MSG_ERROR("LArRawChannelContainer is not a valid container!");
  
  // calorimeter level
  std::bitset<200000>       larClusteredDigits;
  std::vector<size_t>       caloHashMap;
  std::vector<HWIdentifier> channelHwidInClusterMap; //unique for any readout in the ATLAS
  std::vector<int>          cellIndexMap;
  std::vector<float>        channelIndexMap;
  std::vector<int>          cellLayerMap;
  // cell level
  std::vector<double>       clusCellEtaMap;//unused
  std::vector<double>       clusCellPhiMap; //unused
  // channel level
  std::vector<double>       channelEnergyInClusterMap;
  std::vector<double>       channelTimeInClusterMap;
  std::vector<double>       channelEtaInClusterMap;
  std::vector<double>       channelPhiInClusterMap;
  std::vector<float>        channelEffectiveSigmaClusterMap;
  std::vector<float>        channelNoiseClusterMap;

  std::vector<int>          channelCaloRegionMap;
  std::vector<bool>         badChannelMap;

  // cluster
  double  clusEta           = cl->eta();
  double  clusPhi           = cl->phi();
  double  clusEt            = cl->et();
  double  clusPt            = cl->pt();
  double  clusTime          = cl->time();

  ATH_MSG_DEBUG (" (Cluster) Cluster: "<< clusIndex <<", numberCells: " << cl->numberCells() << ", e = " << cl->e() << ", time = "<< clusTime << ", pt = " << cl->pt() << " , eta = " << cl->eta() << " , phi = " << cl->phi());

  // loop over cells in cluster
  auto itrCellsBegin = cl->cell_begin();
  auto itrCellsEnd = cl->cell_end();
  int cellNum = 0; //cell number just for printing out

  for (auto itCells=itrCellsBegin; itCells != itrCellsEnd; ++itCells){
    const CaloCell* cell = (*itCells); //get the caloCells
    if (cell){ // check for empty clusters
      Identifier cellId = cell->ID();

      int caloRegionIndex = getCaloRegionIndex(cell); // custom caloRegionIndex based on caloDDE

      double  eneCell   = cell->energy();
      double  timeCell  = cell->time();
      double  etaCell   = cell->eta();
      double  phiCell   = cell->phi();          
      int     gainCell  = cell->gain(); // CaloGain type
      int     layerCell = m_calocell_id->calo_sample(cell->ID());
      bool    badCell   = cell->badcell(); // applied to LAR channels. For Tile, we use TileCell* tCell->badch1() or 2.
      bool    isTile    = cell->caloDDE()->is_tile();
      bool    isLAr     = cell->caloDDE()->is_lar_em();
      bool    isLArFwd  = cell->caloDDE()->is_lar_fcal();
      bool    isLArHEC  = cell->caloDDE()->is_lar_hec();
      double  detaCell  = cell->caloDDE()->deta(); //delta eta - granularity
      double  dphiCell  = cell->caloDDE()->dphi(); //delta phi - granularity
      float   effSigma  = m_noiseCDO->getEffectiveSigma(cell->ID(), cell->gain(), cell->energy()); // effective sigma of cell related to its noise.
      float   cellNoise = m_noiseCDO->getNoise(cell->ID(), cell->gain()); // cell noise value

      IdentifierHash chidHash;
      HWIdentifier chhwid;
      size_t index;

      // ========= TileCal ==========
      if (isTile) continue;
    
      // ========= LAr ==========
      else if ((isLAr) || (isLArFwd) || (isLArHEC)) { //LAr
        chhwid = m_larCabling->createSignalChannelID(cellId);
        chidHash =  m_onlineLArID->channel_Hash(chhwid);
        index = (size_t) (chidHash);

        if (larClusteredDigits.test(index)) {
          ATH_MSG_ERROR (" ##### (Cluster) Error LAr: Conflict index position in Channel map. Position was already filled in this event. Skipping the cell of index: " << index << " and hwid: " << chhwid << ". Cell_E/t="<<cell->energy() << "/"<< cell->time() << ". Eta/Phi: "<< cell->eta() << "/" << cell->phi());
          continue;
          }
        if (badCell && m_noBadCells){
          ATH_MSG_INFO (" (Cluster)(LAr) Cell "<< cellNum <<" in cluster is a bad LAr channel! Skipping the cell.");
          continue;
        }
        larClusteredDigits.set(index);
        caloHashMap.push_back(index);
        cellIndexMap.push_back(m_c_cellIndexCounter);
        clusCellEtaMap.push_back(etaCell);
        clusCellPhiMap.push_back(phiCell);
        cellLayerMap.push_back(layerCell);
        channelIndexMap.push_back( m_c_cellIndexCounter ); 
        channelHwidInClusterMap.push_back(chhwid);
        channelTimeInClusterMap.push_back(timeCell);
        channelEnergyInClusterMap.push_back(eneCell);
        channelCaloRegionMap.push_back(caloRegionIndex);
        channelEffectiveSigmaClusterMap.push_back(effSigma);
        channelNoiseClusterMap.push_back(cellNoise);
        if (!m_noBadCells) badChannelMap.push_back(badCell);

        if (m_printCellsClus){ //optional to help debugging
          ATH_MSG_INFO (" (Cluster) in IsLAr: Cell (layer) "<< cellNum << " (" << layerCell <<") ID: " << cellId << "). HwId (B_EC/P_N/FeedTr/slot/ch) " <<  chhwid << " (" << m_onlineLArID->barrel_ec(chhwid) << "/" << m_onlineLArID->pos_neg(chhwid) << "/"<< m_onlineLArID->feedthrough(chhwid) << "/" << m_onlineLArID->slot(chhwid) << "/" << m_onlineLArID->channel(chhwid) << ") . Index: " << index << ". ene " << eneCell << ". time " << timeCell << ". D_eta: " << detaCell << ". D_phi: " << dphiCell << " ):" << ". Eta/Phi: "<< cell->eta() << "/" << cell->phi());
        }
      } //end else LAr

      else {
        ATH_MSG_ERROR (" ####### (Cluster) ERROR ! No CaloCell region was found!");
        continue;
      }

      // ##############################
      //  CELL INFO DUMP (CLUSTER)
      // ##############################
      const double clusterDphiRaw = clusPhi - phiCell;      
      double cluster_dphi = 0.;

      if (clusterDphiRaw  < -pi) {
        cluster_dphi = twopi + clusterDphiRaw;
      }
      else if (clusterDphiRaw > pi) {
        cluster_dphi = clusterDphiRaw - twopi;
      }
      else {
        cluster_dphi = clusterDphiRaw;
      }

      m_c_clusterIndex_cellLvl->push_back(clusIndex);
      m_c_clusterCellIndex->push_back(m_c_cellIndexCounter);
      m_c_cellGain->push_back(gainCell);
      m_c_cellLayer->push_back(layerCell);
      m_c_cellEnergy->push_back(eneCell);
      m_c_cellTime->push_back(timeCell);
      m_c_cellEta->push_back(etaCell);
      m_c_cellPhi->push_back(phiCell);
      m_c_cellDEta->push_back(detaCell);
      m_c_cellDPhi->push_back(dphiCell);
      m_c_cellToClusterDEta->push_back(clusEta - etaCell);
      m_c_cellToClusterDPhi->push_back(cluster_dphi);
      m_c_cellRegion->push_back(caloRegionIndex);
      
      m_c_cellIndexCounter++;
      cellNum++;
    } // end if-cell is empty verification
  } // end loop at cells inside cluster

  // ##############################
  //  CLUSTER INFO DUMP (CLUSTER)
  // // ##############################
  m_c_clusterIndex->push_back(clusIndex);
  m_c_clusterEnergy->push_back(clusEt);
  m_c_clusterTime->push_back(clusTime);
  m_c_clusterEta->push_back(clusEta);
  m_c_clusterPhi->push_back(clusPhi);
  m_c_clusterPt->push_back(clusPt);

  // ##############################
  //  DIGITS CHANNEL INFO DUMP (CLUSTER)
  // ##############################
  // Loop over ALL channels in LAr Digit Container.
  // Dump only the channels inside the previous clusters

  // ========= LAr ==========
  if (larClusteredDigits.any()){    
    // -- LAr_Hits --
    if (m_isMC && m_doLArEMBHitsDump){
      ATH_MSG_DEBUG (" (Cluster-HITS) Dumping LAr EMB Hits...");

      m_ncell = m_calocell_id->calo_cell_hash_max();

      SG::ReadHandle<LArHitContainer> larHitCnt = SG::makeHandle(m_larEMBHitCntSgKey,ctx);
      if(!larHitCnt.isValid()) ATH_MSG_ERROR("LAr EMB Hit container is not valid!");

      for (unsigned int k=0 ; k < channelHwidInClusterMap.size() ; k++){
        double cell_total_ene       = 0.0;
        double cell_total_eneCalib  = 0.0;
        double cell_hit_tof         = -999.0;
        double hit_ene              = 0.;
        double hit_time             = 0.;
        float  hit_fSampCalib       = 1.;
        int hit_sampling            = 0;
        size_t hit_index1           = 0;
        bool thereIsOneHitToDump    = false;
        std::vector<double>         hitEnergiesFromMap;
        std::vector<double>         hitTimesFromMap  ;
        
        for (const LArHit* hit : *larHitCnt){
          Identifier hit_cellID         = hit->cellID();
          HWIdentifier hit_hwid         = m_larCabling->createSignalChannelID(hit_cellID);
          IdentifierHash hit_idHash     = m_onlineLArID->channel_Hash(hit_hwid);
          hit_index1                    = (size_t) (hit_idHash);//(m_calocell_id->calo_cell_hash(hit_cellID));          

          if (larClusteredDigits.test(hit_index1)){
            if (channelHwidInClusterMap[k]==hit_hwid){
          
              hit_ene                   = hit->energy();
              hit_time                  = hit->time();
              hit_sampling              = m_calocell_id->calo_sample(hit_cellID);

              if (hit_index1 < m_ncell && fabs(hit_time) < 25.){ // time < 25 ns (current BC)
                hitEnergiesFromMap.push_back(hit_ene);
                hitTimesFromMap.push_back(hit_time);
                hit_fSampCalib = m_fSampl->FSAMPL(hit_cellID);                
                thereIsOneHitToDump = true;

                ATH_MSG_DEBUG ("(HITS) Match in "<< k <<"/"<< channelHwidInClusterMap.size() <<"! Sampling "<< hit_sampling << ",  Cell (clus): "<< channelIndexMap[k] <<" ("<< clusIndex <<"), ID (mapHwid / hwid / hash ): "<< hit_cellID << " ("<< channelHwidInClusterMap[k] << " / "<< hit_hwid << " / "<< hit_index1 <<"), hitEne: "<< hit_ene <<", hitTime: "<< hit_time << ", m_fSampl: "<< hit_fSampCalib <<". eta="<< clusCellEtaMap[k]<< ", phi="<<clusCellPhiMap[k]<<".");             

              }// end-if caloHashMap matches
            }// end-if cluster map has this hit_index 
          }// end-if current bc test
        }// end-if current bc test

        if (thereIsOneHitToDump){
          for (auto hitE : hitEnergiesFromMap){
            cell_total_ene      += hitE;
            cell_total_eneCalib += hitE / hit_fSampCalib; //calib energy from hits
          }
          cell_hit_tof = *std::min_element(hitTimesFromMap.begin(), hitTimesFromMap.end());
          
          m_hits_energy->push_back(cell_total_ene);
          m_hits_time->push_back(cell_hit_tof);
          m_hits_sampling->push_back(hit_sampling);
          m_hits_clusterIndex_chLvl->push_back(clusIndex);
          m_hits_clusterChannelIndex->push_back(channelIndexMap[k]);
          m_hits_hash->push_back(hit_index1);
          m_hits_sampFrac->push_back(hit_fSampCalib);
          m_hits_energyConv->push_back(cell_total_eneCalib);
          m_hits_cellEta->push_back(clusCellEtaMap[k]);
          m_hits_cellPhi->push_back(clusCellPhiMap[k]);
        }
        else{
          ATH_MSG_DEBUG("(HITS) Hit from cell "<< k <<"/"<< channelHwidInClusterMap.size() <<" not found! Sampling "<< hit_sampling << ",  Cell (clus): "<< channelIndexMap[k] <<" ("<< clusIndex <<"), mapHwid: "<< channelHwidInClusterMap[k]<<". eta="<< clusCellEtaMap[k]<< ", phi="<<clusCellPhiMap[k]<<".");
        }
        hitEnergiesFromMap.clear();
        hitTimesFromMap.clear();
        
      }// end for loop over caloHashMaps
    }// end-if isMC and dumpHits 


    // -- LAr_Digits --
    ATH_MSG_DEBUG (" (Cluster) Dumping LAr Digits...");

    for (const LArDigit* dig : *LarDigCnt) {
      HWIdentifier channelID  = dig->channelID();
      IdentifierHash idHash   = m_onlineLArID->channel_Hash(channelID);
      size_t index = (size_t) (idHash);

      if (larClusteredDigits.test(index)) {
        for (unsigned int k=0 ; k < channelHwidInClusterMap.size() ; k++){
          if (channelHwidInClusterMap[k]==channelID){
            
            // digits
            std::vector<short> larDigitShort = dig->samples();
            std::vector<float> larDigit( larDigitShort.begin(), larDigitShort.end() ); // get vector conversion from 'short int' to 'float'
            m_c_channelDigits->push_back(larDigit);

            const HWIdentifier   digId           = dig->hardwareID();
            const int            digGain         = dig->gain();
            auto                 ofc_a           = m_ofcs->OFC_a(digId, digGain);
            auto                 ofc_b           = m_ofcs->OFC_b(digId, digGain);

            std::vector<double>   ofca, ofcb, shape, shapeDer;
            const IdentifierHash& dig_cell_hashId = m_onlineLArID->channel_Hash(channelID);
            for (size_t k=0; k < ofc_a.size(); k++) ofca.push_back((double) ofc_a[k]);
            for (size_t k=0; k < ofc_b.size(); k++) ofcb.push_back((double) ofc_b[k]);

            const auto&          adc2mev            = m_adc2MeVs->ADC2MEV(digId, digGain);
            const float          pedLAr             = m_peds->pedestal(digId, digGain);
            const auto&          minBiasLAr         = m_minBiasAvgs->minBiasAverage(digId);
            
              // Remark: Looks like the EC does not have this rescale in DB (?)            
            if (!m_isMC){
              auto                 sig_shape       = m_shapes->Shape(digId, digGain);
              auto                 sig_shapeDer    = m_shapes->ShapeDer(digId, digGain);
              for (size_t k=0; k < sig_shape.size(); k++) shape.push_back((double) sig_shape[k]);
              for (size_t k=0; k < sig_shapeDer.size(); k++) shapeDer.push_back((double) sig_shapeDer[k]);

              float                offl_EneRescaler   = 1.0; // if the channel is outside of hash_id limits from DB, set scale to 1.0 (energy remains unchanged).
              const auto&          offl_hv            = m_oflHVCorr->HVScaleCorr(digId);
              if (dig_cell_hashId < m_EneRescaler->getNChans()){ // if the channel is INSIDE the hash_id limit, get the data from DB.
                offl_EneRescaler   = m_EneRescaler->getData(dig_cell_hashId,0,0); // IMPORTANT REMARK: There was an update in Athena v23, compared to v22. There is a new <T> for getData() instance, which no longer needs the 2nd and 3rd accessor (adc, gain).
              }
              m_c_channelDSPThreshold->push_back(m_run2DSPThresh->tQThr(digId));
              m_c_channelOfflEneRescaler->push_back(offl_EneRescaler);
              m_c_channelOfflHVScale->push_back(offl_hv);
              m_c_channelShape->push_back(shape);
              m_c_channelShapeDer->push_back(shapeDer);
            }
            // Cell / readout data
            m_c_channelEnergy->push_back(channelEnergyInClusterMap[k]);
            m_c_channelTime->push_back(channelTimeInClusterMap[k]);
            m_c_channelLayer->push_back(cellLayerMap[k]);
            m_c_channelEffectiveSigma->push_back(channelEffectiveSigmaClusterMap[k]);
            m_c_channelNoise->push_back(channelNoiseClusterMap[k]);
            if (!m_noBadCells) m_c_channelBad->push_back(badChannelMap[k]);
            // Calibration constants         
            m_c_channelOFCa->push_back(ofca);
            m_c_channelOFCb->push_back(ofcb);
            m_c_channelOFCTimeOffset->push_back(m_ofcs->timeOffset(digId,digGain));
            m_c_channelADC2MEV0->push_back(adc2mev[0]);
            m_c_channelADC2MEV1->push_back(adc2mev[1]);
            m_c_channelPed->push_back(pedLAr);
            m_c_channelMinBiasAvg->push_back(minBiasLAr);

            // Channel info
            int barrelEc = m_onlineLArID->barrel_ec(channelID);
            int posNeg = m_onlineLArID->pos_neg(channelID);
            int feedThr = m_onlineLArID->feedthrough(channelID); // feedthrough - cabling interface from cold and hot side. in barrel, there are 2 feedThr for each crate.
            int slot = m_onlineLArID->slot(channelID);
            int chn = m_onlineLArID->channel(channelID);      
            std::vector<int> chInfo {barrelEc, posNeg, feedThr, slot, chn } ; //unite info
            m_c_channelChInfo->push_back(chInfo); //add to the list of cell info (both calo)

            // important indexes
            m_c_clusterChannelIndex->push_back(channelIndexMap[k]);//each LAr cell is an individual read out.
            m_c_clusterIndex_chLvl->push_back(clusIndex);  // what roi this cell belongs at the actual event: 0, 1, 2 ... 
            m_c_channelHashMap->push_back(index); // online id hash for channel
            m_c_channelChannelIdMap->push_back(channelID.get_identifier32().get_compact());


            if (m_printCellsClus){

              ATH_MSG_INFO ("(Cluster) In DumpLAr Digits: ReadOutIndex "<< channelIndexMap[k]  << ". HwId " <<  channelID);
              ATH_MSG_INFO ("(Cluster) In DumpLAr Digits: ReadOutIndex "<< channelIndexMap[k]  << " HwId.get_compact() " << channelID.get_compact());
              ATH_MSG_INFO ("(Cluster) In DumpLAr Digits: ReadOutIndex "<< channelIndexMap[k]  <<  " HwId.get_identifier32().get_compact() " << channelID.get_identifier32().get_compact());

              ATH_MSG_INFO("(Cluster) LAr OFC timeOffset: "<< m_ofcs->timeOffset(digId, digGain)  <<", dig->hardwareID: " << digId << ", dig->channelID: "<< channelID);
              ATH_MSG_INFO("\tOFCa ("<< ofca.size()<<"): ["<< ofca[0] <<", " << ofca[1] <<", " << ofca[2]<<", "  << ofca[3] <<"]");
              ATH_MSG_INFO("\tOFCb ("<< ofcb.size()<<"): ["<< ofcb[0] <<", " << ofcb[1] <<", " << ofcb[2]<<", "  << ofcb[3] <<"]");
              
              ATH_MSG_DEBUG ("(Cluster) In DumpLAr Digits: ReadOutIndex "<< channelIndexMap[k]  << ". HwId (B_EC/P_N/FeedTr/slot/ch) " <<  channelID << " (" << barrelEc << "/" << posNeg << "/"<< feedThr << "/" << slot << "/" << chn << ". Dig (float): " << larDigit);

            }
          } //end-if caloHashMap matches
        } //end for loop over caloHashMaps
      } // end-if clustBits Test
    } //end loop over LAr digits container
  } //end-if larClustBits is not empty

  // ##############################
  //  RAW CHANNEL INFO DUMP (CLUSTER)
  // ##############################

  // ========= LAr ==========
  if (larClusteredDigits.any()) {
    ATH_MSG_INFO ("(Cluster) Dumping LAr Raw Channel...");

    for (const LArRawChannel& LArChannel : *LArRawCnt){

      HWIdentifier    channelID   = LArChannel.channelID();
      IdentifierHash  chHwidHash  = m_onlineLArID->channel_Hash(channelID);

      size_t index = static_cast<size_t>(chHwidHash);

      if (larClusteredDigits.test(index)){ //if this channel index is inside 
        for (unsigned int k=0 ; k < channelHwidInClusterMap.size() ; k++){  //loop over the vector of hashIndex, and verify if the cell index match.
          if (channelHwidInClusterMap[k] == channelID){
            
            //RawCh info
            int rawEnergy        = LArChannel.energy(); // energy in MeV (rounded to integer)
            int rawTime          = LArChannel.time();    // time in ps (rounded to integer)            
            uint16_t rawQuality  = LArChannel.quality(); // quality from pulse reconstruction
            int provenance       = static_cast<int>(LArChannel.provenance()); // its uint16_t
            float rawEnergyConv  = static_cast<float>(rawEnergy); 
            float rawTimeConv    = static_cast<float>(rawTime);
            float rawQualityConv = static_cast<float>(rawQuality);

            // Channel info
            int barrelEc = m_onlineLArID->barrel_ec(channelID);
            int posNeg = m_onlineLArID->pos_neg(channelID);
            int feedThr = m_onlineLArID->feedthrough(channelID); // feedthrough - cabling interface from cold and hot side. in barrel, there are 2 feedThr for each crate.
            int slot = m_onlineLArID->slot(channelID);
            int chn = m_onlineLArID->channel(channelID);      
            std::vector<int> chInfo {barrelEc, posNeg, feedThr, slot, chn } ; //unite info
            m_c_rawChannelChInfo->push_back(chInfo); //add to the list of cell info (both calo)

            if ( (rawEnergy != rawEnergyConv) || (rawTime != rawTimeConv) || (rawQuality != rawQualityConv) ){
              ATH_MSG_ERROR (" ###### (Cluster) LAR RAW CHANNEL: Value conversion from int to float of amplitude, time or quality (uint16_t) had changed its actual value !!!");
            }
            m_c_rawChannelAmplitude->push_back(rawEnergyConv);
            m_c_rawChannelTime->push_back(rawTimeConv);
            m_c_rawChannelLayer->push_back(cellLayerMap[k]);
            m_c_rawChannelQuality->push_back(rawQualityConv);
            m_c_rawChannelProv->push_back(provenance);
            m_c_rawChannelPed->push_back(-8888); // masked LAr
            if (!m_isMC){
              m_c_rawChannelDSPThreshold->push_back(m_run2DSPThresh->tQThr(channelID));
            }

            // important indexes
            m_c_clusterRawChannelIndex->push_back(channelIndexMap[k]);
            m_c_clusterIndex_rawChLvl->push_back(clusIndex); // what roi this ch belongs
            m_c_rawChannelIdMap->push_back(channelID.get_identifier32().get_compact());

            if (m_printCellsClus){ //optional to help debugging

              HWIdentifier hardwareID = LArChannel.hardwareID();

              if (!m_isMC){
                ATH_MSG_INFO ("(Cluster) In DumpLAr Raw "<< channelIndexMap[k] <<": hardwareID (B_EC/P_N/feedThr/slot/chn): " << hardwareID << "(" << barrelEc << "/" << posNeg << "/" << feedThr << "/" << slot << "/" << chn << ")" << ". Energy: " << rawEnergyConv << ". Time: " << rawTimeConv << ". Provenance: " << provenance << ". Quality: " << rawQualityConv <<" ecut (DSPThr) = " << m_run2DSPThresh->tQThr(channelID));
              }
              else{
                ATH_MSG_INFO ("(Cluster) In DumpLAr Raw "<< channelIndexMap[k] <<": hardwareID (B_EC/P_N/feedThr/slot/chn): " << hardwareID << "(" << barrelEc << "/" << posNeg << "/" << feedThr << "/" << slot << "/" << chn << ")" << ". Energy: " << rawEnergyConv << ". Time: " << rawTimeConv << ". Provenance: " << provenance << ". Quality: " << rawQualityConv);
              }
              
            } // end-if optional cell print
          } // end-if hwid match rawChID
        } // end loop over HWIDClusterMap
      } // end-if clust.test
    } //end loop LArRawChannelContainer
  } // end-if clustBits have Cells

  larClusteredDigits.reset();

  caloHashMap.clear();

  channelHwidInClusterMap.clear();
  cellIndexMap.clear();
  cellLayerMap.clear();
  clusCellEtaMap.clear();
  clusCellPhiMap.clear();
  channelIndexMap.clear();
  channelEnergyInClusterMap.clear();
  channelTimeInClusterMap.clear();
  channelEtaInClusterMap.clear();
  channelPhiInClusterMap.clear();
  channelEffectiveSigmaClusterMap.clear();
  channelNoiseClusterMap.clear();
  channelCaloRegionMap.clear();
  if (!m_noBadCells) badChannelMap.clear();
  
  ATH_MSG_INFO ("  (Cluster) RawChannel: "<< m_c_rawChannelAmplitude->size() <<" channels dumped, from " << m_c_cellEta->size() <<" cluster cells. ");
  ATH_MSG_INFO ("  (Cluster) Digits: "<< m_c_channelDigits->size() <<" channels dumped, out of " << m_c_rawChannelAmplitude->size() <<" raw channel cluster channels. ");
  if (m_c_channelDigits->size() == m_c_rawChannelAmplitude->size()){
    ATH_MSG_INFO ("  (Cluster) ALL Digits from the cluster were dumped successfully!");}
  else{
    ATH_MSG_INFO ("  (Cluster) The digits from "<< (m_c_rawChannelAmplitude->size() - m_c_channelDigits->size()) <<" channels are missing!");}


  return StatusCode::SUCCESS;
}


StatusCode EventReaderAlg::finalize() {
  ATH_MSG_DEBUG ("Finalizing " << name() << "...");
  m_secondTree->Fill();

  return StatusCode::SUCCESS;
}
