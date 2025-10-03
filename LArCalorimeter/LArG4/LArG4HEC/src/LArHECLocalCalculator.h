/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// LArG4HECLocalCalculator.hh

// Revision history:

// 17-Feb-2006 : Pavol Strizenec

#ifndef LARG4HEC_LARG4HECLOCALCALCULATOR_H
#define LARG4HEC_LARG4HECLOCALCALCULATOR_H

#include "LArG4Code/LArCalculatorSvcImp.h"
#include "CLHEP/Units/SystemOfUnits.h"

// Forward declarations.
class G4Step;
class LArG4BirksLaw;

namespace LArG4 {
  namespace HEC {
    class ILocalGeometry;
  }
}

class LArHECLocalCalculator : virtual public LArCalculatorSvcImp {

public:
  LArHECLocalCalculator(const std::string& name, ISvcLocator * pSvcLocator);  
  LArHECLocalCalculator (const LArHECLocalCalculator&) = delete;
  LArHECLocalCalculator operator= (const LArHECLocalCalculator&) = delete;
  virtual ~LArHECLocalCalculator() = default;
  virtual StatusCode initialize() override final;
  virtual StatusCode finalize() override final;

  virtual G4float OOTcut() const override final { return m_OOTcut; }

  virtual G4bool Process(const G4Step* a_step, std::vector<LArHitData>& hdata) const override final { return this->Process(a_step,0, 4.*CLHEP::mm, hdata);}
  virtual G4bool Process(const G4Step* a_step, int depthadd, double deadzone, std::vector<LArHitData>& hdata) const final; //FIXME not part of interface...

  // Check if the current hitTime is in-time
  virtual G4bool isInTime(G4double hitTime) const override final
  {
    return !(hitTime > m_OOTcut); //FIXME should we be checking the absolute value of hitTime here?
  }

private:
  ServiceHandle<LArG4::HEC::ILocalGeometry> m_Geometry {this, "GeometryCalculator", "LocalHECGeometry"};
  Gaudi::Property<G4bool> m_isX {this, "IsX", false};
  LArG4BirksLaw *m_birksLaw{nullptr};
};

#endif
