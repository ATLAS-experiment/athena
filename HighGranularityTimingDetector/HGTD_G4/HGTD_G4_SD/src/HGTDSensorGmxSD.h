/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////
// HGTDSensorSD.h, (c) ATLAS Detector software
//////////////////////////////////////////////////////////

#ifndef HGTD_G4_Gmx_SD_HGTDSENSORSD_H
#define HGTD_G4_Gmx_SD_HGTDSENSORSD_H

// Base class
#include "G4VSensitiveDetector.hh"

//new LAr stuff
#include "HGTD_Identifier/HGTD_ID.h"
#include "InDetSimEvent/SiHitCollection.h"

// Geant4 headers
#include "G4ThreeVector.hh"

#include <string>

// G4 needed classes
class G4Step;
class G4HCofThisEvent;

namespace GeoModelIO{
  class ReadGeoModel;
}

class HGTDSensorGmxSD : public G4VSensitiveDetector
{

public:

    // Constructor
    HGTDSensorGmxSD(const std::string& name, const std::string& hitCollectionName, GeoModelIO::ReadGeoModel * sqlreader=nullptr);

    // Destructor
    virtual ~HGTDSensorGmxSD() {}

    // Process the hits from G4
    G4bool ProcessHits(G4Step*, G4TouchableHistory*) override final;

    // Initialize from G4
    void Initialize(G4HCofThisEvent *) override final;

private:

    SiHitCollection* getHitCollection() const;

    std::string m_hitCollectionName;
    // Non-owning cache set by Initialize; HitCollectionMap owns the collection.
    SiHitCollection* m_hitColl{};
    GeoModelIO::ReadGeoModel * m_sqlreader;

};

#endif // HGTD_G4_Gmx_SD_HGTDSENSORSD_H
