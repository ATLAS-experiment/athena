/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// class header
#include "MCTruthBase/TruthStrategyManager.h"

// Framework includes
#include "AthenaBaseComps/AthMsgStreamMacros.h"

#include "TruthUtils/MagicNumbers.h"

// Geant4 Includes
#include "G4Event.hh"
#include "G4EventManager.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4Step.hh"
#include "G4TransportationManager.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VSolid.hh"

// ISF includes
#include "ISF_Interfaces/ITruthSvc.h"
#include "ISF_Interfaces/IGeoIDSvc.h"
#include "ISF_Event/ISFParticle.h"

// DetectorDescription
#include "AtlasDetDescr/AtlasRegionHelper.h"
#include "ISF_Geant4Event/Geant4TruthIncident.h"
#include "ISF_Geant4Event/ISFG4GeoHelper.h"

TruthStrategyManager::TruthStrategyManager()
  : m_truthSvc(nullptr)
  , m_geoIDSvc(nullptr)
{
}

const TruthStrategyManager& TruthStrategyManager::GetStrategyManager()
{
  static const TruthStrategyManager theMgr;
  return theMgr;
}

TruthStrategyManager& TruthStrategyManager::GetStrategyManager_nc ATLAS_NOT_THREAD_SAFE ()
{
  return const_cast<TruthStrategyManager&>(GetStrategyManager());
}

void TruthStrategyManager::SetISFTruthSvc(ISF::ITruthSvc *truthSvc)
{
  m_truthSvc = truthSvc;
}


void TruthStrategyManager::SetISFGeoIDSvc(ISF::IGeoIDSvc *geoIDSvc)
{
  m_geoIDSvc = geoIDSvc;
}

bool TruthStrategyManager::CreateTruthIncident(const G4Step* aStep, int subDetVolLevel) const
{
  AtlasDetDescr::AtlasRegion geoID = iGeant4::ISFG4GeoHelper::nextGeoId(aStep, subDetVolLevel, m_geoIDSvc);

  iGeant4::Geant4TruthIncident truth(aStep, geoID);

  m_truthSvc->registerTruthIncident(truth);
  return false;
}

