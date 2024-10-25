/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDCINJPULSERAMPMAP_H
#define ZDCINJPULSERAMPMAP_H

#include "AsgMessaging/AsgMessaging.h"
#include <nlohmann/json.hpp>

#include <string>
#include <vector>
#include <cmath>


class ZdcInjPulserAmpMap : public asg::AsgMessaging
{
private:
  //
  // Data members
  //
  std::string m_filePath;
  unsigned int m_firstLB{1};
  std::vector<float> m_LBPulserVMap;

  // Private Methods
  //
  void ReadPulserSteps(std::ifstream& ifs);
  void FillVVector(const nlohmann::json& entry);
  
public:
  ZdcInjPulserAmpMap();

  static const ZdcInjPulserAmpMap* getInstance();
  
  std::string getFilePath() const {return m_filePath;}

  // Return the lumi block number at which we start stepping through the different aplitudes
  //
  unsigned int getFirstLumiBlock() const {return m_firstLB;}
  
  // Return the cycle within which the lumi block falls 
  // 
  int getCycleNumber(unsigned int lumiBlock) const
  {
    if (lumiBlock < m_firstLB) return -1;
    return std::floor(float(lumiBlock - m_firstLB)/m_LBPulserVMap.size());
  }

  // Return the pulser amplitude for the given lumi block
  //
  float getPulserAmplitude(unsigned int lumiBlock) const
  {
    // We do a cyclic lookup of the pulser amplitude (in volts) starting from the first LB
    //
    if (lumiBlock < m_firstLB) return -1000.;
    unsigned int vecIndex = (lumiBlock - m_firstLB) % m_LBPulserVMap.size();
    return m_LBPulserVMap.at(vecIndex);
  }

};

#endif //ZDCLUCRODMAPRUN3_H
