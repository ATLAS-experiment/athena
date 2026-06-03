/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Class header
#include "ALFA_SensitiveDetector.h"

// Athena headers
#include "HitManagement/HitCollectionMap.h"
#include "MCTruth/AtlasG4EventUserInfo.h"

// Geant4 headers
#include "G4ParticleDefinition.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4TouchableHistory.hh"
#include "G4ThreeVector.hh"
#include "G4Track.hh"

// STL header
#include <sstream>
#include <limits>
#include <cmath>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

ALFA_SensitiveDetector::ALFA_SensitiveDetector(const std::string& name, const std::string& hitCollectionName, const std::string& ODhitCollectionName)
  : G4VSensitiveDetector( name ),
    m_hitCollectionName(hitCollectionName),
    m_ODHitCollectionName(ODhitCollectionName)
{

  m_hitID = -1;

  m_pos1 = 0;
  m_pos2 = 0;

  m_num[0] = 0;
  m_num[1] = 0;
  m_num[2] = 0;

}

// Initialize from G4.
void ALFA_SensitiveDetector::Initialize(G4HCofThisEvent *)
{
  m_HitCollection = getHitCollection();
  m_ODHitCollection = getODHitCollection();
}

bool ALFA_SensitiveDetector::ProcessHits(G4Step* pStep, G4TouchableHistory*)
{
  if (!m_HitCollection) {
    m_HitCollection = getHitCollection();
  }
  if (!m_ODHitCollection) {
    m_ODHitCollection = getODHitCollection();
  }

  const double energyDeposit(pStep->GetTotalEnergyDeposit());

  // Get kinetic energy of depositing particle

  const G4StepPoint *pPreStepPoint(pStep->GetPreStepPoint());
  const G4StepPoint *pPostStepPoint(pStep->GetPostStepPoint());

  const double kineticEnergy(pPreStepPoint->GetKineticEnergy());

  const G4ThreeVector preStepPoint(pPreStepPoint->GetPosition());
  const G4ThreeVector postStepPoint(pPostStepPoint->GetPosition());

  // Get name of physical volume
  const G4String vol_name(pPreStepPoint->GetTouchableHandle()->GetVolume()->GetName());

  const double preStepX(preStepPoint.x());
  const double preStepY(preStepPoint.y());
  const double preStepZ(preStepPoint.z());

  const double postStepX(postStepPoint.x());
  const double postStepY(postStepPoint.y());
  const double postStepZ(postStepPoint.z());

  int n_plate(0);
  int n_fiber(0);
  int n_station(0);

  m_hitID++;

  // particle encoding
  const G4Track *pTrack(pStep->GetTrack());
  const int particleEncoding(pTrack->GetDefinition()->GetPDGEncoding());

  const double globalTime(pTrack->GetGlobalTime());
  const int trackID(pTrack->GetTrackID()); //DC 2-29-04 use it as barcode

  std::string vol_test_str = vol_name.substr(0,7);


  if(vol_name.find("GVS") != std::string::npos)
    {

      if(vol_name.find("B7L1") != std::string::npos) n_station=0;
      else if(vol_name.find("A7L1") != std::string::npos) n_station=1;
      else if(vol_name.find("A7R1") != std::string::npos) n_station=2;
      else if(vol_name.find("B7R1") != std::string::npos) n_station=3;
      else n_station=-1;
      if(m_HitCollection)
        {
          m_HitCollection->Emplace(m_hitID, trackID, particleEncoding, (float) kineticEnergy,
                                   (float) energyDeposit,(float) preStepX, (float) preStepY, (float) preStepZ,
                                   (float) postStepX, (float) postStepY, (float) postStepZ,(float) globalTime,
                                   -1, 100, -1, (int) n_station);
        }
      else
        {
          G4ExceptionDescription description;
          description << "ProcessHits: Can't access HitCollection with key " << m_hitCollectionName;
          G4Exception("ALFA_SensitiveDetector", "InvalidHitColl1", FatalException, description);
          return false; //The G4Exception call above should abort the job, but Coverity does not seem to pick this up.
        }
    }

  if (vol_test_str.compare("ALFA_Fi") == 0)
    {
      if (std::abs(energyDeposit)<std::numeric_limits<double>::epsilon()) { return true; }
      m_pos2 = 10;
      std::string substring (vol_name);
      std::string num_string (vol_name);


      std::string test_str ("A");
      test_str = substring.substr(m_pos2,1);
      int sign_fiber(0);
      if (test_str.compare("U") == 0)
        {
          sign_fiber = 1;
        }

      if (test_str.compare("V") == 0)
        {
          sign_fiber = -1;
        }


      for ( int k = 0; k < 3; k++ )
        {
          substring = substring.substr(m_pos2+1);
          m_pos1 = int(substring.find('['));
          m_pos2 = int(substring.find(']'));
          num_string = substring.substr(m_pos1+1,m_pos2-1);
          std::istringstream is(num_string);
          is >> m_num[k];
        }

      n_station = m_num[0];
      n_plate   = m_num[1];
      n_fiber   = m_num[2];


      if(m_HitCollection)
        {
          m_HitCollection->Emplace(m_hitID,
                                   trackID,
                                   particleEncoding,
                                   (float) kineticEnergy,
                                   (float) energyDeposit,
                                   (float) preStepX, (float) preStepY, (float) preStepZ,
                                   (float) postStepX, (float) postStepY, (float) postStepZ,
                                   (float) globalTime,
                                   (int) sign_fiber, (int) n_plate, (int) n_fiber, (int) n_station
                                   );
        }
      else
        {
          G4ExceptionDescription description;
          description << "ProcessHits: Can't access HitCollection with key " << m_hitCollectionName;
          G4Exception("ALFA_SensitiveDetector", "InvalidHitColl2", FatalException, description);
          return false; //The G4Exception call above should abort the job, but Coverity does not seem to pick this up.
        }
    }

  if (vol_test_str.compare("ODFiber") == 0)
    {
      if (std::abs(energyDeposit)<std::numeric_limits<double>::epsilon()) { return true; }
      m_pos2 = 7;
      std::string substring (vol_name);
      std::string num_string (std::move(vol_name));

      std::string test_str = substring.substr(m_pos2,1);
      int sign_fiber(0);
      if (test_str.compare("U") == 0)
        {
          sign_fiber = 1;
        }

      if (test_str.compare("V") == 0)
        {
          sign_fiber = -1;
        }

      std::string test_str_side ("A");
      test_str_side = substring.substr(m_pos2+1,1);

      int OD_side(0);
      if (test_str_side.compare("0") == 0)
        {
          OD_side = 0;
        }

      if (test_str_side.compare("1") == 0)
        {
          OD_side = 1;
        }

      for ( int k = 0; k < 3; k++ )
        {
          substring = substring.substr(m_pos2+1);
          m_pos1 = int(substring.find('['));
          m_pos2 = int(substring.find(']'));
          num_string = substring.substr(m_pos1+1,m_pos2-1);
          std::istringstream is(num_string);
          is >> m_num[k];
        }

      n_station = m_num[0];
      n_plate   = m_num[1];
      n_fiber   = m_num[2];

      if(m_ODHitCollection)
        {
          m_ODHitCollection->Emplace(m_hitID,
                                     trackID,
                                     particleEncoding,
                                     (float) kineticEnergy,
                                     (float) energyDeposit,
                                     (float) preStepX, (float) preStepY, (float) preStepZ,
                                     (float) postStepX, (float) postStepY, (float) postStepZ,
                                     (float) globalTime,
                                     (int) sign_fiber, (int) OD_side, (int) n_plate, (int) n_fiber, (int) n_station
                                     );
        }
      else
        {
          G4ExceptionDescription description;
          description << "ProcessHits: Can't access HitCollection with key " << m_ODHitCollectionName;
          G4Exception("ALFA_SensitiveDetector", "InvalidHitColl3", FatalException, description);
          return false; //The G4Exception call above should abort the job, but Coverity does not seem to pick this up.
        }

    }


  return true;
}

ALFA_HitCollection* ALFA_SensitiveDetector::getHitCollection() const
{
  auto* eventInfo = AtlasG4EventUserInfo::GetEventUserInfo();
  if (!eventInfo) {
    return nullptr;
  }
  auto hitCollections = eventInfo->GetHitCollectionMap();
  return hitCollections ? hitCollections->Find<ALFA_HitCollection>(m_hitCollectionName) : nullptr;
}

ALFA_ODHitCollection* ALFA_SensitiveDetector::getODHitCollection() const
{
  auto* eventInfo = AtlasG4EventUserInfo::GetEventUserInfo();
  if (!eventInfo) {
    return nullptr;
  }
  auto hitCollections = eventInfo->GetHitCollectionMap();
  return hitCollections ? hitCollections->Find<ALFA_ODHitCollection>(m_ODHitCollectionName) : nullptr;
}
