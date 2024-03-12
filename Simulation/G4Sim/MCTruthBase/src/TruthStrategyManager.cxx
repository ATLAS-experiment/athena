/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// class header
#include "MCTruthBase/TruthStrategyManager.h"

// Framework includes
#include "AthenaBaseComps/AthMsgStreamMacros.h"

// Geant4 Includes
#include "G4Event.hh"
#include "G4EventManager.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4Step.hh"
#include "G4TransportationManager.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VSolid.hh"

// Truth-related includes
#include "MCTruth/AtlasG4EventUserInfo.h"
#include "MCTruth/TrackHelper.h"

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

  auto* atlasG4EvtUserInfo = static_cast<AtlasG4EventUserInfo*> (G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetUserInformation());

  // This is pretty ugly and but necessary because the Geant4TruthIncident
  // requires an ISFParticle at this point.
  // TODO: cleanup Geant4TruthIncident to not require an ISFParticle instance any longer
  const Amg::Vector3D myPos(0,0,0);
  const Amg::Vector3D myMom(0,0,0);
  double myMass = 0.0;
  double myCharge = 0.0;
  int    myPdgCode = 0;
  int    mystatus = 3333;
  double myTime =0.;
  const ISF::DetRegionSvcIDPair origin(geoID, ISF::fUndefinedSimID);
  ISF::ISFParticle myISFParticle(myPos, myMom, myMass, myCharge, myPdgCode, mystatus, myTime, origin);

  iGeant4::Geant4TruthIncident truth(aStep, myISFParticle, geoID, atlasG4EvtUserInfo);

  m_truthSvc->registerTruthIncident(truth);
  return false;
}

