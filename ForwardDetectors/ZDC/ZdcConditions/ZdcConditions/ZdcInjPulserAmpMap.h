/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef _ZDCINJPULSERAMPMAP_H
#define _ZDCINJPULSERAMPMAP_H

#include "AsgMessaging/AsgMessaging.h"
#include <nlohmann/json.hpp>

#include <string>
#include <vector>
#include <cmath>


#include <iostream>
#include <string>
#include <tuple>
#include <map>
#include <mutex>
#include <vector>
#include <utility>

class ZdcInjPulserAmpMap : public asg::AsgMessaging
{
public:
  class Token
  {
    int m_index;
    float m_scaleFactor;

    int value() const {return m_index;}
    float scaleFactor() const {return m_scaleFactor;}

    Token(int index, float scaleFactor) :
      m_index(index),
      m_scaleFactor(scaleFactor)
    {}

  public:
    friend class ZdcInjPulserAmpMap;

    Token() :
      m_index(-1),
      m_scaleFactor(1)
    {}

    bool isValid() const {return m_index != -1;}
  };
  
  using json = nlohmann::json;
    
private:

  using StepsVector = std::vector<float>;
  using StepsDescr = std::pair<unsigned int, StepsVector>;

  using RunRangeDescr = std::tuple<unsigned int, unsigned int, std::string, float>;
  
  // Mutex for thread safety
  //
  std::mutex m_lock;
  
  //
  // Data members
  //
  std::string m_filePath;
  bool m_validJSon{false};

  // List of run ranges in json file
  //
  std::vector<RunRangeDescr> m_runRangeDescrs;

  // Map of step configurations indexed by config name
  //
  std::map<std::string, StepsDescr> m_stepsConfigs;
  
  // For multi-threaded operation, we keep multiple active configurations with index as the token
  //
  std::vector<const StepsDescr*> m_activeConfigs;

  // Private Methods
  //
  bool parseJsonFile(std::ifstream& ifs);
  void readPulserSteps(StepsDescr& steps, const json& stepsJson);
  void fillVVector(StepsVector& stepVec, const nlohmann::json& entry);
  
  std::pair<unsigned int, StepsVector> getContext(const Token& token) const
  {
    if (!m_validJSon) {
      throw std::runtime_error("Invalid Injected Pulser configuration");
    }
    if (!token.isValid() || size_t(token.value()) >= m_activeConfigs.size()) {
      throw std::runtime_error("Invalid Injected Pulser configuration");
    }
    
    unsigned int index = token.value();
    const StepsDescr& stepsConfig = *(m_activeConfigs[index]);
  
    unsigned int firstLB = stepsConfig.first;
    const StepsVector& steps = stepsConfig.second;
  
    return std::make_pair(firstLB, steps);
  }

public:
  
  ZdcInjPulserAmpMap();

  static const ZdcInjPulserAmpMap* getInstance();

  const std::string& getFilePath() const {return m_filePath;}

  Token lookupRun(unsigned int runNumber, bool allowDefault = false);
    
  // Return the lumi block number at which we start stepping through the different aplitudes
  //
  unsigned int getFirstLumiBlock(const Token& token) const {
    auto result = getContext(token);
    return result.first;
  }
  
  unsigned int getNumSteps(const Token& token) const {
    auto result = getContext(token);
    return result.second.size();
  }

  // Return the cycle within which the lumi block falls 
  // 
  int getCycleNumber(const Token& token, unsigned int lumiBlock) const
  {
    auto [firstLB, map] = getContext(token);
      
    if (lumiBlock < firstLB) return -1;
    return std::floor(float(lumiBlock - firstLB)/map.size());
  }

  // Return the pulser amplitude for the given lumi block
  //
  float getPulserAmplitude(const Token& token, unsigned int lumiBlock) const
  {
    auto [firstLB, map] = getContext(token);

    // We do a cyclic lookup of the pulser amplitude (in volts) starting from the first LB
    //
    if (lumiBlock < firstLB) return -1000.;
    unsigned int vecIndex = (lumiBlock - firstLB) % map.size();
    return map.at(vecIndex) * token.scaleFactor();
  }
};

#endif //
