/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_FASTCALOSIMPARAMETRIZATION_CALOCELLCONTAINERSD_H
#define ISF_FASTCALOSIMPARAMETRIZATION_CALOCELLCONTAINERSD_H

/* Base class header */
#include "G4VSensitiveDetector.hh"
/* CaloCellContainer include */
#include "CaloEvent/CaloCellContainer.h"
#include <string>

class G4TouchableHistory;
class G4HCofThisEvent;
class TFCSSimulationState;

class CaloCellContainerSD : public G4VSensitiveDetector
{
public:
  CaloCellContainerSD(const std::string& name, const std::string& CaloCellContainerName);
  ~CaloCellContainerSD() {}

  // Initialize from G4.
  void Initialize(G4HCofThisEvent*) override final;

  // Needs to be implemented, but is not used for this SD
  G4bool ProcessHits(G4Step*, G4TouchableHistory*) override final;

  // Method to record the cells from a TFCSSimulationState
  void recordCells(TFCSSimulationState&);

protected:
  CaloCellContainer* getCaloCellContainer() const;

  std::string m_caloCellContainerName;
  // Non-owning cache set by Initialize; HitCollectionMap owns the container.
  CaloCellContainer* m_caloCellContainer{};
};

#endif // ISF_FASTCALOSIMPARAMETRIZATION_CALOCELLCONTAINERSD_H
