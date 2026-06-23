/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_FASTCALOSIMPARAMETRIZATION_CALOCELLCONTAINERTOOL_H
#define ISF_FASTCALOSIMPARAMETRIZATION_CALOCELLCONTAINERTOOL_H

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
  // Destructor
  ~CaloCellContainerSDTool() {}

  StatusCode initialize() override final;
  /** Beginning of an athena event.  This is where collection initialization should happen. **/
  StatusCode SetupEvent(HitCollectionMap&) override final;
  /** End of an athena event **/
  StatusCode Gather(HitCollectionMap&) override final;

protected:
  // Make me an SD!
  G4VSensitiveDetector* makeSD() const override final;
  PublicToolHandle<ICaloCellMakerTool> m_EmptyCellBuilderTool;
  PublicToolHandle<ICaloCellMakerTool> m_FastHitConvertTool;

};

#endif //ISF_FASTCALOSIMPARAMETRIZATION_CALOCELLCONTAINERTOOL_H
