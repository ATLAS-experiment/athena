/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "ZdcConditions/ZdcGeometryDB.h"
#include "GaudiKernel/Bootstrap.h"
#include "PathResolver/PathResolver.h"
#include <GaudiKernel/ISvcLocator.h>
#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "RDBAccessSvc/IRDBRecordset.h"
#include "RDBAccessSvc/IRDBRecord.h"

#include <fstream>


const IZdcGeometryDB* ZdcFileGeometryDB::getInstance()
{
  static const ZdcFileGeometryDB file_db_run3;
  return &file_db_run3;
}

const IZdcGeometryDB* ZdcGeoDBGeometryDB::getInstance()
{
  static const ZdcGeoDBGeometryDB geodb_run3;
  return &geodb_run3;
}

//constructor
ZdcFileGeometryDB::ZdcFileGeometryDB() :  asg::AsgMessaging("ZdcFileGeometryDB"),m_fileStr("ZDCgeom_Run3.json")
{

  msg().setLevel(MSG::INFO);

  m_geoLoaded = loadJSONFile();

}

ZdcGeoDBGeometryDB::ZdcGeoDBGeometryDB() :  asg::AsgMessaging("ZdcGeoDBGeometryDB")
{
  msg().setLevel(MSG::INFO);

  m_geoLoaded = loadGeoDB();
}


bool ZdcFileGeometryDB::loadJSONFile()
{
  bool ret = false;
  if (m_fileStr == "")
    {
      ATH_MSG_WARNING("No JSON filename defined!");
      return ret;
    }
  std::string filePath = PathResolver::find_file(m_fileStr,"DATAPATH", PathResolver::RecursiveSearch);
  if (!filePath.empty())
    {
      ATH_MSG_DEBUG( "ZdcGeometryDB::found ZDC JSON at " << filePath );
    }
  else
    {
      ATH_MSG_DEBUG(  "ZdcGeometryDB::geometry NOT FOUND!  Trying local file!" ) ;
      filePath = "./"+m_fileStr;
    }

  std::ifstream ifs(filePath);

  if (ifs.is_open())
    {
      ifs >> m_mainJson;
      ret = true;
      ATH_MSG_INFO("Loaded ZDC DB from file");
    }
  else
    ATH_MSG_ERROR("No ZDC geometry found");

  return ret;
}

bool ZdcGeoDBGeometryDB::loadGeoDB()
{
  SmartIF<IRDBAccessSvc> iAccessSvc(Gaudi::svcLocator()->service("RDBAccessSvc"));
  if (!iAccessSvc)
    {
      ATH_MSG_FATAL("No RDBAccessSvc");
      throw std::runtime_error("No RDBAccessSvc found!");
    }
  //DecodeVersionKey atlasVersion("ATLAS");
  //const std::string& AtlasVersion = atlasVersion.tag();
  IRDBRecordset_ptr tanParams = iAccessSvc->getRecordsetPtr("TanTaxn","TanTaxn-00");
  IRDBRecordset_ptr zdcParams = iAccessSvc->getRecordsetPtr("ZdcPrd","ZdcPrd-00");

  IRDBRecordset::const_iterator AccessSvc_iter;
  for(AccessSvc_iter = tanParams->begin(); AccessSvc_iter != tanParams->end(); ++AccessSvc_iter)
    {
      std::string name = (*AccessSvc_iter)->getString("NAME");
      int side = (*AccessSvc_iter)->getInt("SIDE");
      double x = (*AccessSvc_iter)->getDouble("X");
      double y = (*AccessSvc_iter)->getDouble("Y");
      double z = (*AccessSvc_iter)->getDouble("Z");
      double w = (*AccessSvc_iter)->getDouble("WIDTH");
      double h = (*AccessSvc_iter)->getDouble("HEIGHT");
      double d = (*AccessSvc_iter)->getDouble("DEPTH");
      
      m_mainJson["TAN/TAXN"][name]["name"]=name;
      m_mainJson["TAN/TAXN"][name]["side"]=side;
      m_mainJson["TAN/TAXN"][name]["x"]=x;
      m_mainJson["TAN/TAXN"][name]["y"]=y;
      m_mainJson["TAN/TAXN"][name]["z"]=z;
      m_mainJson["TAN/TAXN"][name]["width"]=w;
      m_mainJson["TAN/TAXN"][name]["height"]=h;
      m_mainJson["TAN/TAXN"][name]["depth"]=d;
    }
  
  for(AccessSvc_iter = zdcParams->begin(); AccessSvc_iter != zdcParams->end(); ++AccessSvc_iter)
    {
      std::string name = (*AccessSvc_iter)->getString("NAME");
      std::string key = name;
      if (key.substr(0,2)=="12" || key.substr(0,2)=="81")
	key = "ZDC"+key;
      int side = (*AccessSvc_iter)->getInt("SIDE");
      int mod = (*AccessSvc_iter)->getInt("MODUL");
      int type = -1;
      if (!(*AccessSvc_iter)->isFieldNull("TYPE"))
	{
	  type = (*AccessSvc_iter)->getInt("TYPE");
	}
      double x = (*AccessSvc_iter)->getDouble("X");
      double y = (*AccessSvc_iter)->getDouble("Y");
      double z = (*AccessSvc_iter)->getDouble("Z");
      double q = (*AccessSvc_iter)->getDouble("Q");
      double i = (*AccessSvc_iter)->getDouble("I");
      double j = (*AccessSvc_iter)->getDouble("J");
      double k = (*AccessSvc_iter)->getDouble("K");
      
      m_mainJson["Detector"][key]["name"]=name;
      m_mainJson["Detector"][key]["module"]=mod;
      m_mainJson["Detector"][key]["side"]=side;
      if (type>-1) m_mainJson["Detector"][key]["type"]=type; // RPD & BRAN have no "type"
      m_mainJson["Detector"][key]["x"]=x;
      m_mainJson["Detector"][key]["y"]=y;
      m_mainJson["Detector"][key]["z"]=z;
      m_mainJson["Detector"][key]["q"]=q;
      m_mainJson["Detector"][key]["i"]=i;
      m_mainJson["Detector"][key]["j"]=j;
      m_mainJson["Detector"][key]["k"]=k;
    }

  ATH_MSG_INFO("Loaded ZDC DB from GeoDB");
  return true;
}


