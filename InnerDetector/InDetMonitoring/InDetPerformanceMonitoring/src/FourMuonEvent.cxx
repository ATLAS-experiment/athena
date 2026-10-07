/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//==================================================================================
// Include files...
//==================================================================================

// This files header
#include "InDetPerformanceMonitoring/FourMuonEvent.h"

// Package Headers
#include "InDetPerformanceMonitoring/PerfMonServices.h"

// ATLAS headers
#include "AthenaKernel/getMessageSvc.h"

//==================================================================================
// Public Methods
//==================================================================================

FourMuonEvent::FourMuonEvent()
{
  m_xSampleName = "FourMuon";

  m_container = PerfMonServices::MUON_COLLECTION; //PerfMonServices::ELECTRON_COLLECTION

  m_doDebug = false;
  m_workAsFourMuons = false;
  m_workAsFourElectrons = false;
  m_workAsFourLeptons = true;

  // Setup the muon tags
  m_uMuonTags   = 4;
  m_LeadingMuonPtCut = 20.;   
  m_SecondMuonPtCut = 15.;
  m_MassWindowLow = 10.0;
  m_MassWindowHigh = 125.0;
  m_deltaXYcut = 0.1; // in mm
  m_Z0GapCut = 5.0; // in mm
  m_SelectMuonByIso = true;
  m_SelectMuonByIP = true;
  m_eventCount = 0;
  m_acceptedEventCount = 0;
  m_acceptedMuonCount = 0;
  m_acceptedElecCount = 0;

  m_msgStream =  new MsgStream(Athena::getMessageSvc(), "InDetPerformanceMonitoring" );
}

//==================================================================================
FourMuonEvent::~FourMuonEvent()
{
  delete m_msgStream;
}

//==================================================================================
void FourMuonEvent::Init()
{
  (*m_msgStream) << MSG::DEBUG << " * FourMuonEvent::Init * START *" << endmsg;
  
  (*m_msgStream) << MSG::DEBUG << " * FourMuonEvent::Init * initializing muon selector *" << endmsg; 
  m_xMuonID.Init();
  (*m_msgStream) << MSG::DEBUG << " * FourMuonEvent::Init * initializing electron selector *" << endmsg; 
  m_xElecID.Init();

  if (m_workAsFourMuons)     {(*m_msgStream) << MSG::INFO << " * FourMuonEvent::Init * working mode: 4 muons"     << endmsg; }
  if (m_workAsFourElectrons) {(*m_msgStream) << MSG::INFO << " * FourMuonEvent::Init * working mode: 4 electrons" << endmsg; }
  if (m_workAsFourLeptons)   {(*m_msgStream) << MSG::INFO << " * FourMuonEvent::Init * working mode: 4 leptons"   << endmsg; }

  PARENT::Init();
  (*m_msgStream) << MSG::DEBUG << " * FourMuonEvent::Init * Completed * " << endmsg; 

  return;
}

//==================================================================================
bool FourMuonEvent::Reco()
{
  m_eventCount++;
  (*m_msgStream) << MSG::DEBUG << " * FourMuonEvent::Reco * STARTING ** New event ** eventCount " << m_eventCount << endmsg;
  
  // Clear out the previous events record.
  this->Clear();
  
  // if muons are requested 
  if (m_workAsFourMuons || m_workAsFourLeptons) {
    (*m_msgStream) << MSG::DEBUG << " * FourMuonEvent::Reco * retrieving xAOD::MuonContainer " << m_container << " of eventCount " << m_eventCount << std::endl; 
    
    const xAOD::MuonContainer* pxMuonContainer = PerfMonServices::getContainer<xAOD::MuonContainer>( m_container );
    
    // check if muon container does exist
    if (pxMuonContainer != nullptr) { 
      (*m_msgStream) << MSG::DEBUG << " * FourMuonEvent::Reco * eventCount " << m_eventCount 
		     << " track list has "<< pxMuonContainer->size() 
		     << " combined muons in container " << m_container 
		     << " container name: " << PerfMonServices::getContainerName ( m_container ) 
		     << endmsg; 

      xAOD::MuonContainer::const_iterator xMuonItr  = pxMuonContainer->begin();
      xAOD::MuonContainer::const_iterator xMuonItrE  = pxMuonContainer->end();
      int theCount = 0;
      while ( xMuonItr != xMuonItrE ){ // start loop on muons
	const xAOD::Muon* pxCMuon = *xMuonItr;
	theCount++;
	// Apply muon cuts
	if ( m_xMuonID.passSelection( pxCMuon)) {
	  RecordMuon( pxCMuon );
	  (*m_msgStream) << MSG::DEBUG <<  " * FourMuonEvent::Reco ** muon " << theCount << " is accepted " << endmsg;
	}
	++xMuonItr;
      } // end loop on muons
      
      // ordering of muons
      this->OrderMuonList();      
    } // end muon container exists
    if (!pxMuonContainer) {
      std::cout << " * FourMuonEvent::Reco * Can't retrieve combined muon collection (container: " << m_container <<") " << std::endl;
      return false;
    } // end muon container does not exist
  } // end requesting muons
  
  //  
  // Electron selection (in case electrons are requested)
  if (m_workAsFourElectrons || m_workAsFourLeptons) {
    // Get the electron AOD container
    const xAOD::ElectronContainer* pxElecContainer = nullptr;
    if (m_doDebug){ std::cout << " * FourMuonEvent::Reco * retrieving xAOD::ElectronContainer " << PerfMonServices::ELECTRON_COLLECTION << std::endl; }
    pxElecContainer = PerfMonServices::getContainer<xAOD::ElectronContainer>( PerfMonServices::ELECTRON_COLLECTION );
    
    //pxElecContainer = evtStore()->retrieve( pxElecContainer, "Electrons" );
    
    if (pxElecContainer != nullptr) {
      (*m_msgStream) << MSG::DEBUG << " * FourMuonEvent::Reco * retrieving xAOD::ElectronContainer SUCCESS. " 
		     << " Container name: " << PerfMonServices::getContainerName (PerfMonServices::ELECTRON_COLLECTION) 
		     << " size: " << pxElecContainer->size()
		     << endmsg;
      if (pxElecContainer->size() > 0 ){
	m_xElecID.PrepareElectronList (pxElecContainer);
	m_numberOfFullPassElectrons = m_xElecID.GetElectronCollectionSize();
	m_acceptedElecCount += m_numberOfFullPassElectrons;
      }    
    }
    else { // no pxElecContainer
      (*m_msgStream) << MSG::DEBUG << " * FourMuonEvent::Reco * retrieving xAOD::ElectronContainer -- FAILED -- eventcount: " << m_eventCount << endmsg;
    }
  } 
  
  /////////////////////
  // reached this point one has the list of muons and electrons in this event
  /////////////////////
  
  // now check if the particles in the event make them to satisfy the event selection
  if (m_workAsFourMuons) {
    m_passedFourMuonSelection = false; // unless the event has 4 muons, assume it is not good
    
    if (m_numberOfFullPassMuons == 4) {
      m_passedFourMuonSelection = this->ReconstructKinematicsNew();
      if (m_passedFourMuonSelection) m_passedFourMuonSelection = this->CheckMuonVertices();
      if (m_passedFourMuonSelection) m_passedFourMuonSelection = this->EventSelectionNew(ID);
      
      if (m_passedFourMuonSelection) {
	m_FourMuonInvMass = m_fInvariantMass[ID]; // store invariant mass
	if (m_doDebug) std::cout << " * FourMuonEvent::Reco * === Event " << m_eventCount << " is a GOOD 4-muon event " << std::endl;
      }
      else {
	if (m_doDebug) std::cout << " * FourMuonEvent::Reco * === Event " << m_eventCount << " FAILS the 4-muon event selection " << std::endl;
      }
    }  
  } // end of workAsFourMuons
  
  if (m_workAsFourElectrons) {
    if (m_doDebug) std::cout << " * FourMuonEvent::Reco * applying 4 electron selection " << std::endl;
    m_passedFourElectronSelection = false; // unless the event has 4 muons, assume it is not good
    
    if (m_numberOfFullPassElectrons == 4) {
      m_passedFourElectronSelection = this->ReconstructKinematicsNew();
      if (m_passedFourElectronSelection) m_passedFourElectronSelection = this->EventSelectionNew(ID);
      
      if (m_passedFourElectronSelection) {
	m_FourMuonInvMass = m_fInvariantMass[ID]; // store invariant mass
	if (m_doDebug) std::cout << " * FourMuonEvent::Reco * === Event " << m_eventCount << " is a GOOD 4-electrom event " << std::endl;
      }
      else {
	if (m_doDebug) std::cout << " * FourMuonEvent::Reco * === Event " << m_eventCount << " FAILS the 4-electron event selection " << std::endl;
      }
    }
    else {
      if (m_doDebug) std::cout << " * FourMuonEvent::Reco * === Event " << m_eventCount << " is not a 4-electron event " << std::endl;
    }
  } // end of workAsFourElectrons
  
  
  if (m_workAsFourLeptons) {
    if (m_doDebug) std::cout << " * FourMuonEvent::Reco * applying 4 lepton selection " << std::endl;
    m_passedFourLeptonSelection = false;
    
    bool enoughleptons = false;
    if (m_numberOfFullPassMuons == 4) enoughleptons = true;
    if (m_numberOfFullPassElectrons == 4) enoughleptons = true;
    if (m_numberOfFullPassMuons >= 2 && m_numberOfFullPassElectrons >= 2) enoughleptons = true;
    
    if ( enoughleptons) {
      if (m_doDebug) {
	std::cout << " * FourMuonEvent::Reco * Global statistics: Total number of accepted muons so far: " << m_acceptedMuonCount 
		  << "  &   electrons: " << m_acceptedElecCount  << " integrated over all events "
		  << std::endl;
	std::cout << " * FourMuonEvent::Reco * This event has " << m_numberOfFullPassMuons
		  << " muons & " << m_numberOfFullPassElectrons << " electrons --> try kinematics, vertex and event selection"
		  << std::endl;
      }
      m_passedFourLeptonSelection = this->ReconstructKinematicsNew();
      if (m_passedFourLeptonSelection && m_numberOfFullPassMuons >= 2) m_passedFourLeptonSelection = this->CheckMuonVertices();
      if (m_passedFourLeptonSelection) m_passedFourLeptonSelection = this->EventSelectionNew(ID);
      
      if (m_passedFourLeptonSelection) {
	m_FourMuonInvMass = m_fInvariantMass[ID]; // store invariant mass
	(*m_msgStream) << MSG::INFO << " * FourMuonEvent::Reco * === Event " << m_eventCount << " is a GOOD 4-lepton event with inv mass: " << m_FourMuonInvMass << endmsg;
      }
      else {
	(*m_msgStream) << MSG::DEBUG << " * FourMuonEvent::Reco * === Event " << m_eventCount << " -- FAILS -- the 4-lepton event selection " << endmsg;
      }
    }
    else {
      (*m_msgStream) << MSG::DEBUG << " * FourMuonEvent::Reco * 4lepton selection FAILURE. Not enough muons or electrons. Event has " << m_numberOfFullPassMuons
		     << " muons & " << m_numberOfFullPassElectrons << " electrons"
		     << endmsg;
    }
  }
  
  m_passedSelectionCuts = false; // assume event is not good, but check the selection according to the use case
  if (m_workAsFourMuons)     m_passedSelectionCuts = m_passedFourMuonSelection;
  if (m_workAsFourElectrons) m_passedSelectionCuts = m_passedFourElectronSelection;
  if (m_workAsFourLeptons)   m_passedSelectionCuts = m_passedFourLeptonSelection;
  
  if ( m_passedSelectionCuts) m_acceptedEventCount++;
  
  (*m_msgStream) << MSG::DEBUG << " * FourMuonEvent::Reco * COMPLETED *   Event has " << m_numberOfFullPassMuons << " muons & " 
		 << m_numberOfFullPassElectrons << " electrons. "
		 << "  So far: " << m_acceptedEventCount << " events were accepted out of " << m_eventCount << " tested" << endmsg;

  return m_passedSelectionCuts;
}

//==================================================================================
// Protected Methods
//==================================================================================
void FourMuonEvent::BookHistograms()
{
}

//==================================================================================
// Private Methods
//==================================================================================
bool FourMuonEvent::EventSelectionNew(ZTYPE eType)
{
  bool inputdebug = m_doDebug;
  //m_doDebug = true;

  bool eventisgood = true;
  (*m_msgStream) << MSG::DEBUG << " * FourMuonEvent::EventSelection( type= " << eType << ") ** started ** " << std::endl 
		 << "                                          event count: " << m_eventCount  << std::endl
		 << "                              m_NumberOfFullPassMuons: " << m_numberOfFullPassMuons << std::endl
		 << "                          m_NumberOfFullPassElectrons: " << m_numberOfFullPassElectrons 
		 << endmsg;
  
  
  // Depending on mode: require a minimum of electrons or muons
  if ( eventisgood && m_workAsFourMuons) {
    if (m_numberOfFullPassMuons < 4) {
      eventisgood = false;
      if(m_doDebug) {std::cout <<" * FourMuonEvent::EventSelection(" << eType << ") * Failing number of good muons == 4 :( " 
			       <<"  m_numberOfFullPassMuons= " <<  m_numberOfFullPassMuons << std::endl;
      }
    } 
  }
  
  if ( eventisgood && m_workAsFourElectrons) {
    if (m_numberOfFullPassElectrons < 4) {
      eventisgood = false;
      if(m_doDebug) {std::cout <<" * FourMuonEvent::EventSelection(" << eType << ") * Failing number of good electrons == 4 :( " 
			       <<"  m_numberOfFullPassElectrons= " <<  m_numberOfFullPassElectrons << std::endl;
      }
    } 
  }
  
  if ( eventisgood && m_workAsFourLeptons) {
    bool thisselection = false;
    if (m_numberOfFullPassMuons     == 4) thisselection = true;
    if (m_numberOfFullPassElectrons == 4) thisselection = true;
    if (m_numberOfFullPassMuons >= 2 && m_numberOfFullPassElectrons >= 2) thisselection = true;
    if (!thisselection) {
      if(m_doDebug) {std::cout <<" * FourMuonEvent::EventSelection(" << eType << ") * Failing number of good muons >= 2 && electrons >= 2 :( " 
			       <<"  m_numberOfFullPassMuons= " <<  m_numberOfFullPassMuons
			       <<"  m_numberOfFullPassElectrons= " <<  m_numberOfFullPassElectrons << std::endl;
      }
    }
    eventisgood = thisselection;
  } 

  //
  // momentum of the leptons 
  // loop over the muons and electrons and count how many pass the leadingPt and secondPt cut
  unsigned int npassleadingpt = 0;
  unsigned int npasssecondpt = 0; 
  if ( eventisgood && (m_workAsFourMuons || m_workAsFourLeptons) ) {
    if (m_numberOfFullPassMuons >= 2) { // electrons pt cuts if there are some electrons
      //for (unsigned int i=0; i < m_numberOfFullPassMuons; i++) {
      for (unsigned int i=0; i < NUM_MUONS; i++) {
	if (m_pxIDTrack[i] == nullptr) continue;
	if(m_doDebug) {std::cout <<" * FourMuonEvent::EventSelection(" << eType << ") * using muon " << i << " with pt: " << m_pxIDTrack[i]->pt() <<  std::endl;}
	if (m_pxIDTrack[i]->pt() > m_LeadingMuonPtCut*CLHEP::GeV) npassleadingpt++;
	if (m_pxIDTrack[i]->pt() > m_SecondMuonPtCut*CLHEP::GeV) npasssecondpt++;
      }
      if(m_doDebug) {std::cout <<" * FourMuonEvent::EventSelection(" << eType << ") * #muons with pt > leading pt: " << m_LeadingMuonPtCut*CLHEP::GeV << " = " << npassleadingpt << std::endl;}
      if(m_doDebug) {std::cout <<" * FourMuonEvent::EventSelection(" << eType << ") * #muons with pt > second  pt: " << m_SecondMuonPtCut*CLHEP::GeV << " = " << npasssecondpt << std::endl;}
      if (npassleadingpt == 0) { // at least 1 muon must pass the leading pt cut
	eventisgood = false;
	if(m_doDebug) {std::cout <<" * FourMuonEvent::EventSelection(" << eType << ") * Failing leading muon pt cut " << m_LeadingMuonPtCut*CLHEP::GeV << std::endl;}
      }
      if (npasssecondpt < m_numberOfFullPassMuons) { // all muons must pass the second pt cut
	eventisgood = false;
	if(m_doDebug) {std::cout <<" * FourMuonEvent::EventSelection(" << eType << ") * Failing second muon pt cut " << m_LeadingMuonPtCut*CLHEP::GeV << std::endl;}
      }
    }
  } 

  // four electrons case
  if ( eventisgood && (m_workAsFourElectrons || m_workAsFourLeptons) ) {
    npassleadingpt = 0;
    npasssecondpt = 0;
    if (m_numberOfFullPassElectrons >= 2) { // electrons pt cuts if there are some electrons
      for (unsigned int i=0; i < NUM_MUONS; i++) {
	if (m_pxELTrack[i] == nullptr) continue;
	if (m_doDebug) {std::cout <<" * FourMuonEvent::EventSelection(" << eType << ") * using electron " << i << " with pt: " << m_pxELTrack[i]->pt() << std::endl;}
	if (m_pxELTrack[i]->pt() > m_LeadingMuonPtCut*CLHEP::GeV) npassleadingpt++;
	if (m_pxELTrack[i]->pt() > m_SecondMuonPtCut*CLHEP::GeV) npasssecondpt++;
      }
      if(m_doDebug) {std::cout <<" * FourMuonEvent::EventSelection(" << eType << ") * #elecs with pt > leading pt: " << m_LeadingMuonPtCut*CLHEP::GeV << " = " << npassleadingpt << std::endl;}
      if(m_doDebug) {std::cout <<" * FourMuonEvent::EventSelection(" << eType << ") * #elecs with pt > second  pt: " << m_SecondMuonPtCut*CLHEP::GeV << " = " << npasssecondpt << std::endl;}
      if (npassleadingpt == 0) { // at least 1 electron must pass the leading pt cut
	eventisgood = false;
	if(m_doDebug) {std::cout <<" * FourMuonEvent::EventSelection(" << eType << ") * Failing leading electron pt cut " << m_LeadingMuonPtCut*CLHEP::GeV << std::endl;}
      }
      if (npasssecondpt < m_numberOfFullPassElectrons) { // all electrons must pass the second pt cut
	eventisgood = false;
	if(m_doDebug) {std::cout <<" * FourMuonEvent::EventSelection(" << eType << ") * Failing second electrons pt cut " << m_LeadingMuonPtCut*CLHEP::GeV << std::endl;}
      }
    } // electron pt cuts apply if there are some electrons
  }

  // Invariant mass window
  if (eventisgood) {
    if ( m_fInvariantMass[eType]  < m_MassWindowLow  ) {
      if(m_doDebug) {
	std::cout <<" * FourMuonEvent::EventSelection * Failing mass window low cut:  reco m= " << m_fInvariantMass[eType] << " > " <<  m_MassWindowLow << std::endl;
      }
      eventisgood = false;
    }
    if ( m_fInvariantMass[eType]  > m_MassWindowHigh ) {
      if(m_doDebug) {
	std::cout <<" * FourMuonEvent::EventSelection * Failing mass window high cut:  reco m= " << m_fInvariantMass[eType] << " > " <<  m_MassWindowHigh << std::endl;
      }
      eventisgood = false;
    }
  }

  // check on vertices 
  if (eventisgood) {
    bool vertexstatus = false;

    std::vector <float> vtxListX; // coordinates of the vertex
    std::vector <float> vtxListY;
    std::vector <float> vtxListZ;
    std::vector <int>   vtxNpart; // count how many particles are in vertex

    int noVertexCountMuon = 0;
    int noVertexCountElec = 0;
    //
    if ( m_workAsFourMuons ) { // till here we have the muon vertices list
      m_nVertex = vtxListX.size(); 
      // no point setting vertexstatus here, it will be overwritten before use 
      // check that the muons are not split into too many vertices is therefore redundant
      // no point setting vertexstatus here, it will be overwritten before use
      // noVertexCountMuon is zero here
      if (m_doDebug || true) {
	std::cout << " * FourMuonEvent::EventSelection(" << eType <<") * vertices ID of the muons = " << std::endl
		  << "                   mu- 1 " << m_muon_vtx[0] << "  pt: " << m_pxMUTrack[0]->pt() << std::endl
		  << "                   mu- 2 " << m_muon_vtx[1] << "  pt: " << m_pxMUTrack[1]->pt() << std::endl
		  << "                   mu+ 1 " << m_muon_vtx[2] << "  pt: " << m_pxMUTrack[2]->pt() << std::endl
		  << "                   mu+ 2 " << m_muon_vtx[3] << "  pt: " << m_pxMUTrack[3]->pt()
		  << std::endl;
      } // end debug      
    } // end m_workAsFourMuons

    if ( m_workAsFourElectrons ) { // till here we have the electrons vertices list
      m_nVertex = vtxListX.size(); 
      if (vtxListX.size()>0) vertexstatus = true;
      // vertexstatus value is overwritten before use if it is set here
      
	std::cout << " * FourMuonEvent::EventSelection(" << eType <<") * vertices ID of the electrons = " 
		  << "\n                   el- 1 " << m_elec_vtx[0] << "  pt: " << m_pxELTrack[0]->pt() << std::endl
		  << "\n                   el- 2 " << m_elec_vtx[1] << "  pt: " << m_pxELTrack[1]->pt() << std::endl
		  << "\n                   el+ 1 " << m_elec_vtx[2] << "  pt: " << m_pxELTrack[2]->pt() << std::endl
		  << "\n                   el+ 2 " << m_elec_vtx[3] << "  pt: " << m_pxELTrack[3]->pt() <<"\n";
    }
    
    if ( m_workAsFourLeptons ) { // till here we have the muons and electrons vertices list
      m_nVertex = vtxListX.size(); 
      if (vtxListX.size()>0) vertexstatus = true;
      // check that the electrons are not split into too many vertices
      // if (vtxListX.size() >= m_numberOfFullPassElectrons - 1) vertexstatus = false;
      // and allow no electron without vertex
      if (m_doDebug) {
	std::cout << " * FourMuonEvent::EventSelection(" << eType <<") * vertices in event = " << vtxListX.size() << std::endl;
	for (size_t imu=0; imu < NUM_MUONS; imu++) {
	  if (m_pxMUTrack[imu]) std::cout << "                   mu    " << m_muon_vtx[imu] << "  pt: " << m_pxMUTrack[imu]->pt() << std::endl;
	}
	for (size_t iel=0; iel < NUM_MUONS; iel++) {
	  if (m_pxELTrack[iel]) std::cout << "                   el    " << m_elec_vtx[iel] << "  pt: " << m_pxELTrack[iel]->pt() << std::endl;
	}
      }
    }
    //how temporary was this?
    vertexstatus = true; // Temporary fix to work on R22

    if(m_doDebug) {
      std::cout <<" * FourMuonEvent::EventSelection(" << eType <<")  * Number of vertex found = " << vtxListX.size() 
		<< "  and mu without vertex: " << noVertexCountMuon 
		<< "  and elec without vertex: " << noVertexCountElec << std::endl;
      for (unsigned int ivtx=0; ivtx < vtxListX.size(); ivtx++) {
	std::cout << "       vtx[" << ivtx << "]= "
		  << "( " << vtxListX.at(ivtx)
		  << ", " << vtxListY.at(ivtx)
		  << ", " << vtxListZ.at(ivtx)
		  << ")  nparticles: " << vtxNpart.at(ivtx)
		  << std::endl;
      }
    }
    

    eventisgood = vertexstatus;
  }

  //
  if(m_doDebug){  std::cout <<" * FourMuonEvent::EventSelection(" << eType << ") ** completed ** result= " << eventisgood << std::endl;}

  m_doDebug = inputdebug;

  return eventisgood;
}
//==================================================================================
bool FourMuonEvent::EventSelection(ZTYPE eType)
{
  if(m_doDebug){  std::cout <<" * FourMuonEvent::EventSelection(" << eType << ") ** started ** " << std::endl 
			    << "                                            event count: " << m_eventCount  << std::endl
			    << "                                m_NumberOfFullPassMuons: " << m_numberOfFullPassMuons << std::endl
			    << "                            m_NumberOfFullPassElectrons: " << m_numberOfFullPassElectrons 
			    << std::endl;
  }

  // First require two muon-id's with cuts pre-applied.
  if ( (m_numberOfFullPassMuons + m_numberOfFullPassElectrons) < 4 ) {
    if(m_doDebug) {std::cout <<" * FourMuonEvent::EventSelection(" << eType << ") * Failing number of good muons and electrons == 4 :( " 
			     << m_numberOfFullPassMuons + m_numberOfFullPassElectrons 
			     << " = " << m_numberOfFullPassMuons << " + " << m_numberOfFullPassElectrons << std::endl;}
    return false;
  }
  
  if ( m_numberOfFullPassMuons > 4 ) {
    if(m_doDebug) {std::cout <<" * FourMuonEvent::EventSelection(" << eType << ") * Too many muons !! Failing number of good muons == 4 :( " 
			     <<  m_numberOfFullPassMuons << std::endl;}
    return false;
  }

  if ( m_numberOfFullPassElectrons > 4 ) {
    if(m_doDebug) {std::cout <<" * FourMuonEvent::EventSelection(" << eType << ") * Too many electrons !! Failing number of good electrons == 4 :( " 
			     <<  m_numberOfFullPassElectrons << std::endl;}
    return false;
  }
  
  // momentum of the muons
  double leadingMuonPt = -1., secondMuonPt=-1., thirdMuonPt=-1, fourthMuonPt=-1.; // negative pt, means not computed yet
  switch ( eType ) {
  case MS :
    {
      if (m_muonpos1 >= 0) leadingMuonPt = m_pxMSTrack[m_muonpos1]->pt();
      if (m_muonpos2 >= 0) secondMuonPt =  m_pxMSTrack[m_muonpos2]->pt();
      if (m_muonneg1 >= 0) thirdMuonPt =   m_pxMSTrack[m_muonneg1]->pt();
      if (m_muonneg2 >= 0) fourthMuonPt =  m_pxMSTrack[m_muonneg2]->pt();
      break;
    }
  case ME:
    {
      if (m_muonpos1 >= 0) leadingMuonPt = m_pxMETrack[m_muonpos1]->pt();
      if (m_muonpos2 >= 0) secondMuonPt =  m_pxMETrack[m_muonpos2]->pt();
      if (m_muonneg1 >= 0) thirdMuonPt =   m_pxMETrack[m_muonneg1]->pt();
      if (m_muonneg2 >= 0) fourthMuonPt =  m_pxMETrack[m_muonneg2]->pt();
      break;
    }
  case CB:
    {
      if (m_muonpos1 >= 0) leadingMuonPt = m_pxRecMuon[m_muonpos1]->pt();
      if (m_muonpos2 >= 0) secondMuonPt =  m_pxRecMuon[m_muonpos2]->pt();
      if (m_muonneg1 >= 0) thirdMuonPt =   m_pxRecMuon[m_muonneg1]->pt();
      if (m_muonneg2 >= 0) fourthMuonPt =  m_pxRecMuon[m_muonneg2]->pt();
      break;
    }
  case ID:
    {
      if (m_muonpos1 >= 0) leadingMuonPt = m_pxIDTrack[m_muonpos1]->pt();
      if (m_muonpos2 >= 0) secondMuonPt =  m_pxIDTrack[m_muonpos2]->pt();
      if (m_muonneg1 >= 0) thirdMuonPt =   m_pxIDTrack[m_muonneg1]->pt();
      if (m_muonneg2 >= 0) fourthMuonPt =  m_pxIDTrack[m_muonneg2]->pt();
      break;
    }
    
  default:
    if (m_muonpos1 >= 0) leadingMuonPt = m_pxIDTrack[m_muonpos1]->pt();
    if (m_muonpos2 >= 0) secondMuonPt =  m_pxIDTrack[m_muonpos2]->pt();
    if (m_muonneg1 >= 0) thirdMuonPt =   m_pxIDTrack[m_muonneg1]->pt();
    if (m_muonneg2 >= 0) fourthMuonPt =  m_pxIDTrack[m_muonneg2]->pt();
  } // end switch

  // up to here the leading and second pt are not really in the right order.
  // order the muon pt:
  double theLeadingPt = leadingMuonPt;
  if (secondMuonPt > theLeadingPt) { theLeadingPt = secondMuonPt;}
  if (thirdMuonPt  > theLeadingPt) { theLeadingPt = thirdMuonPt;}
  if (fourthMuonPt > theLeadingPt) { theLeadingPt = fourthMuonPt;}

  double theTrailingPt = leadingMuonPt;
  if (secondMuonPt < theTrailingPt && secondMuonPt > 0) { theTrailingPt = secondMuonPt;}
  if (thirdMuonPt  < theTrailingPt && thirdMuonPt > 0)  { theTrailingPt = thirdMuonPt;}
  if (fourthMuonPt < theTrailingPt && fourthMuonPt > 0) { theTrailingPt = fourthMuonPt;}
  
  if (m_doDebug || true) {
    std::cout << " * FourMuonEvent::EventSelection * muon pt selection -- cuts: Leading: " << m_LeadingMuonPtCut*CLHEP::GeV << std::endl
	      << "                                                                  2nd: " << m_SecondMuonPtCut*CLHEP::GeV << std::endl;
    std::cout << "                                          Pt of muons in this event 1: " << leadingMuonPt << std::endl
	      << "                                                                    2: " << secondMuonPt << std::endl
	      << "                                                                    3: " << thirdMuonPt << std::endl
	      << "                                                                    4: " << fourthMuonPt << std::endl
	      << "                                                           leading Pt: " << theLeadingPt << std::endl
	      << "                                                          trailing Pt: " << theTrailingPt << std::endl;
  }
  
  // muon pt cut
  if ( !(theLeadingPt > m_LeadingMuonPtCut*CLHEP::GeV && theTrailingPt >  m_SecondMuonPtCut*CLHEP::GeV ) ) {
    if(m_doDebug){  
      std::cout <<" * FourMuonEvent::EventSelection * Failing pt cut * Reco Pt:  leading " << theLeadingPt << "  --> trailing  " << theTrailingPt << std::endl;
    }
    return false;
  }
  if(m_doDebug){
    std::cout << " * FourMuonEvent::EventSelection * Event passed the pt cuts: leading muon pt: " << leadingMuonPt << std::endl;
    std::cout << "                                                            trailing muon pt: " << theTrailingPt << std::endl; 
  }
  
 const auto invariantMass  = m_fInvariantMass.at(eType);
  // Invariant mass window
  if ( invariantMass  < m_MassWindowLow  ) {
    if(m_doDebug) {
      std::cout <<" * FourMuonEvent::EventSelection * Failing mass window low cut:  reco m= " << invariantMass << " > " <<  m_MassWindowLow << std::endl;
    }
    return false;
  }
  if ( invariantMass  > m_MassWindowHigh ) {
    if(m_doDebug) {
      std::cout <<" * FourMuonEvent * Failing mass window high cut:  reco m= " << invariantMass << " > " <<  m_MassWindowHigh << std::endl;
    }
    return false;
  }
  if(m_doDebug){
    std::cout <<" * FourMuonEvent::EventSelection * Event passed the mass window: " << invariantMass << std::endl;
  }
  
  if(m_doDebug) {
    std::cout << " * FourMuonEvent::EventSelection( type= " << eType << ")*  Good 4-muon set: pt range from  " <<  leadingMuonPt/1000 
	      << " to " << secondMuonPt/1000 
	      << " GeV   4-muon invariant mass = " << m_fInvariantMass.at(eType) << " GeV \n";
    std::cout << " * FourMuonEvent::EventSelection( type= " << eType << ")*  completed * \n";
  }
  return true;
}

//==================================================================================
void FourMuonEvent::Clear()
{
  m_numberOfFullPassMuons = 0;
  m_numberOfFullPassElectrons = 0;
  m_passedSelectionCuts   = false;
  m_passedFourMuonSelection = false;
  m_passedFourElectronSelection = false;
  m_passedFourLeptonSelection = false;


  m_FourMuonInvMass = -1.; // flag as no reconstructed inv mass yet
  m_muon1 = MUON1; // point to the first two
  m_muon2 = MUON2;
  m_muonneg1 = -1;
  m_muonneg2 = -1;
  m_muonpos1 = -1;
  m_muonpos2 = -1;


  for ( unsigned int u = 0; u < NUM_MUONS; ++u ) {
      m_pxRecMuon[u] = nullptr;
      m_pxMSTrack[u] = nullptr;
      m_pxMETrack[u] = nullptr;
      m_pxIDTrack[u] = nullptr;
      m_pxMUTrack[u] = nullptr;
      m_pxELTrack[u] = nullptr;
  }
  for ( unsigned int v = 0; v < NUM_TYPES; ++v ) {
    m_fZPt[v]            = -999.9f;
    m_fZEtaDir[v]        = -999.9f;
    m_fZPhiDir[v]        = -999.9f;
    m_fInvariantMass[v]  = -999.9f;
    m_fMuonDispersion[v] = -999.9f;
  }

  // tell us to which vertex the muons are associated 
  m_nVertex = 0; // reset vertex count
  m_muonneg1_vtx = 0;
  m_muonneg2_vtx = 0;
  m_muonpos1_vtx = 0;
  m_muonpos2_vtx = 0;

  for (size_t i=0; i < NUM_MUONS; i++) m_muon_vtx[i] = 0; // reset the vertex ID to which the electrons are associated
  for (size_t i=0; i < NUM_MUONS; i++) m_elec_vtx[i] = 0; // reset the vertex ID to which the electrons are associated
  return;
}

//==================================================================================
void FourMuonEvent::RecordMuon( const xAOD::Muon* pxMuon )
{
  constexpr bool thisdebug = false;
  // This shouldn't really ever happen but just in case.
  if ( !pxMuon ) {
    if(m_doDebug){  std::cout <<" * FourMuonEvent * RecordMuon * bad pxMuon --> EXIT "<< std::endl;}
    return;
  }

  if ( m_numberOfFullPassMuons < NUM_MUONS ) {
      // The main Muon
      m_pxRecMuon[m_numberOfFullPassMuons] = pxMuon;
      if (thisdebug) {
	std::cout <<" * FourMuonEvent * RecordMuon * m_pxRecMuon for this muon--> pt "<< m_pxRecMuon[m_numberOfFullPassMuons]->pt() << std::endl;
	std::cout <<"                                                             d0 "<< m_pxRecMuon[m_numberOfFullPassMuons]->trackParticle(xAOD::Muon::TrackParticleType::Primary)->d0() << std::endl;
	std::cout <<"                                                       sigma_d0 "<< m_pxRecMuon[m_numberOfFullPassMuons]->trackParticle(xAOD::Muon::TrackParticleType::Primary)->definingParametersCovMatrixVec()[0] << std::endl;
      }

      const xAOD::TrackParticle* pxMSTrack   = pxMuon->trackParticle(xAOD::Muon::TrackParticleType::MuonSpectrometerTrackParticle);
      if (!pxMSTrack) {
	if (m_doDebug){  std::cout <<" * FourMuonEvent * RecordMuon * bad pxMSmuon --> EXIT "<< std::endl;}
	return;
      } 
      m_pxMSTrack[m_numberOfFullPassMuons] = pxMSTrack;
      if (thisdebug) {
	std::cout <<" * FourMuonEvent * RecordMuon * m_pxMSTrack for this muon--> pt "<< m_pxMSTrack[m_numberOfFullPassMuons]->pt() << std::endl;
	std::cout <<"                                                             d0 "<< m_pxMSTrack[m_numberOfFullPassMuons]->d0() << std::endl;
	std::cout <<"                                                       sigma_d0 "<< m_pxMSTrack[m_numberOfFullPassMuons]->definingParametersCovMatrixVec()[0] << std::endl;
      }

      // ID muon 
      const xAOD::TrackParticle*  pxIDTrack  = pxMuon->trackParticle(xAOD::Muon::TrackParticleType::InnerDetectorTrackParticle);
      if (!pxIDTrack) {
	if (m_doDebug){  std::cout <<" * FourMuonEvent * RecordMuon * bad pxIDTrack for this muon--> EXIT "<< std::endl;}
	return;
      }      
      m_pxIDTrack[m_numberOfFullPassMuons] = pxIDTrack;
      if (thisdebug) {
	std::cout <<" * FourMuonEvent * RecordMuon * m_pxIDTrack for this muon--> pt "<< m_pxIDTrack[m_numberOfFullPassMuons]->pt() << std::endl;
	std::cout <<"                                                             d0 "<< m_pxIDTrack[m_numberOfFullPassMuons]->d0() << std::endl;
	std::cout <<"                                                       sigma_d0 "<< m_pxIDTrack[m_numberOfFullPassMuons]->definingParametersCovMatrixVec()[0] << std::endl;
      }
      //
      ++m_numberOfFullPassMuons;
      ++m_acceptedMuonCount;
  }
  // if(m_doDebug){  std::cout <<" * FourMuonEvent * RecordMuon * completed -- return with a total of " << m_numberOfFullPassMuons << std::endl;}
  return;
}


//==================================================================================
bool FourMuonEvent::ReconstructKinematics()
{
  if(m_doDebug){ std::cout << " * FourMuonEvent * ReconstructKinematics * -- start -- " << std::endl; }
  bool kinematicscomputed = false;

  // Three ways. No checks here. Thus make sure the pointers are ok before this.
  if ( m_numberOfFullPassMuons == 4 ) {
    // crosscheck identifiers are good:
    bool goodidentifiers = true;
    if (m_muonneg1 < 0) goodidentifiers = false;
    if (m_muonneg2 < 0) goodidentifiers = false;
    if (m_muonpos1 < 0) goodidentifiers = false;
    if (m_muonpos2 < 0) goodidentifiers = false;

    if (goodidentifiers) {
      // before computing the kinematic parameters check track particles are ok
      bool goodtracks = true;
      if (m_pxIDTrack[m_muonneg1] == nullptr) goodtracks = false;
      if (m_pxIDTrack[m_muonneg2] == nullptr) goodtracks = false;
      if (m_pxIDTrack[m_muonpos1] == nullptr) goodtracks = false;
      if (m_pxIDTrack[m_muonpos2] == nullptr) goodtracks = false;
      
      if (m_pxIDTrack[m_muonneg1] != nullptr) m_pxMUTrack[0] = m_pxIDTrack[m_muonneg1]; 
      if (m_pxIDTrack[m_muonneg2] != nullptr) m_pxMUTrack[1] = m_pxIDTrack[m_muonneg2]; 
      if (m_pxIDTrack[m_muonpos1] != nullptr) m_pxMUTrack[2] = m_pxIDTrack[m_muonpos1]; 
      if (m_pxIDTrack[m_muonpos2] != nullptr) m_pxMUTrack[3] = m_pxIDTrack[m_muonpos2]; 

      if (goodtracks) { // Everything is ready
	m_fInvariantMass[ID]      = EvalFourMuInvMass( m_pxMUTrack[0], m_pxMUTrack[1], m_pxMUTrack[2], m_pxMUTrack[3] );
	m_fMuonDispersion[ID]     = EvaluateAngle(   m_pxMUTrack[0], m_pxMUTrack[2]); // leading mu- and leading mu+ 
	m_fZPt[ID]                = EvalPt(          m_pxMUTrack[0], m_pxMUTrack[2]); // leading mu- and leading mu+ 
	m_fZEtaDir[ID]            = EvalEta(         m_pxMUTrack[0], m_pxMUTrack[2]); // leading mu- and leading mu+ 
	m_fZPhiDir[ID]            = EvalPhi(         m_pxMUTrack[0], m_pxMUTrack[2]); // leading mu- and leading mu+ 
	
	kinematicscomputed = true;
	
	if(m_doDebug){ 
	  std::cout << " * FourMuonEvent * ReconstructKinematics4Elec * -- Muon ID Tracks -- new -- " << std::endl
		    << "                                           Pt(mu1-)= " << m_pxMUTrack[0]->pt() << std::endl
		    << "                                           Pt(mu2-)= " << m_pxMUTrack[1]->pt() << std::endl
		    << "                                           Pt(mu1+)= " << m_pxMUTrack[2]->pt() << std::endl
		    << "                                           Pt(mu2+)= " << m_pxMUTrack[3]->pt() << std::endl
		    << "                                           invariant mass (4mu) = " << m_fInvariantMass[ID] << std::endl
		    << std::endl; 
	}
	
      } // good tracks  
    } // goodidentifiers
  } // goodmuons == 4
  
  if (!kinematicscomputed) {
    if(m_doDebug){  std::cout <<" * FourMuonEvent * ReconstructKinematics * -- FAILED -- " << std::endl; }
  }

  if(m_doDebug){  std::cout <<" * FourMuonEvent * ReconstructKinematics * -- completed -- status: " << kinematicscomputed << std::endl; }
  return kinematicscomputed;
}

//==================================================================================
bool FourMuonEvent::ReconstructKinematics4Elec()
{
  if(m_doDebug){ std::cout << " * FourMuonEvent * ReconstructKinematics4Elec * -- start -- " << std::endl; }
  bool kinematicscomputed = false;

  // Three ways. No checks here. Thus make sure the pointers are ok before this.
  if ( m_numberOfFullPassElectrons == 4 ) {
    // before computing the kinematic parameters check track particles are ok
    bool goodtracks = true;
    m_pxELTrack[0] = m_xElecID.GetElecNegTrackParticle(0); 
    m_pxELTrack[1] = m_xElecID.GetElecNegTrackParticle(1); 
    m_pxELTrack[2] = m_xElecID.GetElecPosTrackParticle(0); 
    m_pxELTrack[3] = m_xElecID.GetElecPosTrackParticle(1); 
    if (m_pxELTrack[0] == nullptr) goodtracks = false;
    if (m_pxELTrack[1] == nullptr) goodtracks = false;
    if (m_pxELTrack[2] == nullptr) goodtracks = false;
    if (m_pxELTrack[3] == nullptr) goodtracks = false;
      
    if (goodtracks) { // Everything is ready
      // For the time being analysis is performed only with ID tracks
      m_fInvariantMass[ID]      = EvalFourMuInvMass( m_pxELTrack[0], m_pxELTrack[1], m_pxELTrack[2], m_pxELTrack[3]);
      m_fMuonDispersion[ID]     = EvaluateAngle(   m_pxELTrack[0], m_pxELTrack[2]); // leading e- and leading e+ 
      m_fZPt[ID]                = EvalPt(          m_pxELTrack[0], m_pxELTrack[2]); // leading e- and leading e+ 
      m_fZEtaDir[ID]            = EvalEta(         m_pxELTrack[0], m_pxELTrack[2]); // leading e- and leading e+ 
      m_fZPhiDir[ID]            = EvalPhi(         m_pxELTrack[0], m_pxELTrack[2]); // leading e- and leading e+ 
      kinematicscomputed = true;
      
      if(m_doDebug){ 
	std::cout << " * FourMuonEvent * ReconstructKinematics4Elec * -- Electron Tracks -- " << std::endl
		  << "                                           Pt(e1-)= " << m_pxELTrack[0]->pt() << std::endl
		  << "                                           Pt(e2-)= " << m_pxELTrack[1]->pt() << std::endl
		  << "                                           Pt(e1+)= " << m_pxELTrack[2]->pt() << std::endl
		  << "                                           Pt(e2+)= " << m_pxELTrack[3]->pt() << std::endl
		  << "                                           invariant mass (4e) = " << m_fInvariantMass[ID] << std::endl
		  << std::endl; 
      }
    } // good tracks  
  } // goodidentifiers

  if (!kinematicscomputed) {
    if(m_doDebug){  std::cout <<" * FourMuonEvent * ReconstructKinematics4Elec * -- FAILED -- " << std::endl; }
  }
  
  if(m_doDebug){  std::cout <<" * FourMuonEvent * ReconstructKinematics4Elec * -- completed -- status: " << kinematicscomputed << std::endl; }
  return kinematicscomputed;
}

//==================================================================================
bool FourMuonEvent::ReconstructKinematicsNew()
{
  (*m_msgStream) << MSG::DEBUG << " * FourMuonEvent * ReconstructKinematicsNew * -- START -- " << endmsg; 

  bool kinematicscomputed = false;

  // first step get the list of TrackParticles for muons and electrons
  // -- muons (a bit more complex than for electrons)
  if (m_muonneg1 >= 0) m_pxMUTrack[0] = m_pxIDTrack[m_muonneg1];  
  if (m_muonneg2 >= 0) m_pxMUTrack[1] = m_pxIDTrack[m_muonneg2];  
  if (m_muonpos1 >= 0) m_pxMUTrack[2] = m_pxIDTrack[m_muonpos1];  
  if (m_muonpos2 >= 0) m_pxMUTrack[3] = m_pxIDTrack[m_muonpos2];  
  // add an extra proteccion
  if (m_numberOfFullPassMuons < 2) {
    for (int i=0; i<4; i++) m_pxMUTrack[i] = nullptr;
  } 
  
  // -- electrons
  m_pxELTrack[0] = m_xElecID.GetElecNegTrackParticle(0); 
  m_pxELTrack[1] = m_xElecID.GetElecNegTrackParticle(1); 
  m_pxELTrack[2] = m_xElecID.GetElecPosTrackParticle(0); 
  m_pxELTrack[3] = m_xElecID.GetElecPosTrackParticle(1);
  // add an extra proteccion
  if (m_numberOfFullPassElectrons < 2) {
    for (int i=0; i<4; i++) m_pxELTrack[i] = nullptr;
  } 
  
  if ( m_numberOfFullPassMuons == 4 ) {
    bool goodtracks = true;
    if (m_pxMUTrack[0] == nullptr) goodtracks = false;
    if (m_pxMUTrack[1] == nullptr) goodtracks = false;
    if (m_pxMUTrack[2] == nullptr) goodtracks = false;
    if (m_pxMUTrack[3] == nullptr) goodtracks = false;
    
    if (goodtracks) { // Everything is ready
      m_fInvariantMass[ID]      = EvalFourMuInvMass ( m_pxMUTrack[0], m_pxMUTrack[1], m_pxMUTrack[2], m_pxMUTrack[3] );
      m_fMuonDispersion[ID]     = EvaluateAngle     ( m_pxMUTrack[0], m_pxMUTrack[2]); // leading mu- and leading mu+ 
      m_fZPt[ID]                = EvalPt            ( m_pxMUTrack[0], m_pxMUTrack[2]); // leading mu- and leading mu+ 
      m_fZEtaDir[ID]            = EvalEta           ( m_pxMUTrack[0], m_pxMUTrack[2]); // leading mu- and leading mu+ 
      m_fZPhiDir[ID]            = EvalPhi           ( m_pxMUTrack[0], m_pxMUTrack[2]); // leading mu- and leading mu+ 
      kinematicscomputed = true;
    }      
  }
    
  if(m_doDebug){ 
    std::cout << " * FourMuonEvent * ReconstructKinematicsNew * -- Muon ID Tracks -- new -- " << std::endl;
    if (m_pxMUTrack[0] != nullptr) std::cout << "                                           Pt(mu1-)= " << m_pxMUTrack[0]->pt() << std::endl;
    if (m_pxMUTrack[1] != nullptr) std::cout << "                                           Pt(mu2-)= " << m_pxMUTrack[1]->pt() << std::endl;
    if (m_pxMUTrack[2] != nullptr) std::cout << "                                           Pt(mu1+)= " << m_pxMUTrack[2]->pt() << std::endl;
    if (m_pxMUTrack[3] != nullptr) std::cout << "                                           Pt(mu2+)= " << m_pxMUTrack[3]->pt() << std::endl;
    if (kinematicscomputed)        std::cout << "                                           invariant mass (4mu) = " << m_fInvariantMass[ID] << std::endl;
  }

  double invmass_test = -1.; // default value
  if ( m_numberOfFullPassElectrons == 4) { 
    // before computing the kinematic parameters check track particles are ok
    bool goodtracks = true;
    if (m_pxELTrack[0] == nullptr) goodtracks = false;
    if (m_pxELTrack[1] == nullptr) goodtracks = false;
    if (m_pxELTrack[2] == nullptr) goodtracks = false;
    if (m_pxELTrack[3] == nullptr) goodtracks = false;
      
    if (goodtracks && !kinematicscomputed) { // Everything is ready
      // For the time being analysis is performed only with ID tracks
      m_fInvariantMass[ID]      = EvalFourMuInvMass( m_pxELTrack[0], m_pxELTrack[1], m_pxELTrack[2], m_pxELTrack[3]);
      invmass_test              = m_fInvariantMass[ID];
      m_fMuonDispersion[ID]     = EvaluateAngle(   m_pxELTrack[0], m_pxELTrack[2]); // leading e- and leading e+ 
      m_fZPt[ID]                = EvalPt(          m_pxELTrack[0], m_pxELTrack[2]); // leading e- and leading e+ 
      m_fZEtaDir[ID]            = EvalEta(         m_pxELTrack[0], m_pxELTrack[2]); // leading e- and leading e+ 
      m_fZPhiDir[ID]            = EvalPhi(         m_pxELTrack[0], m_pxELTrack[2]); // leading e- and leading e+ 
      kinematicscomputed = true;
    } // good tracks  
  }
  if(m_doDebug){ 
    std::cout << " * FourMuonEvent * ReconstructKinematicsNew * -- Electron Tracks -- " << std::endl;
    if (m_pxELTrack[0] != nullptr) std::cout << "                                           Pt(e1-)= " << m_pxELTrack[0]->pt() << std::endl;
    if (m_pxELTrack[1] != nullptr) std::cout << "                                           Pt(e2-)= " << m_pxELTrack[1]->pt() << std::endl;
    if (m_pxELTrack[2] != nullptr) std::cout << "                                           Pt(e1+)= " << m_pxELTrack[2]->pt() << std::endl;
    if (m_pxELTrack[3] != nullptr) std::cout << "                                           Pt(e2+)= " << m_pxELTrack[3]->pt() << std::endl;
    std::cout << "                                           invariant mass (4e) = " <<  invmass_test << std::endl; 
  }

  if ( m_numberOfFullPassMuons >= 2 && m_numberOfFullPassElectrons >= 2 && !kinematicscomputed) {
    // before computing the kinematic parameters check track particles are ok
    bool goodtracks = true;
    if (m_pxMUTrack[0] == nullptr) goodtracks = false; // leading mu-
    if (m_pxMUTrack[2] == nullptr) goodtracks = false; // leading mu+
    if (m_pxELTrack[0] == nullptr) goodtracks = false; // leading e-
    if (m_pxELTrack[2] == nullptr) goodtracks = false; // leading e+
      
    if (goodtracks && !kinematicscomputed) { // Everything is ready
      // For the time being analysis is performed only with ID tracks
      m_fInvariantMass[ID]      = EvalFourMuInvMass ( m_pxMUTrack[0], m_pxMUTrack[2], m_pxELTrack[0], m_pxELTrack[2]);
      invmass_test              = m_fInvariantMass[ID];
      m_fMuonDispersion[ID]     = EvaluateAngle     ( m_pxMUTrack[0], m_pxELTrack[0]); // leading mu- and leading e- 
      m_fZPt[ID]                = EvalPt            ( m_pxMUTrack[0], m_pxELTrack[0]); // leading mu- and leading e- 
      m_fZEtaDir[ID]            = EvalEta           ( m_pxMUTrack[0], m_pxELTrack[0]); // leading mu- and leading e- 
      m_fZPhiDir[ID]            = EvalPhi           ( m_pxMUTrack[0], m_pxELTrack[0]); // leading mu- and leading e- 
      kinematicscomputed = true;
    } // good tracks  
  }
  if(m_doDebug){ 
    std::cout << " * FourMuonEvent * ReconstructKinematicsNew * -- Muon and Electron Tracks -- " << std::endl;
    if (m_pxMUTrack[0] != nullptr) std::cout << "                                           Pt(mu1-)= " << m_pxMUTrack[0]->pt() << std::endl;
    if (m_pxMUTrack[1] != nullptr) std::cout << "                                           Pt(mu2-)= " << m_pxMUTrack[1]->pt() << std::endl;
    if (m_pxMUTrack[2] != nullptr) std::cout << "                                           Pt(mu1+)= " << m_pxMUTrack[2]->pt() << std::endl;
    if (m_pxMUTrack[3] != nullptr) std::cout << "                                           Pt(mu2+)= " << m_pxMUTrack[3]->pt() << std::endl;
    if (m_pxELTrack[0] != nullptr) std::cout << "                                           Pt(e1-)= " << m_pxELTrack[0]->pt() << std::endl;
    if (m_pxELTrack[1] != nullptr) std::cout << "                                           Pt(e2-)= " << m_pxELTrack[1]->pt() << std::endl;
    if (m_pxELTrack[2] != nullptr) std::cout << "                                           Pt(e1+)= " << m_pxELTrack[2]->pt() << std::endl;
    if (m_pxELTrack[3] != nullptr) std::cout << "                                           Pt(e2+)= " << m_pxELTrack[3]->pt() << std::endl;
    std::cout << "                                           invariant mass used = " <<  m_fInvariantMass[ID]  << std::endl; 
    std::cout << "                                           invariant mass test = " <<  invmass_test << std::endl; 
  }
  
  if (!kinematicscomputed) {
    if(m_doDebug){  std::cout <<" * FourMuonEvent * ReconstructKinematicsNew * -- FAILED -- " << std::endl; }
  }
  
  (*m_msgStream) << MSG::DEBUG << " * FourMuonEvent * ReconstructKinematicsNew * -- COMPLETED -- status " << kinematicscomputed << endmsg; 
  return kinematicscomputed;
}

//==================================================================================
float FourMuonEvent::getPtImbalance( ZTYPE eType )
{
  // First determine what's positive
  if ( m_numberOfFullPassMuons == 2 )
    {
      switch ( eType )
	{
	case MS :
	  {
	    return EvalPtDiff( m_pxMSTrack[m_muon1], m_pxMSTrack[m_muon2] );
	  }
	case ME:
	  {
	    return EvalPtDiff( m_pxMETrack[m_muon1], m_pxMETrack[m_muon2] );
	  }
	case CB:
	  {
	    return EvalPtDiff( m_pxRecMuon[m_muon1], m_pxRecMuon[m_muon2] );
	  }
	case ID:
	  {
	    return EvalPtDiff( m_pxIDTrack[m_muon1], m_pxIDTrack[m_muon2] );
	  }
	default:
	  return -999.0;
	}
    }
  else
    {
      return -999.0;
    }
}

//==================================================================================
int FourMuonEvent::getZCharge( ZTYPE eType )
{
  switch ( eType )
    {
    case MS :
      {
	return ( static_cast<int>( EvalCharge( m_pxMSTrack[m_muon1], m_pxMSTrack[m_muon2] ) ) );
      }
    case ME:
      {
	return ( static_cast<int>( EvalCharge( m_pxMETrack[m_muon1], m_pxMETrack[m_muon2] ) ) );
      }
    case CB:
      {
	return ( static_cast<int>( EvalCharge( m_pxRecMuon[m_muon1], m_pxRecMuon[m_muon2] ) ) );
      }
    case ID:
      {
	return ( static_cast<int>( EvalCharge( m_pxIDTrack[m_muon1], m_pxIDTrack[m_muon2] ) ) );
      }
    default:
      return -999;
    }
}

//==================================================================================
unsigned int FourMuonEvent::getPosMuon( int eType )
{
  //if ( getNumberOfTaggedMuons() != 2 ) return 999;
  //if ( getZCharge(eType) != 0        ) return 999;

  unsigned int muid = m_muonpos1;
  if (eType==2) muid = m_muonpos2;
  return muid;
}

//==================================================================================
unsigned int FourMuonEvent::getNegMuon( int eType )
{
  unsigned int muid = m_muonneg1;
  if (eType==2) muid = m_muonneg2;
  return muid;
}

//==================================================================================
const xAOD::TrackParticle*  FourMuonEvent::getLooseIDTk( unsigned int /*uPart*/ )
{
  const xAOD::TrackParticleContainer*  pxTrackContainer =
    PerfMonServices::getContainer<xAOD::TrackParticleContainer>( PerfMonServices::TRK_COLLECTION );

  if ( pxTrackContainer )
    {
      xAOD::TrackParticleContainer::const_iterator xTrkItr  = pxTrackContainer->begin();
      xAOD::TrackParticleContainer::const_iterator xTrkItrE  = pxTrackContainer->end();
      while ( xTrkItr != xTrkItrE )
	{
	  const xAOD::TrackParticle* pxTrack = *xTrkItr;
	  if ( !pxTrack ) continue;
	  const Trk::Track* pxTrkTrack = pxTrack->track();
	  if(!pxTrkTrack) continue;
	  const Trk::Perigee* pxPerigee = pxTrkTrack->perigeeParameters() ;
	  if ( !pxPerigee ) continue;
	  const float fTrkPhi   = pxPerigee->parameters()[Trk::phi];
	  const float fTrkEta   = pxPerigee->eta();

	  float fDPhi = fabs( fTrkPhi -  m_pxMETrack[m_muon1]->phi() );
	  float fDEta = fabs( fTrkEta -  m_pxMETrack[m_muon2]->eta() );
	  float fDR = sqrt( fDPhi*fDPhi + fDEta*fDEta );

	  if ( fDR < 0.3f )
	    {
	      return pxTrack;
	    }

	  ++xTrkItr;
	}
    }
  // if ()
  return nullptr;
}

//==================================================================================
void FourMuonEvent::SetLeadingMuonPtCut (double newvalue)
{
  // first set the new pt cut value
  m_LeadingMuonPtCut = newvalue;

  // the second muon pt cut can not be higher than the leading muon pt cut:
  if (m_LeadingMuonPtCut < m_SecondMuonPtCut) this->SetSecondMuonPtCut(m_LeadingMuonPtCut);

  // this has to be translated to the MuonSelector
  // but there one has to use the minimum momentum --> second muon
  //this->SetMuonPtCut(m_SecondMuonPtCut);
  if(m_doDebug){ 
    std::cout <<" * FourMuonEvent * SetLeadingMuonPtCut * new Pt cuts:  " << m_LeadingMuonPtCut << " & " 
	      << m_SecondMuonPtCut 
	      << "  MuonSelector: " << m_xMuonID.GetPtCut() << std::endl;
  }
  return;
}

//==================================================================================
void FourMuonEvent::SetSecondMuonPtCut (double newvalue) 
{
  m_SecondMuonPtCut = newvalue;

  // use same for electrons  
  m_xElecID.SetPtCut(m_SecondMuonPtCut);

  // second muon pt shouldn't be higher than the leading muon pt
  if (m_LeadingMuonPtCut < m_SecondMuonPtCut) this->SetLeadingMuonPtCut(m_LeadingMuonPtCut);

  // this has to be translated to the MuonSelector
  this->SetMuonPtCut(m_SecondMuonPtCut);

  if(m_doDebug) {
    std::cout <<" * FourMuonEvent * SetSecondMuonPtCut * new Pt cuts:  " << m_LeadingMuonPtCut 
	      << " & " << m_SecondMuonPtCut 
	      << "  MuonSelector: " << m_xMuonID.GetPtCut() << std::endl;
  }

  return;
}

//==================================================================================
void FourMuonEvent::OrderMuonList()
{
  // Salva: 20/January/2020 RecMuon -> IDTrack
  constexpr bool thisdebug = false;

  if (m_doDebug || thisdebug) {std::cout << " * FourMuonEvent::OrderMuonList * -- start -- " << std::endl 
					 << "                                  #muons: " << m_numberOfFullPassMuons<< std::endl;}

  int muPlus1Id = -9;
  int muPlus2Id = -9;
  int muMinus1Id = -9;
  int muMinus2Id = -9;
  double muPlus1Pt = 0.;
  double muPlus2Pt = 0.;
  double muMinus1Pt = 0.;
  double muMinus2Pt = 0.;

  int muposcount = 0;
  int munegcount = 0;

  int nMuonsAtEntry = m_numberOfFullPassMuons;
  m_numberOfFullPassMuons = 0; // reset the number of full pass muons

  if (nMuonsAtEntry >= 2) { // we need at least 2 muons
    for (int imuon=0; imuon < (int) nMuonsAtEntry; imuon++) {
      if(m_doDebug && false ){ std::cout << " * FourMuonEvent::OrderMuonList * testing imuon= " << imuon 
					 << "   with charge= " << m_pxRecMuon[imuon]->charge()
					 << "   and pt= " << m_pxRecMuon[imuon]->pt()
					 << std::endl;
      }
      if (m_pxIDTrack[imuon] != nullptr) {
	
	if (m_pxIDTrack[imuon]->charge()==1) { // positive muon
	  muposcount++;
	  if (m_pxIDTrack[imuon]->pt()> muPlus1Pt) {
	    // store 1st in 2nd
	    muPlus2Pt = muPlus1Pt;
	    muPlus2Id = muPlus1Id;
	    // now store the new one in 1st place
	    muPlus1Pt = m_pxIDTrack[imuon]->pt();
	    muPlus1Id = imuon;	
	  } 
	  else if (m_pxIDTrack[imuon]->pt()> muPlus2Pt) {
	    // store the new one in 2nd place
	    muPlus2Pt = m_pxIDTrack[imuon]->pt();
	    muPlus2Id = imuon;
	  }
	}
	// Negative muons
	if (m_pxIDTrack[imuon]->charge()==-1) {
	  munegcount++;
	  if(m_pxIDTrack[imuon]->pt()> muMinus1Pt) {
	    // store 1st in 2nd
	    muMinus2Pt = muMinus1Pt;
	    muMinus2Id = muMinus1Id;
	    muMinus1Pt = m_pxIDTrack[imuon]->pt();
	    muMinus1Id = imuon;
	  } 
	  else if(m_pxRecMuon[imuon]->pt()> muMinus2Pt) {
	    muMinus2Pt = m_pxIDTrack[imuon]->pt();
	    muMinus2Id = imuon;
	  }
	}
      } // muon exist
    } // for (int imuon)
  } // if (nMuonsAtEntry >= 2)

  // require at least one opposite charge muon pair
  if (nMuonsAtEntry >= 2 && (muposcount == 0 || munegcount == 0)) {
    if (m_doDebug) std::cout << " -- FourMuonEvent::OrderMuonList -- No opposite charge muons in the " << nMuonsAtEntry << " input muons"
			     << " #mu+ " << muposcount
			     << " #mu- " << munegcount
			     << " --> DISCARD ALL MUONS -- \n";
    muPlus1Id = -9;
    muPlus2Id = -9;
    muMinus1Id = -9;
    muMinus2Id = -9;
  }


  if (muPlus1Id>=0) {m_muonpos1 = muPlus1Id; m_numberOfFullPassMuons++;}
  if (muPlus2Id>=0) {m_muonpos2 = muPlus2Id; m_numberOfFullPassMuons++;}
  if (muMinus1Id>=0) {m_muonneg1 = muMinus1Id; m_numberOfFullPassMuons++;}
  if (muMinus2Id>=0) {m_muonneg2 = muMinus2Id; m_numberOfFullPassMuons++;}

  m_muon1 = m_muonpos1; // to be deleted when no more m_muon is left
  m_muon2 = m_muonneg1; // to be deleted when no more m_muon is left

  if ((m_doDebug || thisdebug) && m_numberOfFullPassMuons >= 2){ 
    std::cout << " * FourMuonEvent::OrderMuonList * taking " << m_numberOfFullPassMuons << "  muons from the input list of " << nMuonsAtEntry << " muons: " << std::endl;
    if (muMinus1Id >= 0) std::cout << "                                  leading mu-: " << muMinus1Id << "   Pt = " << muMinus1Pt << std::endl;
    if (muMinus2Id >= 0) std::cout << "                                  second  mu-: " << muMinus2Id << "   Pt = " << muMinus2Pt << std::endl;
    if (muPlus1Id >= 0)  std::cout << "                                  leading mu+: " << muPlus1Id  << "   Pt = " << muPlus1Pt << std::endl;
    if (muPlus2Id >= 0)  std::cout << "                                  second  mu+: " << muPlus2Id  << "   Pt = " << muPlus2Pt << std::endl;
  }
  else {
    if (m_doDebug) std::cout << " * FourMuonEvent::OrderMuonList * This event has less than 2 muons :("  << std::endl;
  }
  
  if (m_doDebug || thisdebug) std::cout << " * FourMuonEvent::OrderMuonList * completed * m_numberOfFullPassMuons= " << m_numberOfFullPassMuons << std::endl;
  return;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////                                                                      
bool FourMuonEvent::CheckMuonVertices ()
{
  (*m_msgStream) << MSG::DEBUG << " * FourMuonsEvents::CheckMuonVertices * -- START --" << endmsg; 
  
  if (m_doDebug) std::cout << " -- FourMuonEvent::CheckMuonVertices -- WARNING -- MUONS DO NOT COME FROM SAME VERTEX \n" ; 

  (*m_msgStream) << MSG::DEBUG << " * FourMuonsEvents::CheckMuonVertices * -- COMPLETED -- status: " << true << endmsg; 
  return true;
}
