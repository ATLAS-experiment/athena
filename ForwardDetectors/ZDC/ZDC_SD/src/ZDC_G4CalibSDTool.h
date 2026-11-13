/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDCG4SDTOOL_H
#define ZDCG4SDTOOL_H

// Base class
#include "G4AtlasTools/SensitiveDetectorBase.h"

// For escaped energy
#include "CaloG4Sim/EscapedEnergyRegistry.h"
#include "CaloG4Sim/CalibrationDefaultProcessing.h"
#include "ZDC_EscapedEnergyProcessing.h"
#include "CaloSimEvent/CaloCalibrationHitContainer.h"
#include "Gaudi/Property.h"

class G4VSensitiveDetector;

class ZDC_G4CalibSDTool : public SensitiveDetectorBase
{
 public:
  // Constructor
  ZDC_G4CalibSDTool(const std::string& type, const std::string& name, const IInterface *parent);
  // Destructor
  virtual ~ZDC_G4CalibSDTool() {}
  StatusCode SetupEvent(HitCollectionMap&) override final;
  StatusCode Gather(HitCollectionMap&) override final;

 protected:
  //Make an SD
  G4VSensitiveDetector* makeSD()const override final;

 private:
  Gaudi::Property<bool> m_deadSD{this, "deadSD", false};
  Gaudi::Property<bool> m_doPid{this, "doPid", false};
};

#endif
