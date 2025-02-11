/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEO2G4_Geo2G4SolidFactory_h
#define GEO2G4_Geo2G4SolidFactory_h

#include <map>
#include <string>

#include "GaudiKernel/ServiceHandle.h"

#include "AthenaBaseComps/AthMessaging.h"
#include "CxxUtils/checker_macros.h"
#include "StoreGate/StoreGateSvc.h"
#include "LArWheelSolid_type.h"

class G4VSolid;
class GeoShape;
class GeoUnidentifiedShape;
struct EMECData;
class Geo2G4SolidFactory : public AthMessaging
{
public:
  	typedef ServiceHandle<StoreGateSvc> StoreGateSvc_t;
  	typedef std::pair<LArWheelSolid_t, int> LArWheelSolidDef_t;
  	typedef std::map<std::string,  LArWheelSolidDef_t> LArWheelSolid_typemap;

  Geo2G4SolidFactory();
  G4VSolid* Build ATLAS_NOT_THREAD_SAFE (const GeoShape*, const std::string & name="");

private:
   G4VSolid* createLArWheelSolid ATLAS_NOT_THREAD_SAFE (const std::string& name, const LArWheelSolidDef_t & lwsdef, const EMECData &emecData) const;
   G4VSolid* createLArWheelSliceSolid ATLAS_NOT_THREAD_SAFE (const GeoUnidentifiedShape* ,const EMECData &emecData) const;

   static const LArWheelSolid_typemap s_lwsTypes;
   /// Pointer to StoreGate (detector store by default)
   StoreGateSvc_t m_detStore;
};

#endif
