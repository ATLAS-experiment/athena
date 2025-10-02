/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARG4CODE_LARCALCULATORSVCIMP_H
#define LARG4CODE_LARCALCULATORSVCIMP_H

#include "ILArCalculatorSvc.h"
#include "AthenaBaseComps/AthService.h"
#include "CLHEP/Units/SystemOfUnits.h"

class LArCalculatorSvcImp: public extends<AthService, ILArCalculatorSvc>
{
public:
  LArCalculatorSvcImp(const std::string& name, ISvcLocator * pSvcLocator)
    : base_class(name, pSvcLocator) {};

  // Give this method an empty default since it's mostly not used
  virtual void initializeForSDCreation() override {};

protected:
  // Birks' law
  Gaudi::Property<bool> m_BirksLaw{this, "BirksLaw", true};

  // Birks' law, constant k
  // value updated for G4 10.6.p03 - 1.2 times the previous value of 0.0486 used in all campaigns before MC21.
  Gaudi::Property<double> m_Birksk{this, "Birksk", 0.05832};

  // OOTcut
  Gaudi::Property<double> m_OOTcut{this, "OOTcut", 300*CLHEP::ns};
};

#endif
