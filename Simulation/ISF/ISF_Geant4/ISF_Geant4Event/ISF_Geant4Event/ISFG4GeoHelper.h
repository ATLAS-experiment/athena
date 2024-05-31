/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_GEANT4EVENT_ISFG4GEOHELPER_H
#define ISF_GEANT4EVENT_ISFG4GEOHELPER_H


#include "AtlasDetDescr/AtlasRegion.h"


class G4Step;
class G4LogicalVolume;
class G4StepPoint;


namespace ISF{
  class IGeoIDSvc;
}


namespace iGeant4 {

class ISFG4GeoHelper {

 public:
  ISFG4GeoHelper() = delete;

  static AtlasDetDescr::AtlasRegion nextGeoId(const G4Step* aStep, int truthVolLevel,ISF::IGeoIDSvc *geoIDSvc);
  static bool checkVolumeDepth(G4LogicalVolume* logicalVol, int volLevel, int depth=0);

  /// get the next GeoID using only the geoIDSvc
  static AtlasDetDescr::AtlasRegion getNextGeoIDFromSvc(const G4StepPoint& postStep, const ISF::IGeoIDSvc &geoIDSvc);

};

}
#endif
