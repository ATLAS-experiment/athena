/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


// Class header
#include "LUCID_SensitiveDetector.h"

// Package headers
#include "LUCID_HitHelper.h"

// Athena headers
#include "HitManagement/HitCollectionMap.h"
#include "LUCID_GeoModel/LUCID_Constants.h"
#include "MCTruth/AtlasG4EventUserInfo.h"
#include "MCTruth/TrackHelper.h"


// Geant4 headers
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4VProcess.hh"
#include "G4OpticalPhoton.hh"

// CLHEP headers
#include "CLHEP/Units/PhysicalConstants.h"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

LUCID_SensitiveDetector::LUCID_SensitiveDetector(const std::string& name, const std::string& hitCollectionName)
  : G4VSensitiveDetector( name )
  , m_hitCollectionName( hitCollectionName )
{
  m_hit = new LUCID_HitHelper();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
// Initialize from G4.
void LUCID_SensitiveDetector::Initialize(G4HCofThisEvent *)
{
  m_HitColl = getHitCollection();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

bool LUCID_SensitiveDetector::ProcessHits(G4Step* aStep, G4TouchableHistory*) {
  if (!m_HitColl) {
    m_HitColl = getHitCollection();
    if (!m_HitColl) {
      return false;
    }
  }

  if (verboseLevel>5)
    {
      G4cout << "LUCID_SensitiveDetector::ProcessHits - Begin" << G4endl;
    }
  G4Track* aTrack = aStep->GetTrack();

  if (aTrack->GetDefinition() != G4OpticalPhoton::OpticalPhotonDefinition()) return false;

  if (verboseLevel>5)
    {
      G4cout << "LUCID_SensitiveDetector::ProcessHits(): There is an OpticalPhoton " << G4endl;
    }

  aTrack->SetTrackStatus(fKillTrackAndSecondaries);

  if (aTrack->GetCreatorProcess()->GetProcessName() != "Cerenkov") return false;

  if (verboseLevel>5)
    {
      G4cout << "LUCID_SensitiveDetector::ProcessHits(): It is from a Cerenkov process "  << G4endl;
    }

  TrackHelper trHelp(aTrack);
  double energy = aTrack->GetKineticEnergy()/CLHEP::eV;
  double lambda = m_hit->GetWaveLength(energy);

  m_HitColl->Emplace(m_hit->GetTubNumber(aStep),
                     aTrack->GetDefinition()->GetPDGEncoding(),
                     trHelp.GenerateParticleLink(),
                     LUCID_HitHelper::GetVolNumber    (aTrack->GetLogicalVolumeAtVertex()->GetName()),
                     m_hit->GetPreStepPoint (aStep).x(),
                     m_hit->GetPreStepPoint (aStep).y(),
                     m_hit->GetPreStepPoint (aStep).z(),
                     m_hit->GetPostStepPoint(aStep).x(),
                     m_hit->GetPostStepPoint(aStep).y(),
                     m_hit->GetPostStepPoint(aStep).z(),
                     m_hit->GetPreStepTime  (aStep),
                     m_hit->GetPostStepTime (aStep),
                     lambda,
                     energy);
  return true;
}

LUCID_SimHitCollection* LUCID_SensitiveDetector::getHitCollection() const
{
  auto* eventInfo = AtlasG4EventUserInfo::GetEventUserInfo();
  if (!eventInfo) {
    return nullptr;
  }
  auto hitCollections = eventInfo->GetHitCollectionMap();
  return hitCollections ? hitCollections->Find<LUCID_SimHitCollection>(m_hitCollectionName) : nullptr;
}
