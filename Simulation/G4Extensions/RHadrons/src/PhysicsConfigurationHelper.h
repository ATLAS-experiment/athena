/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef RHADRONS_PHYSICSCONFIGURATIONHELPER_H
#define RHADRONS_PHYSICSCONFIGURATIONHELPER_H 1

#include "CxxUtils/checker_macros.h"
#include"globals.hh"
#include "CLHEP/Units/PhysicalConstants.h"
#include <vector>

class PhysicsConfigurationHelper {

public:
  static const PhysicsConfigurationHelper* Instance();

  inline bool Resonant() const { return m_resonant; }
  inline bool ReggeModel() const { return m_reggemodel; }
  inline double ResonanceEnergy() const { return m_ek_0; }
  inline double XsecMultiplier() const { return m_xsecmultiplier; }
  inline double Gamma() const { return m_gamma; }
  inline double Amplitude() const { return m_amplitude; }
  inline double SuppressionFactor() const { return m_suppressionfactor; }
  inline double Lifetime() const { return (m_doDecays == 1) ? m_hadronlifetime * CLHEP::ns : m_hadronlifetime * CLHEP::s; }
  inline double Mixing() const { return m_mixing; }
  inline int DoDecays() const { return m_doDecays; }

protected:
  PhysicsConfigurationHelper();
  PhysicsConfigurationHelper(const PhysicsConfigurationHelper&);
  PhysicsConfigurationHelper& operator= (const PhysicsConfigurationHelper&);

private:

  void ReadAndParse(const G4String& str,
                    std::vector<G4String>& tokens,
                    const G4String& delimiters = " ") const;
  void ReadInPhysicsParameters(std::map<G4String,G4double>&  parameters) const;

  //The parameters themselves
  bool m_resonant{false};
  bool m_reggemodel{false};
  double m_ek_0{0.};
  double m_gamma{0.};
  double m_amplitude{0.};
  double m_xsecmultiplier{1.};
  double m_suppressionfactor{0.};
  double m_hadronlifetime{0.};
  double m_mixing{0.};
  int m_doDecays{0};

};
#endif // RHADRONS_PHYSICSCONFIGURATIONHELPER_H
