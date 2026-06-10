/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//************************************************************
//
// Class PhantomBarrelSD.
// Sensitive detector for the phantom calorimeter in combined 2004
//
// Author: franck Martin <Franck.Martin@cern.ch>
// december 15th, 2003.
//
//************************************************************

#include "PhantomBarrelSD.hh"

#include "CaloIdentifier/TileTBID.h"
#include "HitManagement/HitCollectionMap.h"
#include "MCTruth/AtlasG4EventUserInfo.h"
#include "StoreGate/StoreGateSvc.h"

#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/Bootstrap.h"

#include "G4EventManager.hh"
#include "G4HCofThisEvent.hh"
#include "G4VPhysicalVolume.hh"
#include "G4Step.hh"
#include "G4VTouchable.hh"
#include "G4TouchableHistory.hh"

PhantomBarrelSD::PhantomBarrelSD(const std::string& name, const std::string& hitCollectionName)
    : G4VSensitiveDetector(name),
    m_hitCollectionName(hitCollectionName)
{
  SmartIF<StoreGateSvc> detStore{Gaudi::svcLocator()->service("DetectorStore")};
  if ( !detStore ) {
    G4ExceptionDescription description;
    description << "Constructor: DetectorStoreSvc not found!";
    G4Exception("CombinedScintillatorSD", "NoDetStore", FatalException, description);
    abort();
  } else if (verboseLevel > 5) {
    G4cout << "DetectorStoreSvc initialized" << G4endl;
  }

  if (detStore->retrieve(m_tileTBID).isFailure()) {
    G4ExceptionDescription description;
    description << "Constructor: No TileTBID helper!";
    G4Exception("CombinedScintillatorSD", "NoTileTBIDHelper", FatalException, description);
    abort();
  } else if (verboseLevel > 5) {
    G4cout << "TileTBID helper retrieved" << G4endl;
  }
  int type = TileTBID::ADC_TYPE, module = TileTBID::PHANTOM_CALO;
  for (int channel = 0; channel < NCells; ++channel) {
    m_id[channel] = m_tileTBID->channel_id(type, module, channel);
  }
}

PhantomBarrelSD::HitVectorBuilder* PhantomBarrelSD::GetHitCollection()
{
  auto* eventManager = G4EventManager::GetEventManager();
  if (!eventManager) {
    return nullptr;
  }

  auto* eventInfo = dynamic_cast<AtlasG4EventUserInfo*>(eventManager->GetUserInformation());
  if (!eventInfo) {
    return nullptr;
  }

  auto hitCollections = eventInfo->GetHitCollectionMap();
  return hitCollections ? hitCollections->Find<HitVectorBuilder>(m_hitCollectionName) : nullptr;
}

void PhantomBarrelSD::Initialize(G4HCofThisEvent* /* HCE */) {
  if (verboseLevel > 5) {
    G4cout << "PhantomBarrelSD::Initialize()" << G4endl;
  }

  m_hitCollection = GetHitCollection();
}

G4bool PhantomBarrelSD::ProcessHits(G4Step* aStep, G4TouchableHistory* /* ROhist */) {
  if (verboseLevel > 10) {
    G4cout << "PhantomBarrelSD:: ProcessHits" << G4endl;
  }

  const G4TouchableHistory* theTouchable = static_cast<const G4TouchableHistory*>(aStep->GetPreStepPoint()->GetTouchable());
  const G4VPhysicalVolume* physVol = theTouchable->GetVolume();
  const G4LogicalVolume* logiVol = physVol->GetLogicalVolume();
  const G4String nameLogiVol = logiVol->GetName();
  const G4int nScinti = physVol->GetCopyNo();

  const G4double edep = aStep->GetTotalEnergyDeposit() * aStep->GetTrack()->GetWeight();
  G4double stepl = 0.;

  if (aStep->GetTrack()->GetDefinition()->GetPDGCharge() != 0.) { // FIXME not-equal check on double

    stepl = aStep->GetStepLength();
  }

  if ((edep == 0.) && (stepl == 0.)) { //FIXME equality check on double

    return false;
  }

  if (verboseLevel > 10) {
    G4cout << "Where " << nameLogiVol << G4endl;
  }

  if (nameLogiVol.find("ScintillatorLayer") != G4String::npos) {
    int ind;
    G4double scinti = nScinti / 4.;

    if (scinti <= 1)                 ind = 0;
    if (scinti <= 2 && scinti > 1)   ind = 1;
    if (scinti <= 4 && scinti > 2)   ind = 2;
    if (scinti <= 6 && scinti > 4)   ind = 3;
    if (scinti <= 8 && scinti > 6)   ind = 4;
    if (scinti <= 10 && scinti > 8)  ind = 5;
    if (scinti <= 12 && scinti > 10) ind = 6;
    if (scinti > 12)                 ind = 7;

    HitVectorBuilder* hitCollection = m_hitCollection ? m_hitCollection : GetHitCollection();
    if (!hitCollection) {
      if (verboseLevel > 5) {
        G4cout << "PhantomBarrelSD::ProcessHits WARNING hit collection is not available" << G4endl;
      }
      return false;
    }
    m_hitCollection = hitCollection;

    if (hitCollection->HasHit(ind)) {
      if (verboseLevel > 10) {
        G4cout << "Additional hit in CombinedScintillator " << nScinti
               << " ene=" << edep << G4endl;
      }
    } else {
      // First hit in a cell
      if (verboseLevel > 10) {
        G4cout << "First hit in CombinedScintillator " << nScinti
               << " ene=" << edep << G4endl;
      }
    }
    hitCollection->AddHit(ind, m_id[ind], edep);
  }
  return true;
}
