/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "LArCalibTools/LArOFCBin_PhysCaliTdiffFromStdNtuple.h"

#include "LArIdentifier/LArOnlineID.h"
#include "CaloIdentifier/CaloGain.h"
#include "LArRawConditions/LArOFCBinComplete.h"
#include "LArRawConditions/LArPhysCaliTdiffComplete.h"

#include "TFile.h"
#include "TBranch.h"
#include "TTree.h"
#include "TChain.h"

#include <vector>
#include <iostream>
#include <fstream>
#include <string>


LArOFCBin_PhysCaliTdiffFromStdNtuple::~LArOFCBin_PhysCaliTdiffFromStdNtuple() = default;

StatusCode LArOFCBin_PhysCaliTdiffFromStdNtuple::initialize() 
{
  return StatusCode::SUCCESS ;
}


StatusCode LArOFCBin_PhysCaliTdiffFromStdNtuple::stop()
{
  ATH_MSG_INFO ( "... in stop()" );
  
  const LArOnlineID* onlineHelper = nullptr;
  ATH_CHECK( detStore()->retrieve(onlineHelper, "LArOnlineID") );

  TChain* outfit = new TChain(m_ntuple_name.value().c_str());
  for (const std::string& s : m_root_file_names) {
    outfit->Add(s.c_str());
  }


  Int_t           channelId{};
  Int_t           FT{}, slot{}, channel{};
  Int_t           bin{};
  Double_t        tdiff{};


  Int_t           gain = 0; // LARHIGHGAIN = 0, LARMEDIUMGAIN = 1,  LARLOWGAIN = 2,
  outfit->SetBranchAddress("channelId", &channelId);
  outfit->SetBranchAddress("FT", &FT);
  outfit->SetBranchAddress("slot", &slot);
  outfit->SetBranchAddress("channel", &channel);
  outfit->SetBranchAddress("gain", &gain);
  if(m_filltdiff) {
   outfit->SetBranchAddress("Tdiff", &tdiff);
  }
  if(m_fillofc) {
   outfit->SetBranchAddress("OFCBin", &bin);
  }

  // Create new objects
  auto larOFCParams = std::make_unique<LArOFCBinComplete>();
  ATH_CHECK ( larOFCParams->setGroupingType(m_groupingType, msg()) );
  ATH_CHECK ( larOFCParams->initialize() );
  auto larTdiffParams = std::make_unique<LArPhysCaliTdiffComplete>();
  ATH_CHECK ( larTdiffParams->setGroupingType(m_groupingType, msg()) );
  ATH_CHECK ( larTdiffParams->initialize() );


  unsigned int hwid{};
  //unsigned int uflag;
  // loop over entries in the Tuple, one entry = one channel
  Long64_t nentries = outfit->GetEntries();
  for ( Long64_t i = 0; i < nentries; i++ )
  {
    outfit->GetEvent(i);
    ATH_MSG_INFO ( " Chan " <<  std::hex << channelId << std::dec );
    ATH_MSG_INFO ( bin << " | " << tdiff );
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
	  
    // Add to objects
    if(m_fillofc) larOFCParams->set(id,gain,bin);
    if(m_filltdiff) larTdiffParams->set(id,gain,tdiff);
  }


  // store 
  if(m_fillofc) {
    ATH_CHECK( detStore()->record(std::move(larOFCParams),m_store_key_ofcbin) );
  }

  if(m_filltdiff) {
    ATH_CHECK( detStore()->record(larTdiffParams.get(),m_store_key_tdiff) );
  }


  ATH_MSG_INFO ( "LArOFCBin_PhysCaliTdiffFromStdNtuple finalized!" );
  return StatusCode::SUCCESS;
}
