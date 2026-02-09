/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef InDetGeoModelUtils_InDetDDAthenaComps_H
#define InDetGeoModelUtils_InDetDDAthenaComps_H

#include "AthenaBaseComps/AthMessaging.h"
#include "CxxUtils/checker_macros.h"
#include <string>

class  StoreGateSvc;
class  IGeoDbTagSvc;
class  IRDBAccessSvc;

namespace InDetDD {

/// Class to hold various Athena components.
class AthenaComps
  : public AthMessaging
{
public:

  AthenaComps(const std::string & msgStreamName);

  void setDetStore(StoreGateSvc *);
  void setGeoDbTagSvc(IGeoDbTagSvc *);
  void setRDBAccessSvc(IRDBAccessSvc *);

  const StoreGateSvc * detStore() const;
  const IGeoDbTagSvc * geoDbTagSvc() const;
  
  StoreGateSvc * detStore();
  IGeoDbTagSvc * geoDbTagSvc();
  IRDBAccessSvc * rdbAccessSvc();
  
private:
  StoreGateSvc * m_detStore;
  IGeoDbTagSvc * m_geoDbTagSvc;
  IRDBAccessSvc * m_rdbAccessSvc;
};

inline StoreGateSvc * AthenaComps::detStore()
{
  return m_detStore;
}

inline const StoreGateSvc * AthenaComps::detStore() const
{
  return m_detStore;
}

inline const IGeoDbTagSvc * AthenaComps::geoDbTagSvc() const
{
  return m_geoDbTagSvc;
}

inline IGeoDbTagSvc * AthenaComps::geoDbTagSvc()
{
  return m_geoDbTagSvc;
}


inline IRDBAccessSvc * AthenaComps::rdbAccessSvc()
{
  return m_rdbAccessSvc;
}

} // endnamespace

#endif // InDetGeoModelUtils_InDetDDAthenaComps_H

