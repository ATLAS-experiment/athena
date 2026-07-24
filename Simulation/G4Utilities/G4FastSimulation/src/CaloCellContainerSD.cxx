/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloCellContainerSD.h"

#include "CaloCellContainerBuilder.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "HitManagement/HitCollectionMap.h"
#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "MCTruth/AtlasG4EventUserInfo.h"
#include "StoreGate/StoreGateSvc.h"
#include "GaudiKernel/ServiceHandle.h"

#include "FastCaloSim/Core/TFCSSimulationState.h"

#include "G4EventManager.hh"

CaloCellContainerSD::CaloCellContainerSD(
    const std::string& name, const std::string& CaloCellContainerName)
  : G4VSensitiveDetector(name),
    m_caloCellContainerName(CaloCellContainerName)
{
  ServiceHandle<StoreGateSvc> detStore("DetectorStore", name);
  if(detStore.retrieve().isFailure() || detStore->retrieve(m_cellIdHelper, "CaloCell_ID").isFailure()) {
    G4Exception("CaloCellContainerSD", "FailedCaloCellIDRetrieval", FatalException, "CaloCellContainerSD: Failed to retrieve the CaloCell_ID helper.");
    abort();
  }
}

void CaloCellContainerSD::Initialize(G4HCofThisEvent*)
{
  m_caloCellContainer = getCaloCellContainer();
}

G4bool CaloCellContainerSD::ProcessHits(G4Step*, G4TouchableHistory* ){
  G4Exception("CaloCellContainerSD", "UnexpectedProcessHitsCall",
              FatalException,
              "ProcessHits must not be called for CaloCellContainerSD.");
  abort();
  return true;
}


void CaloCellContainerSD::recordCells(TFCSSimulationState& simState)
{
  if (!m_caloCellContainer) {
    m_caloCellContainer = getCaloCellContainer();
  }
  if (!m_caloCellContainer) {
    G4Exception("CaloCellContainerSD", "MissingCaloCellContainer",
                FatalException,
                "Failed to retrieve the event-owned CaloCellContainer.");
    abort();
  }

  for(const auto& icell : simState.cells()) {
    // FastCaloSim uses compact identifiers, not calorimeter cell hashes.
    const IdentifierHash cellHash =
      m_cellIdHelper->calo_cell_hash(Identifier(icell.first));
    CaloCell* caloCell =
      static_cast<CaloCell*>(m_caloCellContainer->findCell(cellHash));
    if (!caloCell) {
      G4cout << "CaloCellContainerSD: skipping energy deposit in unknown cell id "
             << icell.first << G4endl;
      continue;
    }
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

  std::shared_ptr<HitCollectionMap> hitCollections =
    eventInfo->GetHitCollectionMap();
  auto* builder = hitCollections
    ? hitCollections->Find<CaloCellContainerBuilder>(m_caloCellContainerName)
    : nullptr;
  return builder ? builder->container.get() : nullptr;
}
