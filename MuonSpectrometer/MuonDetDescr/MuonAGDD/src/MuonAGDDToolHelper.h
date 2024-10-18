/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONAGDD_MUONAGDDTOOLHELPER_H
#define MUONAGDD_MUONAGDDTOOLHELPER_H

#include "GaudiKernel/StatusCode.h"

#include <vector>

namespace MuonGM 
{
  class MuonDetectorManager;
}
class IRDBAccessSvc;
class IGeoModelSvc;

class MuonAGDDToolHelper
{
public:
  MuonAGDDToolHelper();

  std::vector<std::string> ReadAGDDFlags();

  void SetNSWComponents();

  std::string GetAGDD(const bool dumpIt, const std::string& tableName, const std::string& outFileName);

  static bool BuildMScomponents() ;

  void setAGDDtoGeoSvcName(const std::string& name);

private:

  IRDBAccessSvc* m_rdbAccessSvc{nullptr};
  IGeoModelSvc* m_geoModelSvc{nullptr};
  std::string m_svcName{"AGDDtoGeoSvc"};
	
};

#endif
