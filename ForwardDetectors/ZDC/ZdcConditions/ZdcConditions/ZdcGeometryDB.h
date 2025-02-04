/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDCGEOMETRYDB_H
#define ZDCGEOMETRYDB_H

#include <nlohmann/json.hpp>
#include <iostream>
#include "AsgMessaging/AsgMessaging.h"

class IZdcGeometryDB  
{

protected:
  bool m_geoLoaded;
  nlohmann::json m_mainJson;

public:
  IZdcGeometryDB(){};

  const nlohmann::json& getDB() const {return m_mainJson;};

};

class ZdcFileGeometryDB : public IZdcGeometryDB, asg::AsgMessaging
{

  std::string m_fileStr;

public:
  ZdcFileGeometryDB();
  bool loadJSONFile();
  static const IZdcGeometryDB* getInstance();
  
};

class ZdcGeoDBGeometryDB : public IZdcGeometryDB, asg::AsgMessaging
{

public:
  ZdcGeoDBGeometryDB();
  bool loadGeoDB();
  static const IZdcGeometryDB* getInstance();
  
};


#endif //ZDCGEOMETRYDB_H
