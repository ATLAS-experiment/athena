/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// LArHECWheelCalculator.hh

// Revision history:

// Jan-2008: (M.Fincke)  To be used for new Module-Geometry

#ifndef LARG4HEC_LARHECWHEELCALCULATOR_H
#define LARG4HEC_LARHECWHEELCALCULATOR_H

#include "LArG4Code/LArCalculatorSvcImp.h"
#include "LArHV/HECHVManager.h"

// Forward declarations.

class G4Step;
class HECGeometry;
class LArG4BirksLaw;
class HECDetectorManager;

namespace LArG4 {
  namespace HEC {
     class IHECGeometry;
  }
}


class LArHECWheelCalculator : virtual public LArCalculatorSvcImp {
public:

  LArHECWheelCalculator(const std::string& name, ISvcLocator * pSvcLocator);
  LArHECWheelCalculator (const LArHECWheelCalculator&) = delete;
  LArHECWheelCalculator& operator= (const LArHECWheelCalculator&) = delete;
  virtual StatusCode initialize() override final;
  virtual StatusCode finalize() override final;
  ~LArHECWheelCalculator();
  /////////////////////////////////////////////

  virtual void initializeForSDCreation() override final;
  virtual G4float OOTcut() const override final { return m_OOTcut; }

  virtual G4bool Process(const G4Step* a_step, std::vector<LArHitData>& hdata) const override final;

  // Check if the current hitTime is in-time
  virtual G4bool isInTime(G4double hitTime) const override final
  {
    return !(hitTime > m_OOTcut); //FIXME should we be checking the absolute value of hitTime here?
  }


private:
  ServiceHandle<LArG4::HEC::IHECGeometry> m_Geometry{this, "GeometryCalculator", "HECGeometry"};
  Gaudi::Property<bool>     m_doHV{this, "HECHVEnable", false};
  
  const HECDetectorManager *m_DetectorManager{nullptr};
  LArG4BirksLaw *m_birksLaw{nullptr};

  HECHVManager::HECHVData m_hvdata;
};

#endif
