/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "LArCalibTools/LArPhysWaveFromAscii.h"

#include "LArIdentifier/LArOnlineID.h"
#include "LArIdentifier/LArOnline_SuperCellID.h"
#include "LArRawConditions/LArPhysWaveContainer.h"

#include "CaloIdentifier/CaloGain.h"

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <fstream>
#include <memory>

LArPhysWaveFromAscii::LArPhysWaveFromAscii(const std::string & name, ISvcLocator * pSvcLocator):AthAlgorithm(name, pSvcLocator) {};

LArPhysWaveFromAscii::~LArPhysWaveFromAscii()= default;

StatusCode LArPhysWaveFromAscii::stop()
{
  ATH_MSG_INFO ( "... in stop()" );
  
  // input file
  std::ifstream inf(m_input_file_name);
  if(!inf.is_open()){
     ATH_MSG_ERROR("Could not open input file "<<m_input_file_name);
     return StatusCode::FAILURE;
  }

  // Create new LArPhysWaveContainer
  auto larPhysWaveContainerNew = std::make_unique<LArPhysWaveContainer>();
  ATH_CHECK ( larPhysWaveContainerNew->setGroupingType(m_groupingType, msg()) );
  ATH_CHECK ( larPhysWaveContainerNew->initialize() );

  unsigned int hwid=0;
  unsigned int lcounter=0;
  float timdiff=0;
  float lasttime=0;
  // loop over lines
  std::string line="";
  std::vector<double> wave;
  std::vector<double> wave_err;
  std::vector<int> wave_trig;

  unsigned lsize = m_hasIndex ? 4 : 3;

  while ( ! std::getline(inf, line).eof() )
  {
    std::vector<std::string> strvec;
    boost::split(strvec, line, boost::is_any_of(" "));
    if(strvec.size() != lsize) {
       ATH_MSG_WARNING("Wrong line: "<<line<<", skipped "<<strvec.size());
       continue;
    }
    //check if we have new channel
    if(!hwid) { // first line
          if(std::stoi(strvec[1]) != 0) {
            ATH_MSG_WARNING("Inconsistent first line of channel: "<<line);
            continue;
          }
          hwid = std::stoi(strvec[0]);
          lcounter=0;
          timdiff=0;
          lasttime=std::stof(strvec[lsize - 2]);
          wave.push_back(std::stod(strvec[lsize - 1]));
          wave_err.push_back(0);
          wave_trig.push_back(1);
    } else if(hwid != std::stoul(strvec[0])) { // new channels
          // store previous wave
          unsigned int skipped = 0;
          unsigned int limit = wave.size();
          if ( m_skipPoints < m_prefixPoints ) limit += m_skipPoints-m_prefixPoints;
          std::vector<double> store_wave(limit,0);
          std::vector<double> store_wave_err(limit,0);
          std::vector<int> store_wave_trig(limit,1);
          for ( unsigned int i = 0; i < limit; i++ ) {
            if ( skipped >= m_skipPoints ) {
              store_wave[i-m_skipPoints+m_prefixPoints]=wave[i];
              store_wave_err[i-m_skipPoints+m_prefixPoints]=wave_err[i];
              store_wave_trig[i-m_skipPoints+m_prefixPoints]=wave_trig[i];
            }
            else skipped++;
          }
   
          LArPhysWave newLArPhysWave(store_wave, store_wave_err, store_wave_trig, timdiff, 0, 0);
          // Add physics wave to container
          larPhysWaveContainerNew->setPdata(HWIdentifier(hwid), newLArPhysWave, (CaloGain::CaloGain)m_gain.value());

          // reset vectors
          wave.clear();
          wave_err.clear();
          wave_trig.clear();

          if(m_hasIndex && std::stoi(strvec[1]) != 0) {
            ATH_MSG_WARNING("Inconsistent first line of channel: "<<line);
            continue;
          }
          hwid = std::stoi(strvec[0]);
          lcounter=0;
          timdiff=0;
          lasttime=std::stof(strvec[lsize - 2]);
          wave.push_back(std::stod(strvec[lsize - 1]));
          wave_err.push_back(0);
          wave_trig.push_back(1);

    } else { // next line in channel
          if(m_hasIndex && lcounter+1 != std::stoul(strvec[1])){ // wrong series ?
             ATH_MSG_WARNING("Wrong next line: "<<line);
             ATH_MSG_WARNING(lcounter+1 << " " << std::stoi(strvec[1]));
             continue;
          }
          if(!timdiff) {
             timdiff= std::stof(strvec[lsize - 2]) - lasttime;
          } else {
             if(int(timdiff) != int(std::stof(strvec[lsize - 2]) - lasttime)) {
                ATH_MSG_WARNING("Non equal time bins ? "<<line);
                ATH_MSG_WARNING(int(timdiff)<<" "<<int(std::stof(strvec[lsize - 2]) - lasttime)<<" "<<lasttime);
                ++lcounter;
                continue;
             }
          }
          lasttime=std::stof(strvec[lsize - 2]);
          wave.push_back(std::stod(strvec[lsize - 1]));
          wave_err.push_back(0);
          wave_trig.push_back(1);
          ++lcounter;
    }

  }
  // store last wave
  unsigned int skipped = 0;
  unsigned int limit = wave.size();
  if ( m_skipPoints < m_prefixPoints ) limit += m_skipPoints-m_prefixPoints;
  std::vector<double> store_wave(limit,0);
  std::vector<double> store_wave_err(limit,0);
  std::vector<int> store_wave_trig(limit,1);
  for ( unsigned int i = 0; i < limit; i++ ) {
    if ( skipped >= m_skipPoints ) {
      store_wave[i-m_skipPoints+m_prefixPoints]=wave[i];
      store_wave_err[i-m_skipPoints+m_prefixPoints]=wave_err[i];
      store_wave_trig[i-m_skipPoints+m_prefixPoints]=wave_trig[i];
    }
    else skipped++;
  }
  
  LArPhysWave newLArPhysWave(store_wave, store_wave_err, store_wave_trig, timdiff, 0, 0);
  // Add physics wave to container
  larPhysWaveContainerNew->setPdata(HWIdentifier(hwid), newLArPhysWave, (CaloGain::CaloGain)m_gain.value());

  ATH_CHECK( detStore()->record(std::move(larPhysWaveContainerNew),m_store_key) );
  ATH_MSG_INFO ( "LArPhysWaveFromAscii finalized!" );
  return StatusCode::SUCCESS;
}
