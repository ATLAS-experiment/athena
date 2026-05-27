/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Class header
#include "ISF_FastCaloSimParametrization/CaloCellContainerSD.h"

// Athena headers
#include "HitManagement/HitCollectionMap.h"
#include "ISF_FastCaloSimParametrization/CaloCellContainerBuilder.h"
#include "MCTruth/AtlasG4EventUserInfo.h"

// FastCaloSim simulation include
#include "ISF_FastCaloSimEvent/TFCSSimulationState.h"

#include "G4EventManager.hh"

#include <memory>

CaloCellContainerSD::CaloCellContainerSD(const std::string& name,
                                         const std::string& CaloCellContainerName)
  : G4VSensitiveDetector(name)
  , m_caloCellContainerName(CaloCellContainerName)
{
}

void CaloCellContainerSD::Initialize(G4HCofThisEvent*)
{
  m_caloCellContainer = getCaloCellContainer();
}

G4bool CaloCellContainerSD::ProcessHits(G4Step*, G4TouchableHistory*)
{
  // This method needs to be implemented when deriving from G4VSensitiveDetector has no use in this case
  G4Exception("CaloCellContainerSD", "UndefinedProcessHitsCall", FatalException, "CaloCellContainerSD: Call to undefined ProcessHits.");
  abort();
  return true;
}


void CaloCellContainerSD::recordCells(TFCSSimulationState& simState)
{
  if (!m_caloCellContainer) {
    // ISF can initialize SDs before the per-G4Event user info is installed.
    // Refresh from the current event when available to avoid stale caches.
    m_caloCellContainer = getCaloCellContainer();
  }
  if (!m_caloCellContainer) {
    G4Exception("CaloCellContainerSD", "MissingCaloCellContainer", FatalException, "CaloCellContainerSD: Failed to retrieve the event-owned CaloCellContainer.");
    abort();
  }

  // Add the energies from the simulation state to the CaloCellContainer
  for(const auto& icell : simState.cells()) {
    CaloCell* caloCell =
      static_cast<CaloCell*>(m_caloCellContainer->findCell(icell.first->calo_hash()));
    caloCell->addEnergy(icell.second);
  }
}

CaloCellContainer* CaloCellContainerSD::getCaloCellContainer() const
{
  auto* eventManager = G4EventManager::GetEventManager();
  if (!eventManager) {
    return nullptr;
  }

  auto* eventInfo =
    dynamic_cast<AtlasG4EventUserInfo*>(eventManager->GetUserInformation());
  if (!eventInfo) {
    return nullptr;
  }

  std::shared_ptr<HitCollectionMap> hitCollections = eventInfo->GetHitCollectionMap();
  auto* builder = hitCollections
    ? hitCollections->Find<CaloCellContainerBuilder>(m_caloCellContainerName)
    : nullptr;
  return builder ? builder->container.get() : nullptr;
}
