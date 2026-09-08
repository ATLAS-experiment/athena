/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
//==================================================================================
//
//  ElectronSelector.cxx :       Class designed to reconstruct di-electrons events
//                        in particular Z0 -> e+ e- events.
//==================================================================================

//==================================================================================
// Include files...
//==================================================================================

// This files header
#include "InDetPerformanceMonitoring/ElectronSelector.h"
// Package Headers
#include "InDetPerformanceMonitoring/PerfMonServices.h"

// ATLAS headers
#include "AthenaKernel/getMessageSvc.h"
#include "StoreGate/StoreGateSvc.h"
#include "CLHEP/Random/RandFlat.h"

#include <sstream>
// Static declarations
std::atomic<unsigned int> ElectronSelector::s_uNumInstances;

//==================================================================================
// Public Methods
//==================================================================================
ElectronSelector::ElectronSelector():
  m_doDebug ( false ),
  m_ptCut ( 10. ),
  m_etaCut ( 2.47 ) // 2.47 is the official acceptance for central electrons. Forward electrons is another story...
{
  ++s_uNumInstances;
  
  std::stringstream xTmp;  xTmp << s_uNumInstances;
  m_xSampleName     = "ElectronSelector_" + xTmp.str();
  
  m_pxElectron = nullptr;
  
  m_msgStream =  new MsgStream(Athena::getMessageSvc(), "InDetPerformanceMonitoring" );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ElectronSelector::~ElectronSelector()
{
  --s_uNumInstances;
  delete m_msgStream;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ElectronSelector::Init()
{
  (*m_msgStream) << MSG::DEBUG << " -- ElectronSelector::Init -- START -- " << endmsg;

  // PARENT::Init();

  //---Electron Likelihood tool---
  // m_doIDCuts = true;
  (*m_msgStream) << MSG::INFO << "ElectronSelector::Init -- Setting up electron LH tool." << endmsg;
  m_LHTool2015 = new AsgElectronLikelihoodTool ("m_LHTool2015");

  const std::string elecWorkingPoint = "LooseLHElectron"; // "MediumLHElectron" "TightLHElectron"

  if((m_LHTool2015->setProperty("WorkingPoint",elecWorkingPoint.c_str())).isFailure()) {
    (*m_msgStream) << MSG::WARNING << "Failure loading ConfigFile for electron likelihood tool with working point: " << elecWorkingPoint.c_str()  << endmsg;
  } 
  else  {
    (*m_msgStream) << MSG::INFO << "Loading ConfigFile for electron likelihood tool with working point: " << elecWorkingPoint << ". SUCCESS " << endmsg;    
  } 

  // check config files at: https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/EGammaIdentificationRun2
  std::string confDir = "ElectronPhotonSelectorTools/offline/mc20_20210514/ElectronLikelihoodVeryLooseOfflineConfig2017_Smooth.conf";
  if ( (m_LHTool2015->setProperty("ConfigFile", confDir)).isSuccess()) {
    (*m_msgStream) << MSG::INFO << "Electron likelihood config ("<< confDir.c_str() << ") setting SUCCESS!" << endmsg;
  }
  else {
    (*m_msgStream) << MSG::WARNING << "Electron likelihood config ("<< confDir.c_str() << ") setting FAILURE" << endmsg;
  }

  if (m_LHTool2015->initialize().isSuccess()) {
    (*m_msgStream) << MSG::INFO << "Electron likelihood tool initialize() SUCCESS!" << endmsg;
  }
  else {
    (*m_msgStream) << MSG::WARNING << "Electron likelihood tool initialize() FAILURE!" << endmsg;
  }

  (*m_msgStream) << MSG::DEBUG << " --ElectronSelector::Init -- COMPLETED -- " << endmsg;
  return;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ElectronSelector::PrepareElectronList(const xAOD::ElectronContainer* pxElecContainer)
{
  (*m_msgStream) << MSG::DEBUG << " --ElectronSelector::PrepareElectronList -- START  -- " << endmsg;
  Clear(); // clear current list records

  using electron_iterator = xAOD::ElectronContainer::const_iterator;
  electron_iterator iter    = pxElecContainer->begin();
  electron_iterator iterEnd = pxElecContainer->end();
  
  // Loop over the Electrons                                                                                                                                                       
  int electroncount = 0;
  for(; iter != iterEnd ; ++iter) {
    electroncount++;
    (*m_msgStream) << MSG::DEBUG  << " -- ElectronSelector::PrepareElectronList -- candiate electron " << electroncount 
		   << " has author " << (*iter)->author(xAOD::EgammaParameters::AuthorElectron)
		   << endmsg;
    const xAOD::Electron * ELE = (*iter);
    if ( RecordElectron(ELE) ) {
      (*m_msgStream) << MSG::DEBUG  << " -- ElectronSelector::PrepareElectronList -- candiate electron " << electroncount 
		     << " is good " 
		     << endmsg;
    }
  }
  bool progressingwell = true;
  
  (*m_msgStream) << MSG::DEBUG  << " -- ElectronSelector::PrepareElectronList -- finished recording electrons. "
		 << "  recorded electrons: " << m_pxElTrackList.size()
		 << "  out of tested electron candidates:" << electroncount  << endmsg;
  if (m_pxElTrackList.size() < 2) progressingwell = false;
  if (progressingwell) progressingwell = OrderElectronList ();
  if (progressingwell) progressingwell = RetrieveVertices ();

  if (!progressingwell) {
    (*m_msgStream) << MSG::DEBUG  << " -- ElectronSelector::PrepareElectronList -- FAILED -- this event has not even a good e+e- pair "  << endmsg;
    this->Clear(); // reset the content as it is not going to be used
  }

  (*m_msgStream) << MSG::DEBUG << " -- ElectronSelector::PrepareElectronList -- COMPLETED -- electroncount -- m_pxElTrackList.size() / all = " 
		 << m_pxElTrackList.size() << " / " << electroncount 
		 << endmsg;
  return;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ElectronSelector::RecordElectron (const xAOD::Electron * thisElec)
{
  // start assuming electron candidate is good 
  bool electronisgood = true;

  // check the electron satisfies the working point
  if (!m_LHTool2015->accept(thisElec) ) {
    electronisgood = false;
    (*m_msgStream) << MSG::DEBUG << " -- electron fails workingpoint selection  -- " << endmsg;
  }

  //Get the track particle                                                                                                                                                        
  const xAOD::TrackParticle* theTrackParticle = thisElec->trackParticle();
  
  if (!theTrackParticle) {
    electronisgood = false;
    (*m_msgStream) << MSG::DEBUG << " -- electron fails trackparticle  -- " << endmsg;
  }

  if (electronisgood && thisElec->author(xAOD::EgammaParameters::AuthorElectron) != 1) {
    electronisgood = false;
    (*m_msgStream) << MSG::DEBUG << "   -- electron fails author  -- " << thisElec->author(xAOD::EgammaParameters::AuthorElectron) << endmsg;
  }

  if (electronisgood && theTrackParticle->pt() * m_CGeV < m_ptCut ) { // pt cut given in GeV
    electronisgood = false;
    (*m_msgStream) << MSG::DEBUG << "   -- electron fails pt cut  -- pt= " << theTrackParticle->pt()
		   << " < " << m_ptCut << " (cut value) "
		   << endmsg;
  }

  const xAOD::CaloCluster* cluster = thisElec->caloCluster();
  if(!cluster) {
    electronisgood = false;
    (*m_msgStream) << MSG::DEBUG << "   -- electron candidate has no CaloCluster  " << endmsg; 
  }

  if (electronisgood && (cluster->e() * sin(theTrackParticle->theta())) * m_CGeV < m_ptCut) { // cut on et of the cluster
    electronisgood = false;
    (*m_msgStream) << MSG::DEBUG << "   -- electron fails cluster Et cut  -- Et= " << (cluster->e() * cos(theTrackParticle->theta()))* m_CGeV 
		   << " < " << m_ptCut << " (cut value) "
		   << endmsg;
  }

  if (electronisgood && (std::abs(cluster->eta())> m_etaCut || std::abs(theTrackParticle->eta())> m_etaCut) ) { // cut in eta for the cluster and the track 
    electronisgood = false;
    (*m_msgStream) << MSG::DEBUG << "   -- electron fails eta cut  -- cluster_eta= " << cluster->eta() << endmsg;
  }

  if (electronisgood) {
    // store this electron
    m_pxElTrackList.push_back(theTrackParticle);

    (*m_msgStream) << MSG::DEBUG << " * RecordElectron * good electron found -> store this electron with pt " << theTrackParticle->pt() 
		   << "  --> current m_pxElTrackList.size(): " << m_pxElTrackList.size()
		   << std::endl;
  }

  return electronisgood;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////                                                                      
void ElectronSelector::Clear()
{
  m_pxElTrackList.clear();
  m_goodElecNegTrackParticleList.clear();
  m_goodElecPosTrackParticleList.clear();
 
  // -1 means not assigned
  m_elecneg1 = -1;
  m_elecneg2 = -1;
  m_elecpos1 = -1;
  m_elecpos2 = -1;

  return;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////                                                                      
bool ElectronSelector::OrderElectronList()
{
  (*m_msgStream) << MSG::DEBUG << " -- ElectronSelector::OrderElectronList -- START  -- list size: " << m_pxElTrackList.size( ) << endmsg;
  if (m_pxElTrackList.size() < 2) {
    return false;
  }
  struct LeadingElectrons {
    double leadingPt{};
    double subleadingPt{};
    int leadingIndex{-1};
    int subleadingIndex{-1};
    std::size_t count{};
  };
  //
  LeadingElectrons negative;
  LeadingElectrons positive;
  const auto updateLeading = [](LeadingElectrons& electrons, double pt, int index) {
    ++electrons.count;
    if (pt > electrons.leadingPt) {
      electrons.subleadingPt = electrons.leadingPt;
      electrons.subleadingIndex = electrons.leadingIndex;
      electrons.leadingPt = pt;
      electrons.leadingIndex = index;
    } else if (pt > electrons.subleadingPt) {
      electrons.subleadingPt = pt;
      electrons.subleadingIndex = index;
    }
  };

  for (std::size_t index = 0; index < m_pxElTrackList.size(); ++index){
    const xAOD::TrackParticle* electron = m_pxElTrackList[index];
    if (electron->charge() < 0.) { // negative electrons
      updateLeading(negative, electron->pt(), static_cast<int>(index));
    } else if (electron->charge() > 0.) { // positive electrons
      updateLeading(positive, electron->pt(), static_cast<int>(index));
    }
  }

  if (negative.count == 0 || positive.count == 0) {
    if (m_doDebug) {
      std::cout << " -- ElectronSelector::OrderElectronList -- "
        "No opposite-charge electrons --> DISCARD ALL ELECTRONS --\n";
    }
    Clear();
    return false;
  }
  m_elecneg1 = negative.leadingIndex;
  m_elecneg2 = negative.subleadingIndex;
  m_elecpos1 = positive.leadingIndex;
  m_elecpos2 = positive.subleadingIndex;
  //
  if (m_doDebug) {
    std::cout << " -- ElectronSelector::OrderElectronList -- electron summary list taking "
      << negative.count + positive.count << " electrons from the input list of "
      << m_pxElTrackList.size() << " electrons:\n";
    //
    if (m_elecneg1 >= 0) {
      std::cout << "                                leading e-: " << m_elecneg1
       << "   Pt = " << negative.leadingPt << '\n';
    }
    //
    if (m_elecneg2 >= 0) {
      std::cout << "                                second  e-: "<< m_elecneg2 
      << "   Pt = " << negative.subleadingPt << '\n';
    }
    //
    if (m_elecpos1 >= 0) {
      std::cout << "                                leading e+: " << m_elecpos1
      << "   Pt = " << positive.leadingPt << '\n';
    }
    //
    if (m_elecpos2 >= 0) {
      std::cout << "                                second  e+: " << m_elecpos2
      << "   Pt = " << positive.subleadingPt << '\n';
    }
  }
  const auto addElectron = [this](auto& output, int index) {
    if (index >= 0) {
      output.push_back(m_pxElTrackList[static_cast<std::size_t>(index)]);
    }
  };
  addElectron(m_goodElecNegTrackParticleList, m_elecneg1);
  addElectron(m_goodElecNegTrackParticleList, m_elecneg2);
  addElectron(m_goodElecPosTrackParticleList, m_elecpos1);
  addElectron(m_goodElecPosTrackParticleList, m_elecpos2);
  //
  (*m_msgStream) << MSG::DEBUG << " -- ElectronSelector::OrderElectronList -- COMPLETED  -- status: true\n";
  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////                                                                      
bool ElectronSelector::RetrieveVertices ()
{
  if (m_doDebug) std::cout << " -- ElectronSelector::RetrieveVertices -- START  -- list size: " 
			   << m_goodElecNegTrackParticleList.size() + m_goodElecPosTrackParticleList.size() 
			   << "\n";
  bool goodvertices = false; 
  const int nverticesfound = 1; // WARNING default must be 0 --> set to 1 for R22 --> needs to be fixed
  if (nverticesfound >= 1) goodvertices = true;
  if (m_doDebug) std::cout << " -- ElectronSelector::RetrieveVertices -- COMPLETED -- status: " << goodvertices << std::endl; 
  return goodvertices;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////                                                                      
const xAOD::TrackParticle* ElectronSelector::GetElecNegTrackParticle (size_t i) 
{
  if (i >= m_goodElecNegTrackParticleList.size()) { // requesting out of range electron
    return nullptr;
  } 
  return m_goodElecNegTrackParticleList.at(i);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////                                                                      
const xAOD::TrackParticle* ElectronSelector::GetElecPosTrackParticle (size_t i) 
{
  if (i >=  m_goodElecPosTrackParticleList.size()) { // requesting out of range electron
    return nullptr;
  } 
  return m_goodElecPosTrackParticleList.at(i);
}
