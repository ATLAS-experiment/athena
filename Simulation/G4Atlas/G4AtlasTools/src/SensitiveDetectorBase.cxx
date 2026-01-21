/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/



// Base class
#include "G4AtlasTools/SensitiveDetectorBase.h"
// Geant4 includes used in functions
#include "G4LogicalVolumeStore.hh"
#include "G4MultiSensitiveDetector.hh"
#include "G4SDManager.hh"
// STL includes
#include <sstream>


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

  // Make sure SD isn't already registered
  if(getSD())
    {
      ATH_MSG_ERROR("Trying to create a SD which already exists!");
      return StatusCode::FAILURE;
    }

  // Make the SD stored by this tool
  auto sd = std::unique_ptr<G4VSensitiveDetector>(makeSD());
  if(!sd)
    {
      ATH_MSG_ERROR("Failed to create SD!");
      return StatusCode::FAILURE;
    }
  setSD(sd.get());

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
    auto logicalVolumeStore = G4LogicalVolumeStore::GetInstance();
    for(const auto& volumeName : volumes) {
      // Keep track of how many volumes we find with this name string.
      // We allow for multiple matches.
      int numFound = 0;

      // Find volumes with this name
      for(auto* logVol : *logicalVolumeStore) {

        ATH_MSG_VERBOSE("Check whether "<<logVol->GetName()<<" belongs to the set of sensitive detectors "<<volumeName);
        if( matchStrings( volumeName.data(), logVol->GetName() ) ){
          ++numFound;
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

G4VSensitiveDetector* SensitiveDetectorBase::getSD()
{
#ifdef G4MULTITHREADED
  // Get current thread-ID
  const auto tid = std::this_thread::get_id();
  // Retrieve it from the SD map
  auto sdPair = m_sdThreadMap.find(tid);
  if(sdPair == m_sdThreadMap.end()) return nullptr;
  return sdPair->second;
#else
  return m_SD;
#endif
}

void SensitiveDetectorBase::setSD(G4VSensitiveDetector* sd)
{
#ifdef G4MULTITHREADED
  const auto tid = std::this_thread::get_id();
  ATH_MSG_DEBUG("Creating and registering SD " << sd << " in thread " << tid);
  m_sdThreadMap.insert( std::make_pair(tid, sd) );
#else
  m_SD = sd;
#endif
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

