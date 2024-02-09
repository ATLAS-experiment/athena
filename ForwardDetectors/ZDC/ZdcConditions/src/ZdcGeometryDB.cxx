/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "ZdcConditions/ZdcGeometryDB.h"
#include "PathResolver/PathResolver.h"

#include <fstream>

const ZdcGeometryDB* ZdcGeometryDB::getInstance()
{
  static const ZdcGeometryDB map_run3;
  return &map_run3;
}

//constructor
ZdcGeometryDB::ZdcGeometryDB() : asg::AsgMessaging("ZdcGeometryDB") 
{

  msg().setLevel(MSG::INFO);

  loadJSONFile();

}

void ZdcGeometryDB::loadJSONFile(const std::string& geoStr)
{

  std::string filePath = PathResolver::find_file(geoStr,"DATAPATH", PathResolver::RecursiveSearch);
  if (!filePath.empty())
    {
      ATH_MSG_DEBUG( "ZdcGeometryDB::found ZDC JSON at " << filePath );
    }
  else
    {
      ATH_MSG_DEBUG(  "ZdcGeometryDB::geometry NOT FOUND!" ) ;
      filePath = "./"+m_geoStr;
    }

  std::ifstream ifs(filePath);

  if (ifs.is_open())
    ifs >> m_mainJson;
  else
    ATH_MSG_ERROR("No ZDC geometry found");

}
