/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//
// Pixel Sensitive Detector - specialisation for GeoModelXml
// The Hits are processed here. For every hit I get the position and
// an information on the sensor in which the interaction happened
//

// Class header
#include "PixelSensorGmxSD.h"

// Athena headers
#include "MCTruth/AtlasG4EventUserInfo.h"
#include "MCTruth/TrackHelper.h"

// Geant4 headers
#include "G4ChargedGeantino.hh"
#include <G4EventManager.hh>
#include "G4Event.hh"
#include "G4Geantino.hh"
#include "G4SDManager.hh"
#include "G4Step.hh"
#include "G4ThreeVector.hh"

// CLHEP headers
#include "CLHEP/Geometry/Transform3D.h"
#include "CLHEP/Units/SystemOfUnits.h"

#include <GeoModelKernel/GeoFullPhysVol.h>
#include <GeoModelRead/ReadGeoModel.h>

#include <InDetSimEvent/SiHitIdHelper.h>

namespace {
bool isBCMPrimeHitCollectionName(const std::string& name)
{
  return name == "BCMPrimeHits" || name == "BCMPrimeHits_G4";
}

SiHitCollection* findBCMPrimeHitCollection(AtlasG4EventUserInfo* eventInfo,
                                          const std::string& primaryName)
{
  if (!eventInfo) {
    return nullptr;
  }
  auto hitMap = eventInfo->GetHitCollectionMap();
  if (!hitMap) {
    return nullptr;
  }
  if (auto* coll = hitMap->Find<SiHitCollection>(primaryName)) {
    return coll;
  }
  if (primaryName == "BCMPrimeHits") {
    return hitMap->Find<SiHitCollection>("BCMPrimeHits_G4");
  }
  if (primaryName == "BCMPrimeHits_G4") {
    return hitMap->Find<SiHitCollection>("BCMPrimeHits");
  }
  return nullptr;
}
} // namespace


PixelSensorGmxSD::PixelSensorGmxSD(const std::string& name, const std::string& hitCollectionName,GeoModelIO::ReadGeoModel * sqlreader)
  : G4VSensitiveDetector( name )
  , m_HitCollName( hitCollectionName )
{
    m_sqlreader = sqlreader;
    if (isBCMPrimeHitCollectionName(m_HitCollName)) {
      G4cout << "BCMPrimeSensorSD diagnostic: constructed PixelSensorGmxSD name=" << name
             << " collection=" << m_HitCollName
             << " sqlreader=" << (m_sqlreader ? "set" : "null") << G4endl;
    }
}

// Initialize from G4 - cache the hit collection for the current event
void PixelSensorGmxSD::Initialize(G4HCofThisEvent *)
{
  m_HitColl = nullptr;
  m_g4UserEventInfo = nullptr;

  // ISF calls G4SDManager::PrepareNewEvent() before the Geant4 event loop starts...
  auto* eventManager = G4EventManager::GetEventManager();
  auto* eventInfo = eventManager
                        ? static_cast<AtlasG4EventUserInfo*>(eventManager->GetUserInformation())
                        : nullptr;
  if (eventInfo) {
    m_g4UserEventInfo = eventInfo;
    m_HitColl = findBCMPrimeHitCollection(eventInfo, m_HitCollName);
  }

  if (isBCMPrimeHitCollectionName(m_HitCollName) && m_bcmPrimeInitDiagCount < 3) {
    const int eventNumber = eventManager && eventManager->GetConstCurrentEvent()
                              ? eventManager->GetConstCurrentEvent()->GetEventID()
                              : -1;
    G4cout << "BCMPrimeSensorSD diagnostic: Initialize event=" << eventNumber
           << " requestedCollection=" << m_HitCollName
           << " eventInfo=" << (eventInfo ? "set" : "null")
           << " hitMap=" << (eventInfo && eventInfo->GetHitCollectionMap() ? "set" : "null")
           << " hitCollection=" << (m_HitColl ? "set" : "null");
    if (auto hitMap = eventInfo ? eventInfo->GetHitCollectionMap() : nullptr) {
      G4cout << " lookupBCMPrimeHits="
             << (hitMap->Find<SiHitCollection>("BCMPrimeHits") ? "set" : "null")
             << " lookupBCMPrimeHits_G4="
             << (hitMap->Find<SiHitCollection>("BCMPrimeHits_G4") ? "set" : "null")
             << " lookupITkPixelHits="
             << (hitMap->Find<SiHitCollection>("ITkPixelHits") ? "set" : "null")
             << " lookupPLR_Hits="
             << (hitMap->Find<SiHitCollection>("PLR_Hits") ? "set" : "null");
    }
    G4cout << G4endl;
    ++m_bcmPrimeInitDiagCount;
  }

}


G4bool PixelSensorGmxSD::ProcessHits(G4Step* aStep, G4TouchableHistory* /*ROhist*/)
{
  if (isBCMPrimeHitCollectionName(m_HitCollName) && !m_reportedBCMPrimeHit) {
    G4cout << "BCMPrimeSensorSD diagnostic: first ProcessHits call for "
           << m_HitCollName << G4endl;
    m_reportedBCMPrimeHit = true;
  }
  if (verboseLevel>5) G4cout << "Process Hit" << G4endl;

  G4double edep = aStep->GetTotalEnergyDeposit();
  edep *= CLHEP::MeV;
  if(edep==0.) {
    if(aStep->GetTrack()->GetDefinition() != G4Geantino::GeantinoDefinition() &&
       aStep->GetTrack()->GetDefinition() != G4ChargedGeantino::ChargedGeantinoDefinition())
      return false;
  }

  //use the global time. i.e. the time from the beginning of the event
  //
  // Get the Touchable History:
  //
  const G4TouchableHistory *myTouch = dynamic_cast<const G4TouchableHistory*>(aStep->GetPreStepPoint()->GetTouchable());
  if (not myTouch) {
    G4cout << "PixelSensorGmxSD::ProcessHits bad dynamic_cast" << G4endl;
    return false;
  }
  if(verboseLevel>5){
    for (int i=0;i<myTouch->GetHistoryDepth();i++){
      std::string detname=myTouch->GetVolume(i)->GetLogicalVolume()->GetName();
      int copyno=myTouch->GetVolume(i)->GetCopyNo();
      G4cout << "Volume " <<detname <<" Copy Nr. " << copyno << G4endl;
    }
  }
  //
  // Get the hit coordinates. Start and End Point
  //
  G4ThreeVector coord1 = aStep->GetPreStepPoint()->GetPosition();
  G4ThreeVector coord2 = aStep->GetPostStepPoint()->GetPosition();

  // Calculate the local step begin and end position.
  // From a G4 FAQ:
  // http://geant4-hn.slac.stanford.edu:5090/HyperNews/public/get/geometry/17/1.html
  //
  const G4AffineTransform transformation = myTouch->GetHistory()->GetTopTransform();
  G4ThreeVector localPosition1 = transformation.TransformPoint(coord1);
  G4ThreeVector localPosition2 = transformation.TransformPoint(coord2);

  HepGeom::Point3D<double> lP1,lP2;
  lP1[SiHit::xEta] = localPosition1[2]*CLHEP::mm;
  lP1[SiHit::xPhi] = localPosition1[1]*CLHEP::mm;
  lP1[SiHit::xDep] = localPosition1[0]*CLHEP::mm;

  lP2[SiHit::xEta] = localPosition2[2]*CLHEP::mm;
  lP2[SiHit::xPhi] = localPosition2[1]*CLHEP::mm;
  lP2[SiHit::xDep] = localPosition2[0]*CLHEP::mm;

  TrackHelper trHelp(aStep->GetTrack());
  auto mcParticleLink = trHelp.GenerateParticleLink(m_g4UserEventInfo ? m_g4UserEventInfo->GetEventStore() : nullptr);

    if(m_sqlreader){
        //if sqlite inputs, Identifier indices come from PhysVol Name  
        std::string physVolName = myTouch->GetVolume(0)->GetName();

        int hitIdOfWafer = SiHitIdHelper::GetHelper()->buildHitIdFromStringITk(0,physVolName);
                                                           
 
        m_HitColl->Emplace(lP1,
                     lP2,
                     edep,
                     aStep->GetPreStepPoint()->GetGlobalTime(),//use the global time. i.e. the time from the beginning of the event
                     std::move(mcParticleLink),
                     hitIdOfWafer);
        return true;
        
    }
    // if not from SQLite, we assume that the Identifier has already been written in as the copy number 
  // (it should have done if GeoModel building ran within Athena)

  int id = myTouch->GetVolume()->GetCopyNo();

  m_HitColl->Emplace(lP1,
                     lP2,
                     edep,
                     aStep->GetPreStepPoint()->GetGlobalTime(),
                     std::move(mcParticleLink),
                     id);
  return true; 
}
