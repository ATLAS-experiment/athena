/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LArG4H6WarmTCCalculator_H
#define LArG4H6WarmTCCalculator_H

#include "LArG4Code/LArCalculatorSvcImp.h"
#include "LArG4Code/LArG4Identifier.h"

// Forward declarations.
class G4Step;

class LArG4H6WarmTCCalculator : public LArCalculatorSvcImp
{
public:

  LArG4H6WarmTCCalculator(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~LArG4H6WarmTCCalculator() = default;
  /////////////////////////////////////////////
  // The interface for ILArCalculatorSvc.

  virtual G4float OOTcut() const override final { return m_OOTcut; }

  virtual G4bool Process(const G4Step*, std::vector<LArHitData>&) const override final;

  virtual G4bool isInTime(G4double hitTime) const override final
  {
    return !(hitTime > m_OOTcut);
  }

private:
  Gaudi::Property<bool> m_isX {this, "isX", false};
  Gaudi::Property<bool> m_isABS {this,"isABS", false};
};

#endif
