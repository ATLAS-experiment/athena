/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//************************************************************
//
// Class CombinedScintillatorSD.
// Sensitive detector for the Scintillator between LAr/Tile
//
//************************************************************

#include "CombinedScintillatorSD.hh"

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

CombinedScintillatorSD::CombinedScintillatorSD(const std::string& name, const std::string& hitCollectionName)
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
  int type = TileTBID::ADC_TYPE, module = TileTBID::CRACK_WALL;
  for (int channel = 0; channel < NCells; ++channel) {
    m_id[channel] = m_tileTBID->channel_id(type, module, channel);
  }
}

CombinedScintillatorSD::HitVectorBuilder* CombinedScintillatorSD::GetHitCollection()
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

void CombinedScintillatorSD::Initialize(G4HCofThisEvent* /* HCE */) {
  if (verboseLevel > 5) {
    G4cout << "CombinedScintillatorSD::Initialize()" << G4endl;
  }

  m_hitCollection = GetHitCollection();
}

G4bool CombinedScintillatorSD::ProcessHits(G4Step* aStep, G4TouchableHistory* /* ROhist */) {
  if (verboseLevel > 10) {
    G4cout << "CombinedScintillatorSD::ProcessHits" << G4endl;
  }

  const G4TouchableHistory* theTouchable = static_cast<const G4TouchableHistory*>(aStep->GetPreStepPoint()->GetTouchable());
  const G4VPhysicalVolume* physVol = theTouchable->GetVolume();
  const G4LogicalVolume* logiVol = physVol->GetLogicalVolume();
  const G4String nameLogiVol = logiVol->GetName();
  const G4int nScinti = physVol->GetCopyNo();

  const G4double edep = aStep->GetTotalEnergyDeposit() * aStep->GetTrack()->GetWeight();
  G4double stepl = 0.;

  if (aStep->GetTrack()->GetDefinition()->GetPDGCharge() != 0.) {// FIXME not-equal check on double

    stepl = aStep->GetStepLength();
  }

  if ((edep == 0.) && (stepl == 0.)) {//FIXME equality check on double

    return false;
  }

  if (nameLogiVol.find("CScintillatorLayer") != G4String::npos) {
    int ind = nScinti; // Only one scintillator at the test beam
    // No copy nScinti == 0 !!

    HitVectorBuilder* hitCollection = m_hitCollection ? m_hitCollection : GetHitCollection();
    if (!hitCollection) {
      if (verboseLevel > 5) {
        G4cout << "CombinedScintillatorSD::ProcessHits WARNING hit collection is not available" << G4endl;
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
