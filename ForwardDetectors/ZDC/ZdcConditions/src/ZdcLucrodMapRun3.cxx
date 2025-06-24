/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "ZdcConditions/ZdcLucrodMapRun3.h"
#include "PathResolver/PathResolver.h"

#include <fstream>

const ZdcLucrodMapRun3* ZdcLucrodMapRun3::getInstance()
{
  static const ZdcLucrodMapRun3 map_run3;
  return &map_run3;
}

//constructor
ZdcLucrodMapRun3::ZdcLucrodMapRun3() : asg::AsgMessaging("ZdcLucrodMapRun3") 
{
  msg().setLevel(MSG::INFO);

  std::string filePath = PathResolverFindDataFile("ZdcConditions/ZDC_Run3_conditions.json");
  if (!filePath.empty())
    {
      ATH_MSG_DEBUG( "ZdcLucrodMapRun3::found ZDC JSON at " << filePath );
    }
  else
    {
      ATH_MSG_DEBUG(  "ZdcLucrodMapRun3::mapping NOT FOUND!" ) ;
      filePath = "./ZDC_Run3_mapping.json";
    }

  std::ifstream ifs(filePath);
 
  ifs >> m_mainJson;

  m_lucrodInfo.resize(6);

  for (const auto& element: m_mainJson)
    {
      int source_ID = element["source_ID"] ; 
      m_lucrodInfo[source_ID] = element;
    }

}
