/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#include "InDetGeoModelUtils/InDetDDAthenaComps.h"

namespace InDetDD {
  AthenaComps::AthenaComps(const std::string& msgStreamName)
    : AthMessaging(msgStreamName),
    m_detStore(nullptr),
    m_geoDbTagSvc(nullptr),
    m_rdbAccessSvc(nullptr)
  {}

  void
  AthenaComps::setDetStore(StoreGateSvc* detStore) {
    m_detStore = detStore;
  }

  void
  AthenaComps::setGeoDbTagSvc(IGeoDbTagSvc* geoDbTagSvc) {
    m_geoDbTagSvc = geoDbTagSvc;
  }

  void
  AthenaComps::setRDBAccessSvc(IRDBAccessSvc* rdbAccessSvc) {
    m_rdbAccessSvc = rdbAccessSvc;
  }

}
