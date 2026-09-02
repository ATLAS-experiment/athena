/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "LArCalibTools/LArCaliWaveFromTuple.h"

#include "LArIdentifier/LArOnlineID.h"
#include "LArIdentifier/LArOnline_SuperCellID.h"
#include "LArRawConditions/LArCaliWaveContainer.h"
#include "CaloIdentifier/CaloGain.h"

#include "TFile.h"
#include "TBranch.h"
#include "TTree.h"
#include "TChain.h"

#include <vector>
#include <map>
#include <algorithm> //for std::find
#include <string>
#include <ranges> 
#include <cmath> //for sqrt
#include <ios> //for hex, dec
#include <memory>


StatusCode LArCaliWaveFromTuple::stop()
{
  ATH_MSG_INFO ( "... in stop()" );
  
  const std::vector<int> FEBs={0x39000000, 0x39010000, 0x39018000, 0x39020000, 0x39028000, 0x39030000, 0x39038000, 0x39040000, 0x39048000, 0x39050000, 0x39058000, 0x39060000, 0x39068000};

  TChain* outfit = new TChain(m_ntuple_name.value().c_str());
  outfit->Add(m_root_file_name.value().c_str());

  // This algorithm assumes the input NTuple in contains less than 32 
  // points.  If the NTuple contains less than 32 points, the 
  // remaining points are automatically initialized to 0.
  // Catch potential array index out of range error.
  if ( m_NPoints > 32 ) {
    ATH_MSG_WARNING ( " Too many points specified vs the expected content of the ntuple ! " );
    ATH_MSG_WARNING ( " Only 32 will be used !");
    m_NPoints = 32;
  }

  // loop over delay branch and find number of existing delays
  Int_t           delay; 
  Int_t           febid; 
  outfit->SetBranchAddress("delay", &delay);
  outfit->SetBranchAddress("febId", &febid);
  Long64_t nentries = outfit->GetEntries();
  std::map<int,int> tmpmap;
  std::vector<int> febvec;
  for(Long64_t i = 0; i < nentries; ++i ){
      outfit->GetEntry(i);
      tmpmap[delay]=delay;
      if(std::find(febvec.begin(), febvec.end(), febid) == febvec.end()) febvec.push_back(febid);
  }
  unsigned ndelays = tmpmap.size();
  if(febvec.size() > 13) {
     ATH_MSG_ERROR("Too many FEBs in the ntuple, fix the code");
     return StatusCode::FAILURE;
  }     

  // variable names as in the Ntuple
  Int_t           febchannel; 
  Int_t           gain; 
  Int_t           dac; 
  std::vector<int>    *Amplitude = nullptr; 
  outfit->SetBranchAddress("febChannel", &febchannel);
  outfit->SetBranchAddress("gain", &gain);
  outfit->SetBranchAddress("dac", &dac);
  outfit->SetBranchAddress("ADC", &Amplitude);

  // Create new LArCaliWaveContainer
  auto larCaliWaveContainerNew = std::make_unique<LArCaliWaveContainer>();
  ATH_CHECK ( larCaliWaveContainerNew->setGroupingType(m_groupingType, msg()) );
  ATH_CHECK ( larCaliWaveContainerNew->initialize() );

  // loop over entries in the Tuple, fill the waves sum and sum2
  typedef std::map<HWIdentifier,std::map<std::pair<short, int> , std::vector<long int> > > Wavesmap_t;
  Wavesmap_t wavesum;
  Wavesmap_t wavesum2;
  Wavesmap_t wavensum;
  for ( Long64_t iev = 0; iev < nentries; ++iev ) {
    outfit->GetEvent(iev);
    //check gain validity
    if(gain < CaloGain::LARHIGHGAIN || gain >= CaloGain::LARNGAINR4) continue; // bad gain value
    // hack until real febId in ntuple:
    auto it = std::find(febvec.begin(), febvec.end(), febid);
    unsigned int index= it - febvec.begin();
    HWIdentifier id( FEBs[index] | ((febchannel&0x7F)<<8) );
    ATH_MSG_DEBUG ( "onlineid created " << std::hex << id << std::dec);
    std::pair<short, int>  idpair=std::make_pair(gain,dac);
    // check if we have this HWid and gain.dac pair already
    if(  ! wavesum.count(id) || ! wavesum[id].count(idpair)) { // we have new combination
        wavesum[id][idpair].reserve((m_NPoints + m_prefixPoints) * ndelays);
        wavesum2[id][idpair].reserve((m_NPoints + m_prefixPoints) * ndelays);
        wavensum[id][idpair].reserve((m_NPoints + m_prefixPoints) * ndelays);
        for ( unsigned int i = 0; i < (m_NPoints + m_prefixPoints) * ndelays; ++i ){
          wavesum[id][idpair][i]=0;
          wavesum2[id][idpair][i]=0;
          wavensum[id][idpair][i]=0;
        }
    }
    unsigned upper = m_NPoints.value() > Amplitude->size() ? Amplitude->size() : m_NPoints.value();
    for ( unsigned int i = 0; i < upper; ++i) {
       if ( m_skipPoints > 0 && m_skipPoints <= i+1 ) continue; 
       wavesum[id][idpair][(m_prefixPoints+delay)*ndelays*m_NPoints + i] += Amplitude->at(i);
       wavesum2[id][idpair][(m_prefixPoints+delay)*ndelays*m_NPoints + i] += Amplitude->at(i)*Amplitude->at(i);
       wavensum[id][idpair][(m_prefixPoints+delay)*ndelays*m_NPoints + i] += 1;
    }
 
  }// loop over ntuple
  ATH_MSG_INFO("Loop done");

  // compute the average and fill LArCaliWaveVec
  double dt=m_dt.value();
  for(auto  & [hwid, wmap] : wavesum) {
    std::vector<LArCaliWaveVec> waveVec(CaloGain::LARNGAINR4);
    for(auto dgkey : std::views::keys(wavesum[hwid])) { 
       auto dac = dgkey.second;
       auto gain = dgkey.first;
       std::vector<double> amp((m_NPoints + m_prefixPoints) * ndelays, 0.);
       std::vector<double> err((m_NPoints + m_prefixPoints) * ndelays, 0.);
       std::vector<int> ntrig((m_NPoints + m_prefixPoints) * ndelays, 0);
       for(unsigned int i=0; i< (m_NPoints + m_prefixPoints) * ndelays; ++i) {
          if(wavensum[hwid][dgkey][i] > 0) {
             amp[i] = (double)wavesum[hwid][dgkey][i] / wavensum[hwid][dgkey][i];
             err[i] = std::sqrt((double)wavesum2[hwid][dgkey][i] / wavensum[hwid][dgkey][i] - amp[i]*amp[i]);
             ntrig[i] = wavensum[hwid][dgkey][i];
          }
       }
       waveVec[gain].push_back(LArCaliWave(amp, err, ntrig, dt, dac,1, LArWave::meas));
    }
    // Add waveVec to container
    for(unsigned ig = CaloGain::LARHIGHGAIN; ig < CaloGain::LARNGAINR4; ++ ig) {
       larCaliWaveContainerNew->setPdata(hwid, waveVec[ig], (CaloGain::CaloGain)ig);
    }
  } 
	  

  ATH_CHECK( detStore()->record(std::move(larCaliWaveContainerNew),m_store_key) );
  ATH_MSG_INFO ( "LArCaliWaveFromTuple finalized!" );
  return StatusCode::SUCCESS;
}
