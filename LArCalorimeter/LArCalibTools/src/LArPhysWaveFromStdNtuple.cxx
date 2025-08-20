/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "LArCalibTools/LArPhysWaveFromStdNtuple.h"

#include "LArIdentifier/LArOnlineID.h"
#include "LArIdentifier/LArOnline_SuperCellID.h"
#include "CaloIdentifier/CaloGain.h"
#include "LArRawConditions/LArPhysWave.h"
#include "LArRawConditions/LArPhysWaveContainer.h"

#include "TFile.h"
#include "TBranch.h"
#include "TTree.h"
#include "TChain.h"

#include <vector>
#include <iostream>
#include <string>

LArPhysWaveFromStdNtuple::LArPhysWaveFromStdNtuple(const std::string & name, ISvcLocator * pSvcLocator):AthAlgorithm(name, pSvcLocator) {};

LArPhysWaveFromStdNtuple::~LArPhysWaveFromStdNtuple()= default;

StatusCode LArPhysWaveFromStdNtuple::stop()
{
  ATH_MSG_INFO ( "... in stop()" );
  
  // get LArOnlineID helper
  const LArOnlineID_Base* onlineHelper = nullptr;
  if(m_isSC) {
     const LArOnline_SuperCellID* onltmp;
     ATH_CHECK( detStore()->retrieve(onltmp, "LArOnline_SuperCellID") );
     onlineHelper = static_cast<const LArOnlineID_Base*>( onltmp);
  } else {
     const LArOnlineID* onltmp = nullptr;
     ATH_CHECK( detStore()->retrieve(onltmp, "LArOnlineID") );
     onlineHelper = (const LArOnlineID_Base*) onltmp;
  }

  TChain* outfit = new TChain(m_ntuple_name.value().c_str());
  for (const std::string& s : m_root_file_names) {
    outfit->Add(s.c_str());
  }


  Int_t           timeIndex;
  Int_t           flag;
  Double_t          Dt;
  Double_t        timeOffset;
  Int_t           channelId;
  Int_t           FT, slot, channel;
  //avoid large stack use, with heap allocation
  auto Amplitude = std::make_unique<Double_t[]>(2000); // The function
  auto Error     = std::make_unique<Double_t[]>(2000); // The function
  Int_t          Triggers[2000]; // The function
  Int_t           gain = 0; // LARHIGHGAIN = 0, LARMEDIUMGAIN = 1,  LARLOWGAIN = 2,
  outfit->SetBranchAddress("channelId", &channelId);
  outfit->SetBranchAddress("FT", &FT);
  outfit->SetBranchAddress("slot", &slot);
  outfit->SetBranchAddress("channel", &channel);
  outfit->SetBranchAddress("timeIndex", &timeIndex);
  outfit->SetBranchAddress("Dt", &Dt);
  outfit->SetBranchAddress("timeOffset", &timeOffset);
  outfit->SetBranchAddress("flag", &flag);
  outfit->SetBranchAddress("gain", &gain);
  outfit->SetBranchAddress("Amplitude", Amplitude.get());
  outfit->SetBranchAddress("Error", Error.get());
  outfit->SetBranchAddress("Triggers", Triggers);

  // Create new LArPhysWaveContainer
  LArPhysWaveContainer* larPhysWaveContainerNew = new LArPhysWaveContainer();
  ATH_CHECK ( larPhysWaveContainerNew->setGroupingType(m_groupingType, msg()) );
  ATH_CHECK ( larPhysWaveContainerNew->initialize() );

  unsigned int hwid;
  unsigned int uflag;
  // loop over entries in the Tuple, one entry = one channel
  Long64_t nentries = outfit->GetEntries();
  for ( Long64_t i = 0; i < nentries; i++ )
  {
    outfit->GetEvent(i);
    if(m_isSC && gain >0) continue;
    ATH_MSG_INFO ( " Chan " <<  std::hex << channelId << std::dec );

    hwid = channelId;
    HWIdentifier id(hwid);
    if(FT != onlineHelper->feedthrough(id) || slot != onlineHelper->slot(id) || channel != onlineHelper->channel(id)) {
      ATH_MSG_ERROR ( "Inconsistency in decoding HWID !!!!" );
      ATH_MSG_ERROR ( FT << " - " << onlineHelper->feedthrough(id) );
      ATH_MSG_ERROR ( slot << " - " << onlineHelper->slot(id) );
      ATH_MSG_ERROR ( channel << " - " << onlineHelper->channel(id) );
      ATH_MSG_ERROR ( "Not creating PhysWave !!!!" );
      continue;
    }

    // Catch potential array index out of range error.
    if ( timeIndex >= 2000 ) {
      ATH_MSG_ERROR ( " Too many points specified vs the expected content of the ntuple ! " );
      ATH_MSG_ERROR ( "Not creating PhysWave !!!!" );
      continue;
    }
    std::vector<double> wave(timeIndex);
    std::vector<double> wave_err(timeIndex);
    std::vector<int> wave_trig(timeIndex);
    for ( int i = 0; i < timeIndex; i++ ) {
       wave[i]=0.;
       wave_err[i]=0.;
       wave_trig[i]=0.;
    }
    unsigned int skipped = 0;
    unsigned int limit = timeIndex;
    if ( m_skipPoints < m_prefixPoints ) limit = timeIndex+m_skipPoints-m_prefixPoints;
    for ( unsigned int i = 0; i < limit; i++ )
    {
      if ( skipped >= m_skipPoints ) 
      {
	wave[i-m_skipPoints+m_prefixPoints]=Amplitude[i];
	wave_err[i-m_skipPoints+m_prefixPoints]=Error[i];
	wave_trig[i-m_skipPoints+m_prefixPoints]=Triggers[i];
      }
      else skipped++;
    }
 
    uflag = flag;
    LArPhysWave newLArPhysWave(wave, wave_err, wave_trig, Dt, timeOffset, uflag);
	  
    // Add physics wave to container
    larPhysWaveContainerNew->setPdata(id, newLArPhysWave, (CaloGain::CaloGain)gain);
  } 
  ATH_CHECK( detStore()->record(larPhysWaveContainerNew,m_store_key) );
  ATH_MSG_INFO ( "LArPhysWaveFromStdNtuple finalized!" );
  return StatusCode::SUCCESS;
}
