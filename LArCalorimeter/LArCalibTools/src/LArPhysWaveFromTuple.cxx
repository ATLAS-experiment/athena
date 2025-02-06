/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "LArCalibTools/LArPhysWaveFromTuple.h"

#include "LArIdentifier/LArOnlineID.h"
#include "LArRawConditions/LArPhysWave.h"
#include "LArRawConditions/LArPhysWaveContainer.h"
#include "CaloIdentifier/CaloGain.h"

#include "TFile.h"
#include "TBranch.h"
#include "TTree.h"
#include "TChain.h"

#include <vector>
#include <iostream>
#include <fstream>
#include <string>


LArPhysWaveFromTuple::LArPhysWaveFromTuple (const std::string& name, ISvcLocator* pSvcLocator) : AthAlgorithm(name, pSvcLocator)
{  
}

LArPhysWaveFromTuple::~LArPhysWaveFromTuple() 
= default;

StatusCode LArPhysWaveFromTuple::initialize() 
{
  return StatusCode::SUCCESS ;
}


StatusCode LArPhysWaveFromTuple::stop()
{
  ATH_MSG_INFO ( "... in stop()" );
  
  // get LArOnlineID helper
  const LArOnlineID* onlineHelper = nullptr;
  ATH_CHECK( detStore()->retrieve(onlineHelper, "LArOnlineID") );

  TChain* outfit = new TChain(m_ntuple_name.value().c_str());
  outfit->Add(m_root_file_name.value().c_str());

  // This algorithm assumes the input NTuple in contains less than 2000 
  // points.  If the NTuple contains less than 2000 points, the 
  // remaining points are automatically initialized to 0.
  // Catch potential array index out of range error.
  if ( m_NPoints > 2000 ) {
    ATH_MSG_WARNING ( " Too many points specified vs the expected content of the ntuple ! " );
    ATH_MSG_WARNING ( " Only 2000 will be used !");
    m_NPoints = 2000;
  }

  // variable names as in the Ntuple
  Int_t           channelId; 
  UInt_t           tIndex; 
  Double_t        Amplitude[2000]; 
  Double_t        Time[2000]; 
  outfit->SetBranchAddress("channelId", &channelId);
  outfit->SetBranchAddress("timeIndex", &tIndex);
  outfit->SetBranchAddress("Amplitude", Amplitude);
  outfit->SetBranchAddress("Time", Time);

  // Create new LArPhysWaveContainer
  LArPhysWaveContainer* larPhysWaveContainerNew = new LArPhysWaveContainer();
  ATH_CHECK ( larPhysWaveContainerNew->setGroupingType(m_groupingType, msg()) );
  ATH_CHECK ( larPhysWaveContainerNew->initialize() );

  // loop over entries in the Tuple, one entry = one channel
  Long64_t nentries = outfit->GetEntries();
  std::vector<double> wave(m_NPoints);
  for ( Long64_t i = 0; i < nentries; i++ )
  {
    outfit->GetEvent(i);
    HWIdentifier id(channelId);
    ATH_MSG_INFO ( std::hex << id << std::dec );

    for ( unsigned int i = 0; i < m_NPoints; i++ ) wave[i]=0.;
    unsigned int skipped = 0;
    unsigned int limit = tIndex<m_NPoints.value() ? tIndex : m_NPoints.value();
    if ( m_skipPoints < m_prefixPoints ) limit = m_NPoints+m_skipPoints-m_prefixPoints;
    double dt=Time[1]-Time[0];
    for ( unsigned int i = 0; i < limit; i++ )
    {
      if(i>1 && Time[i] - Time[i-1] != dt) {
         ATH_MSG_ERROR("No equidistant time, could not process....");
         return StatusCode::FAILURE;
      }
      if ( skipped >= m_skipPoints ) 
      {
	wave[i-m_skipPoints+m_prefixPoints]=Amplitude[i];
      }
      else skipped++;
    }
 
    LArPhysWave newLArPhysWave(wave, dt, m_flag);
	  
    // Add physics wave to container
    larPhysWaveContainerNew->setPdata(id, newLArPhysWave, (CaloGain::CaloGain)m_gain.value());
  }

  ATH_CHECK( detStore()->record(larPhysWaveContainerNew,m_store_key) );
  ATH_MSG_INFO ( "LArPhysWaveFromTuple finalized!" );
  return StatusCode::SUCCESS;
}
