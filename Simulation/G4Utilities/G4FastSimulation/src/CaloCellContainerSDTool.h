/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4FASTSIMULATION_CALOCELLCONTAINERSDTOOL_H
#define G4FASTSIMULATION_CALOCELLCONTAINERSDTOOL_H

// Base class header
#include "G4AtlasTools/SensitiveDetectorBase.h"

/* Fast hit converter include to take into account sampling fractions */
#include "CaloInterface/ICaloCellMakerTool.h"

class G4VSensitiveDetector;

class CaloCellContainerSDTool : public SensitiveDetectorBase
{
public:
  // Constructor
  CaloCellContainerSDTool(const std::string& type, const std::string& name, const IInterface* parent);
  ~CaloCellContainerSDTool() override = default;

  StatusCode initialize() override final;
  /** Create and initialize the event's calorimeter cell container. */
  StatusCode SetupEvent(HitCollectionMap&) override final;
  /** Convert and record the event's calorimeter cell container. */
  StatusCode Gather(HitCollectionMap&) override final;

protected:
  G4VSensitiveDetector* makeSD() const override final;
  PublicToolHandle<ICaloCellMakerTool> m_EmptyCellBuilderTool;
  PublicToolHandle<ICaloCellMakerTool> m_FastHitConvertTool;

};

#endif  // G4FASTSIMULATION_CALOCELLCONTAINERSDTOOL_H
