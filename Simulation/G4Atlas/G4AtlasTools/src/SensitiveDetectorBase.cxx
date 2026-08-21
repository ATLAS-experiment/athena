/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/



// Base class
#include "G4AtlasTools/SensitiveDetectorBase.h"
// Geant4 includes used in functions
#include "G4Box.hh"
#include "G4Event.hh"
#include "G4EventManager.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4MultiSensitiveDetector.hh"
#include "G4Navigator.hh"
#include "G4RotationMatrix.hh"
#include "G4RunManager.hh"
#include "G4SDManager.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4ThreeVector.hh"
#include "G4Track.hh"
#include "G4TransportationManager.hh"
#include "G4UserSteppingAction.hh"
#include "G4VPhysicalVolume.hh"
#include "G4ios.hh"
// STL includes
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <mutex>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

namespace {

bool isBCMPrimePadLV(const G4String& name)
{
  return name == "BCMPrime::BCMpBigDiamondPad" ||
         name == "BCMPrime::BCMpSmallDiamondPad";
}

bool inBCMPrimeSteppingRegion(const G4ThreeVector& pos)
{
  return std::abs(pos.z()) > 1700. && pos.perp() < 150.;
}

class BCMPrimeSteppingChainer : public G4UserSteppingAction
{
public:
  explicit BCMPrimeSteppingChainer(const G4UserSteppingAction* existingAction)
    : m_existingAction(existingAction)
    , m_diagnostic()
  {}

  void UserSteppingAction(const G4Step* step) override
  {
    m_diagnostic.UserSteppingAction(step);
    if (m_existingAction) {
      // G4 returns a const pointer even though UserSteppingAction is non-const.
      const_cast<G4UserSteppingAction*>(m_existingAction)->UserSteppingAction(step);
    }
  }

private:
  const G4UserSteppingAction* m_existingAction{nullptr};
};

void printBCMPrimePlacement(const G4VPhysicalVolume* physVol,
                            const G4RotationMatrix& worldRot,
                            const G4ThreeVector& worldTrans,
                            const std::string& path)
{
  const auto* logicalVol = physVol->GetLogicalVolume();
  const auto* box = dynamic_cast<const G4Box*>(logicalVol->GetSolid());

  G4cout << "BCMPrimeSensorSD diagnostic: G4 placement '" << path
         << "' LV='" << logicalVol->GetName()
         << "' PV='" << physVol->GetName()
         << "' copyNo=" << physVol->GetCopyNo()
         << " center(mm)=(" << worldTrans.x() << ", "
         << worldTrans.y() << ", " << worldTrans.z() << ")"
         << " r(mm)=" << worldTrans.perp()
         << " phi=" << worldTrans.phi()
         << " eta=" << (worldTrans.perp() > 0. ? std::asinh(worldTrans.z() / worldTrans.perp()) : 0.)
         << G4endl;

  if (!box) {
    G4cout << "BCMPrimeSensorSD diagnostic:   solid is not G4Box, skipping AABB dump"
           << G4endl;
    return;
  }

  const double hx = box->GetXHalfLength();
  const double hy = box->GetYHalfLength();
  const double hz = box->GetZHalfLength();
  const std::array<G4ThreeVector, 8> corners = {{
    {-hx, -hy, -hz}, {-hx, -hy, hz}, {-hx, hy, -hz}, {-hx, hy, hz},
    { hx, -hy, -hz}, { hx, -hy, hz}, { hx, hy, -hz}, { hx, hy, hz}
  }};

  G4ThreeVector minCorner(std::numeric_limits<double>::max(),
                          std::numeric_limits<double>::max(),
                          std::numeric_limits<double>::max());
  G4ThreeVector maxCorner(-std::numeric_limits<double>::max(),
                          -std::numeric_limits<double>::max(),
                          -std::numeric_limits<double>::max());

  for (const G4ThreeVector& corner : corners) {
    const G4ThreeVector worldCorner = worldRot * corner + worldTrans;
    minCorner.setX(std::min(minCorner.x(), worldCorner.x()));
    minCorner.setY(std::min(minCorner.y(), worldCorner.y()));
    minCorner.setZ(std::min(minCorner.z(), worldCorner.z()));
    maxCorner.setX(std::max(maxCorner.x(), worldCorner.x()));
    maxCorner.setY(std::max(maxCorner.y(), worldCorner.y()));
    maxCorner.setZ(std::max(maxCorner.z(), worldCorner.z()));
  }

  G4cout << "BCMPrimeSensorSD diagnostic:   global AABB min(mm)=("
         << minCorner.x() << ", " << minCorner.y() << ", " << minCorner.z()
         << ") max(mm)=(" << maxCorner.x() << ", " << maxCorner.y() << ", "
         << maxCorner.z() << ")" << G4endl;
}

void dumpBCMPrimePadPlacements(const G4VPhysicalVolume* physVol,
                               const G4RotationMatrix& parentRot,
                               const G4ThreeVector& parentTrans,
                               const std::string& path,
                               int& nFound)
{
  if (!physVol) {
    return;
  }

  const G4RotationMatrix localRot = physVol->GetObjectRotationValue();
  const G4ThreeVector localTrans = physVol->GetObjectTranslation();
  const G4RotationMatrix worldRot = parentRot * localRot;
  const G4ThreeVector worldTrans = parentRot * localTrans + parentTrans;

  const auto* logicalVol = physVol->GetLogicalVolume();
  const std::string currentPath = path + "/" + physVol->GetName();
  if (logicalVol && isBCMPrimePadLV(logicalVol->GetName())) {
    ++nFound;
    printBCMPrimePlacement(physVol, worldRot, worldTrans, currentPath);
  }

  if (!logicalVol) {
    return;
  }

  const int nDaughters = logicalVol->GetNoDaughters();
  for (int i = 0; i < nDaughters; ++i) {
    dumpBCMPrimePadPlacements(logicalVol->GetDaughter(i),
                              worldRot,
                              worldTrans,
                              currentPath,
                              nFound);
  }
}

void dumpBCMPrimePadPlacements()
{
  static std::once_flag dumped;
  std::call_once(dumped, [] {

    const auto* world =
      G4TransportationManager::GetTransportationManager()->GetNavigatorForTracking()->GetWorldVolume();
    if (!world) {
      G4cout << "BCMPrimeSensorSD diagnostic: no G4 world volume available for placement dump"
             << G4endl;
      return;
    }

    int nFound = 0;
    dumpBCMPrimePadPlacements(world,
                              G4RotationMatrix(),
                              G4ThreeVector(),
                              "",
                              nFound);
    G4cout << "BCMPrimeSensorSD diagnostic: dumped " << nFound
           << " BCMPrime pad physical placements from G4 tree" << G4endl;
  });
}

} // namespace


SensitiveDetectorBase::SensitiveDetectorBase(const std::string& type,
                                             const std::string& name,
                                             const IInterface* parent)
  : base_class(type,name,parent)
{
}

// Athena method used to set up the SDs for the current worker thread.
StatusCode SensitiveDetectorBase::initializeSD()
{
  ATH_MSG_VERBOSE( name() << "::initializeSD()" );

  // Sanity check for volume configuration problems.
  // It would be better to have a more robust solution for this.
  if(m_volumeNames.empty() != m_noVolumes) {
    ATH_MSG_ERROR("Initializing SD from " << name() << ", NoVolumes = "
                  << (m_noVolumes? "true" : "false") << ", but LogicalVolumeNames = "
                  << m_volumeNames.value());
    return StatusCode::FAILURE;
  }

  // Make the SD stored by this tool
  auto sd = std::unique_ptr<G4VSensitiveDetector>(makeSD());
  if(!sd)
    {
      ATH_MSG_ERROR("Failed to create SD!");
      return StatusCode::FAILURE;
    }
  // Assign the SD to our list of volumes
  ATH_CHECK( assignSD( std::move(sd), m_volumeNames.value() ) );

  ATH_MSG_DEBUG( "Initialized and added SD " << name() );
  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------
// Assign an SD to a list of volumes
//-----------------------------------------------------------------------------
StatusCode SensitiveDetectorBase::
assignSD(std::unique_ptr<G4VSensitiveDetector> sd, const std::vector<std::string>& volumes) const
{
  // Propagate verbosity setting to the SD
  if(msgLvl(MSG::VERBOSE)) sd->SetVerboseLevel(10);
  else if(msgLvl(MSG::DEBUG)) sd->SetVerboseLevel(5);

  // Add the sensitive detector to the SD manager in G4 for SDs,
  // even if it has no volumes associated to it.
  auto sdMgr = G4SDManager::GetSDMpointer();
  auto sdPtr = sd.get();
  // SDManager is now the SD owner
  //for later use
  auto sdName = sd->GetName();
  sdMgr->AddNewDetector(sd.release());

  if(!volumes.empty()) {
    bool gotOne = false;
    const bool diagnoseBCMPrime =
      name().find("BCMPrime") != std::string::npos ||
      sdName.find("BCMPrime") != std::string::npos ||
      std::find(m_outputCollectionNames.value().begin(),
                m_outputCollectionNames.value().end(),
                "BCMPrimeHits") != m_outputCollectionNames.value().end() ||
      std::find(m_outputCollectionNames.value().begin(),
                m_outputCollectionNames.value().end(),
                "BCMPrimeHits_G4") != m_outputCollectionNames.value().end();
    auto logicalVolumeStore = G4LogicalVolumeStore::GetInstance();
    
    for(const auto& volumeName : volumes) {
      // Keep track of how many volumes we find with this name string.
      // We allow for multiple matches.
      int numFound = 0;
      std::vector<std::string> matchedVolumes;

      // Find volumes with this name
      for(auto* logVol : *logicalVolumeStore) {

        ATH_MSG_VERBOSE("Check whether "<<logVol->GetName()<<" belongs to the set of sensitive detectors "<<volumeName);
        if( matchStrings( volumeName.data(), logVol->GetName() ) ){
          ++numFound;
          if (diagnoseBCMPrime) {
            matchedVolumes.push_back(logVol->GetName());
          }
          SetSensitiveDetector(logVol, sdPtr);
        }
        
      }
      // Warn if no volumes were found
      if(numFound == 0) {
        ATH_MSG_WARNING("Volume " << volumeName <<
                        " not found in G4LogicalVolumeStore.");
      }
      else {
        ATH_MSG_VERBOSE("Found " << numFound << " copies of LV " << volumeName <<
                        "; SD " << sdName << " assigned.");
        gotOne = true;
      }
      if (diagnoseBCMPrime) {
        ATH_MSG_INFO("BCMPrimeSensorSD diagnostic: pattern '" << volumeName
                     << "' matched " << numFound << " G4 logical volumes");
        for (const std::string& matchedVolume : matchedVolumes) {
          ATH_MSG_INFO("BCMPrimeSensorSD diagnostic: matched LV '" << matchedVolume << "'");
        }
      }

    }

    // Abort if we have failed to assign any volume
    if(!gotOne) {
      ATH_MSG_ERROR( "Failed to assign *any* volume to SD " << name() <<
                     " and expected at least one. Size of the volume store "<<G4LogicalVolumeStore::GetInstance()->size() );
      return StatusCode::FAILURE;
    }
  }

  return StatusCode::SUCCESS;
}

//This function was adapted from the example found at
//https://www.geeksforgeeks.org/wildcard-character-matching/
bool SensitiveDetectorBase::matchStrings(const char *first, const char *second)
{
  // If we reach at the end of both strings, we are done
  if (*first == '\0' && *second == '\0')
    return true;

  // If there are consecutive '*' present in the first string
  // advance to the next character
  if(*first == '*' && *(first + 1) == '*')
    return matchStrings(first + 1, second);

  // Make sure that the characters after '*' are present in second string.
  if (*first == '*' && *(first + 1) != '\0' && *second == '\0')
    return false;

  // If the current characters of both strings match
  if (*first == *second)
    return matchStrings(first + 1, second + 1);

  // If there is *, then there are two possibilities
  // a) We consider current character of second string
  // b) We ignore current character of second string.
  if (*first == '*')
    return matchStrings(first + 1, second) || matchStrings(first, second + 1);
  return false;
}

void SensitiveDetectorBase::
SetSensitiveDetector(G4LogicalVolume* logVol, G4VSensitiveDetector* aSD) const
{
  // New Logic: allow for "multiple" SDs being attached to a single LV.
  // To do that we use a special proxy SD called G4MultiSensitiveDetector

  // Get existing SD if already set and check if it is of the special type
  G4VSensitiveDetector* originalSD = logVol->GetSensitiveDetector();
  if ( originalSD == nullptr )
    {
      logVol->SetSensitiveDetector(aSD);
    }
  else
    {
      G4MultiSensitiveDetector* msd = dynamic_cast<G4MultiSensitiveDetector*>(originalSD);
      if ( msd != nullptr )
        {
          msd->AddSD(aSD);
        }
      else
        {
          // Construct a unique name using the volume address
          std::stringstream ss;
          ss << static_cast<const void*>(logVol);
          const G4String msdname = "/MultiSD_" + logVol->GetName() + ss.str();
          msd = new G4MultiSensitiveDetector(std::move(msdname));
          // We need to register the proxy to have correct handling of IDs
          G4SDManager::GetSDMpointer()->AddNewDetector(msd);
          msd->AddSD(originalSD);
          msd->AddSD(aSD);
          logVol->SetSensitiveDetector(msd);
        }
    }
}

