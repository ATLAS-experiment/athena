/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
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

  // std::string filePath = PathResolver::find_file("ZDC_InjectPulseSteps.json","DATAPATH", PathResolver::RecursiveSearch);
  std::string filePath = PathResolverFindCalibFile("ZdcConditions/ZDC_InjectPulseSteps.json");

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
  ReadPulserSteps(ifs);
}

void ZdcInjPulserAmpMap::ReadPulserSteps(std::ifstream& ifs)
{
  nlohmann::json j = nlohmann::json::parse(ifs);

  m_LBPulserVMap.clear();
  
  // Get the starting LB entry in the json if it exists
  //
  nlohmann::json first = j[0];
  
  if (first.find("startLB") != first.end()) {
    m_firstLB = first["startLB"];
    j.erase(j.begin());
  }

  for (auto entry : j) {
    FillVVector(entry);
  }
}

void ZdcInjPulserAmpMap::FillVVector(const nlohmann::json& entry)
{
  unsigned int nStep = entry.at("nStep");
  float vStart = entry.at("vStart");
  float vStep = entry.at("vStep");

  float voltage = vStart;
  for (size_t step = 0; step < nStep; step++) {
    m_LBPulserVMap.push_back(voltage);
    voltage += vStep;
  }
}

