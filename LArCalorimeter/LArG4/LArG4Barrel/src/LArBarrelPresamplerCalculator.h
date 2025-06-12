/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// LArBarrelPresamplerCalculator.hh
// Prepared 05-Dec-2002 Bill Seligman

// A first pass at determing hit cell ID in the LAr barrel presampler.

#ifndef LARG4BARREL_LARBARRELPRESAMPLERCALCULATOR_H
#define LARG4BARREL_LARBARRELPRESAMPLERCALCULATOR_H

#include "LArG4Code/LArG4Identifier.h"
#include "LArG4Code/LArCalculatorSvcImp.h"
#include "ILArBarrelPresamplerGeometry.h"

#include "globals.hh"
#include <stdexcept>
// Forward declarations.
class G4Step;
class PsMap;
class LArG4BirksLaw;
class G4String;

class LArBarrelPresamplerCalculator : public LArCalculatorSvcImp {

public:

  LArBarrelPresamplerCalculator(const std::string& name, ISvcLocator *pSvcLocator);
  LArBarrelPresamplerCalculator(const LArBarrelPresamplerCalculator&) = delete;
  LArBarrelPresamplerCalculator& operator=(const  LArBarrelPresamplerCalculator&) = delete;

  virtual StatusCode initialize() override final;
  virtual StatusCode finalize() override final;

  /////////////////////////////////////////////

  virtual G4float OOTcut() const override final { return m_OOTcut; }

  virtual G4bool Process(const G4Step* a_step, std::vector<LArHitData>& hdata) const override final;

  // Check if the current hitTime is in-time
  virtual G4bool isInTime(G4double hitTime) const override final
  {
    return !(std::fabs(hitTime) > m_OOTcut);
  }

private:
  
  // Class for calculating the identifier.
  ServiceHandle<ILArBarrelPresamplerGeometry> m_geometry{this, "GeometryCalculator", "LArBarrelPresamplerGeometry"};

  Gaudi::Property<bool> m_IflCur{this, "IflCur", true};

  // detector name, for translated geometry
  Gaudi::Property<std::string> m_detectorName{this, "DetectorName", "LArMgr"};

  Gaudi::Property<bool> m_testbeam{this, "isTestbeam", false};

  const PsMap* m_psmap{nullptr};
  const LArG4BirksLaw *m_birksLaw{nullptr};
  G4String m_volname{"LArMgr::LAr::Barrel::Presampler"};
};

#endif // __LArBarrelPresamplerCalculator_H__
