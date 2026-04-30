/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_FASTCALOSIMPARAMETRIZATION_CALOCELLCONTAINERSD_H
#define ISF_FASTCALOSIMPARAMETRIZATION_CALOCELLCONTAINERSD_H

/* Base class header */
#include "G4VSensitiveDetector.hh"
/* Athena WriteHandle include */
#include "StoreGate/WriteHandle.h"
/* Empty cell builder interface include */
#include "CaloInterface/ICaloCellMakerTool.h"
/* CaloCellContainer include */
#include "CaloEvent/CaloCellContainer.h"
#include <string>

class G4TouchableHistory;
class TFCSSimulationState;
class EventContext;

class CaloCellContainerSD : public G4VSensitiveDetector
{
public:
  CaloCellContainerSD(const std::string& name, const std::string& CaloCellContainerName, const PublicToolHandle<ICaloCellMakerTool> & FastHitConvertTool);
  ~CaloCellContainerSD() {}

  // Start of *ATHENA* event
  void StartOfAthenaEvent(const EventContext& ctx);

  // End of *ATHENA* event
  void EndOfAthenaEvent(const EventContext& ctx);

  // Needs to be implemented, but is not used for this SD
  G4bool ProcessHits(G4Step*, G4TouchableHistory*) override final;

  // Method to record the cells from a TFCSSimulationState
  void recordCells(TFCSSimulationState&);

protected:
  // The empty cell builder tool to initialize an empty calo cell container
  PublicToolHandle<ICaloCellMakerTool> m_EmptyCellBuilderTool;
  // The write handle to the CaloCellContainer
  SG::WriteHandle<CaloCellContainer> m_caloCellContainer;
  // Fast hit converter - converts CaloCells into LAr and Tile Hits - takes into account sampling fractions
  PublicToolHandle<ICaloCellMakerTool> m_FastHitConvertTool;


};

#endif // ISF_FASTCALOSIMPARAMETRIZATION_CALOCELLCONTAINERSD_H
