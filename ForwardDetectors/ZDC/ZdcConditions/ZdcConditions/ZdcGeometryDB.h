/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDCGEOMETRYDB_H
#define ZDCGEOMETRYDB_H

#include <nlohmann/json.hpp>
#include <iostream>
#include "AsgMessaging/AsgMessaging.h"

class ZdcGeometryDB : public asg::AsgMessaging
{

private:
  nlohmann::json m_mainJson;
  std::string m_geoStr;

public:
  ZdcGeometryDB();

  static const ZdcGeometryDB* getInstance();
  const nlohmann::json& getDB() const {return m_mainJson;};
  void loadJSONFile(std::string geoStr="ZDCgeom_Run3.json");

};

#endif //ZDCGEOMETRYDB_H
