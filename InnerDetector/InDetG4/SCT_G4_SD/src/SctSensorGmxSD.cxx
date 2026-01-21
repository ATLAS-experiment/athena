/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//
// SCT Sensitive Detectors where the sensitive volume identifier is already the required identifier
// (previous versions built up the identifier from the parent volume numeric identifiers)
// GeoModelXml introduced this way of doing things.
//
// The Geant 4 Hits are processed here. For every hit: get the local start and end points, 
// energy deposit, hit time, barcode, and identifier;  put these in a list to be persistified.
//

// class headers
#include "SctSensorGmxSD.h"

// athena includes
#include "MCTruth/AtlasG4EventUserInfo.h"
#include "MCTruth/TrackHelper.h"

// Geant4 includes
#include "G4Step.hh"
#include "G4ThreeVector.hh"
#include "G4SDManager.hh"
#include "G4Geantino.hh"
#include "G4ChargedGeantino.hh"

#include <GeoModelKernel/GeoFullPhysVol.h>
#include <GeoModelRead/ReadGeoModel.h>

#include <InDetSimEvent/SiHitIdHelper.h>

SctSensorGmxSD::SctSensorGmxSD(const std::string& name, const std::string& hitCollectionName,GeoModelIO::ReadGeoModel * sqlreader)
  : SctSensorSD( name, hitCollectionName) {
      m_sqlreader = sqlreader;
   }

SctSensorGmxSD::~SctSensorGmxSD() { }

G4bool SctSensorGmxSD::ProcessHits(G4Step *aStep, G4TouchableHistory * /* not used */) {
  double edep = aStep->GetTotalEnergyDeposit();
  if (edep == 0.) {
    if (aStep->GetTrack()->GetDefinition() != G4Geantino::GeantinoDefinition() &&
       aStep->GetTrack()->GetDefinition() != G4ChargedGeantino::ChargedGeantinoDefinition())
      return false;
  }
  edep *= CLHEP::MeV;

  const G4TouchableHistory *myTouch = dynamic_cast<const G4TouchableHistory*>(aStep->GetPreStepPoint()->GetTouchable());
  if (not myTouch) {
    G4cout << "SctSensorGmxSD::ProcessHits bad dynamic_cast" << G4endl;
    return false;
  }
  //
  // Get the hit start and end point local coordinates
  //
  G4ThreeVector coord1 = aStep->GetPreStepPoint()->GetPosition();
  G4ThreeVector coord2 = aStep->GetPostStepPoint()->GetPosition();
  const G4AffineTransform transformation = myTouch->GetHistory()->GetTopTransform();
  G4ThreeVector localPosition1 = transformation.TransformPoint(coord1);
  G4ThreeVector localPosition2 = transformation.TransformPoint(coord2);
  HepGeom::Point3D<double> lP1, lP2;


  lP1[SiHit::xEta] = localPosition1[2]*CLHEP::mm;
  lP1[SiHit::xPhi] = localPosition1[1]*CLHEP::mm;
  lP1[SiHit::xDep] = localPosition1[0]*CLHEP::mm;

  lP2[SiHit::xEta] = localPosition2[2]*CLHEP::mm;
  lP2[SiHit::xPhi] = localPosition2[1]*CLHEP::mm;
  lP2[SiHit::xDep] = localPosition2[0]*CLHEP::mm;

  
  // get the HepMcParticleLink from the TrackHelper
  TrackHelper trHelp(aStep->GetTrack());
  auto particleLink = trHelp.GenerateParticleLink(m_g4UserEventInfo ? m_g4UserEventInfo->GetEventStore() : nullptr);

  if(m_sqlreader){
    //if sqlite inputs, Identifier indices come from PhysVol Name  
    std::string physVolName = myTouch->GetVolume(0)->GetName();
  
    int hitIdOfWafer = SiHitIdHelper::GetHelper()->buildHitIdFromStringITk(1,physVolName);                                                   
 
    m_HitColl->Emplace(lP1,
                    lP2,
                    edep,
                    aStep->GetPreStepPoint()->GetGlobalTime(),//use the global time. i.e. the time from the beginning of the event
                    std::move(particleLink),
                    hitIdOfWafer);
    return true;
  }

  // if not from SQLite, we assume that the Identifier has already been written in as the copy number 
  // (it should hsave done if GeoModel building ran within Athena)
  //
  //    Get the indexes of which detector the hit is in
  //
  const int id = myTouch->GetVolume(0)->GetCopyNo();

  m_HitColl->Emplace(lP1, lP2, edep, aStep->GetPreStepPoint()->GetGlobalTime(), std::move(particleLink), id);

  return true;
}
