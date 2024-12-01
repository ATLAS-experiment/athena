/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "ZdcConditions/ZdcInjPulserAmpMap.h"
#include "PathResolver/PathResolver.h"
#include <fstream>

// Get singleton instance
//
const ZdcInjPulserAmpMap* ZdcInjPulserAmpMap::getInstance()
{
  static const ZdcInjPulserAmpMap pulser_map;
  return &pulser_map;
}


//constructor
ZdcInjPulserAmpMap::ZdcInjPulserAmpMap() : asg::AsgMessaging("ZdcInjPulserAmpMap")
{
  msg().setLevel(MSG::INFO);

  // std::string filePath = PathResolverFindCalibFile("ZdcConditions/ZDC_InjectPulseSteps.json"); // change back file name after other changes get merged + ready for T0 reprocessing
  std::string filePath = PathResolverFindCalibFile("ZdcConditions/INJpulser_combined_2024.json");

  if (!filePath.empty())
    {
      ATH_MSG_DEBUG( "ZdcInjPulserAmpMap::found ZDC JSON at " << filePath );
    }
  else
    {
      ATH_MSG_WARNING(  "ZdcInjPulserAmpMap constructor, JSON file not found in search path, trying local file" ) ;
      filePath = "./ZDC_InjectPulseSteps.json";
    }


  std::ifstream ifs(filePath);
  if (!ifs.is_open()) {
    ATH_MSG_FATAL("ZdcInjPulserAmpMap constructor, JSON file cannot be opened!" ) ;
  }

  m_filePath = filePath;
  if (parseJsonFile(ifs)) m_validJSon = true;;
}

bool ZdcInjPulserAmpMap::parseJsonFile(std::ifstream& ifs)
{
  try {
    nlohmann::json j = nlohmann::json::parse(ifs);
    
    // Check for the existence of the run range section of the json file
    //
    auto rangeIter = j.find("RunRanges");
    if (rangeIter == j.end()) return false;
    
    //
    //  We loop over all the defined run ranges and save
    //
    for (json range : *rangeIter) {
      unsigned int first = range.at("First");
      unsigned int last = range.at("Last");
      std::string name = range.at("StepsConfig");
      float scale = range.at("ScaleFactor");
      
      m_runRangeDescrs.push_back(std::make_tuple(first, last, name, scale));

      ATH_MSG_DEBUG( "ZdcInjPulserAmpMap::found run range: first, last = " << first << ", " << last
		     << ", config name = " << name << ", scale factor = " << scale );
    }
    
    auto stepConfigIter = j.find("StepConfigurations");
    if (stepConfigIter == j.end()) return false;
    
    // Now loop over the configurations and save
    //
    for (json::iterator configIt = stepConfigIter->begin(); configIt != stepConfigIter->end(); ++configIt) {
      std::string confName = configIt.key();
      
      // First we make a spot in the map
      //
      auto [insertIter, insertResult] = m_stepsConfigs.insert(std::make_pair(confName, StepsDescr()));
      if (!insertResult) return false;
      
      // And now we fill it
      //
      json steps = configIt.value();
      if (!steps.is_array()) {
	return false;
      }
      
      // Unpack the elements of the step confg array
      //
      StepsDescr& stepsDesc = insertIter->second;
      readPulserSteps(stepsDesc, steps);

      ATH_MSG_DEBUG( "ZdcInjPulserAmpMap::found steps configuration with name " << confName  << ", starting LB = "
		     << stepsDesc.first << ", number of LBs = " <<  stepsDesc.second.size());    
    }
  }
  catch (...) {
    return false;
  }

  return true;
}

void ZdcInjPulserAmpMap::readPulserSteps(StepsDescr& steps, const json& stepsJson)
{
  // Loop over all entries in the json array
  //
  for (unsigned int index = 0; index < stepsJson.size(); index++) {
    const json& elem = stepsJson[index];

    // Get the starting LB entry in the json if it exists
    //
    if (index ==0 && elem.size() == 1) {
      if (elem.find("startLB") != elem.end()) {
	steps.first = elem["startLB"];
      }
    }
    else {
      // Fill out the pulser amplitudes for this step
      //
      fillVVector(steps.second, elem);
    }
  }
}

void ZdcInjPulserAmpMap::fillVVector(StepsVector& stepVec, const nlohmann::json& entry)
{
  unsigned int nStep = entry.at("nStep");
  float vStart = entry.at("vStart");
  float vStep = entry.at("vStep");

  float voltage = vStart;
  for (size_t step = 0; step < nStep; step++) {
    stepVec.push_back(voltage);
    voltage += vStep;
  }
}

ZdcInjPulserAmpMap::Token ZdcInjPulserAmpMap::lookupRun(unsigned int runNumber, bool allowDefault)
{
  std::string configName = (allowDefault ? "Default" : "_none_");
  float scaleFactor = 1;
  
  for ( auto rangeDescr : m_runRangeDescrs) {
    if (runNumber >= std::get<0>(rangeDescr) && runNumber < std::get<1>(rangeDescr)) {
      configName = std::get<2>(rangeDescr);
      scaleFactor = std::get<3>(rangeDescr);

      ATH_MSG_DEBUG( "ZdcInjPulserAmpMap::lookupRun():  found config name " << configName  << " for run number " 
		     << runNumber);
    }
  }


  // Find the corresponding configuration -- we hope
  //
  auto findIter = m_stepsConfigs.find(configName);
  if (findIter == m_stepsConfigs.end()) return Token();

  // Now we make a new active configuration -- the only thing that needs to be thread-protected
  //
  const std::lock_guard<std::mutex> lock(m_lock);

  // Put a new entry in the active configuration vector 
  //
  int newIndex = m_activeConfigs.size();
  m_activeConfigs.push_back(&findIter->second);

  ATH_MSG_DEBUG( "ZdcInjPulserAmpMap::lookupRun():  creating token for run number " << runNumber
		 << " with index " << newIndex << ", and scaleFactor " << scaleFactor);
  
  return Token(newIndex, scaleFactor);
}
