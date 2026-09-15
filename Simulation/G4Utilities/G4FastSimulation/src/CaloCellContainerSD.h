/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4FASTSIMULATION_CALOCELLCONTAINERSD_H
#define G4FASTSIMULATION_CALOCELLCONTAINERSD_H

#include "G4VSensitiveDetector.hh"
#include "CaloEvent/CaloCellContainer.h"

class G4TouchableHistory;
class G4HCofThisEvent;
class TFCSSimulationState;
class CaloCell_ID;

class CaloCellContainerSD : public G4VSensitiveDetector
{
public:
  CaloCellContainerSD(const std::string& name,
                      const std::string& CaloCellContainerName);
  ~CaloCellContainerSD() override = default;

  void Initialize(G4HCofThisEvent*) override final;

  // G4VSensitiveDetector requires this method, but this detector is filled
  // directly by FastCaloSimModel.
  G4bool ProcessHits(G4Step*, G4TouchableHistory*) override final;

  /// Add the FastCaloSim cell energies to the event container.
  void recordCells(TFCSSimulationState&);

protected:
  CaloCellContainer* getCaloCellContainer() const;

  std::string m_caloCellContainerName;
  CaloCellContainer* m_caloCellContainer{};
  const CaloCell_ID* m_cellIdHelper{};
};

#endif  // G4FASTSIMULATION_CALOCELLCONTAINERSD_H
