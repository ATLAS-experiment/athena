/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// LArG4H6COLDTCMod0Calculator
// Prepared 8-March-2004: Mohsen Khakzad
// Updated for GeoModelize simulations 27-May-2007: P. Strizenec

// Defines constants specific to a single FCAL module.

#ifndef LArG4H6COLDTCMod0Calculator_H
#define LArG4H6COLDTCMod0Calculator_H

#include "LArG4Code/LArG4Identifier.h"
#include "LArG4Code/LArCalculatorSvcImp.h"
#include "AthenaKernel/Units.h"

#include "globals.hh"

#include "LArG4H6COLDTCMod0ChannelMap.h"

namespace Units = Athena::Units;

class LArG4H6COLDTCMod0Calculator : public LArCalculatorSvcImp
{
public:

  LArG4H6COLDTCMod0Calculator(const std::string& name, ISvcLocator* pSvcLocator);
  virtual StatusCode initialize() override final;
  // destructor
  virtual ~LArG4H6COLDTCMod0Calculator() = default;

  virtual G4float OOTcut() const override final { return m_OOTcut; }

  virtual G4bool Process(const G4Step*, std::vector<LArHitData>&) const override final;

  virtual G4bool isInTime(G4double hitTime) const override final
  {
    return !(hitTime > m_OOTcut);
  }

private:
  // private datamember handling the hit
  Gaudi::Property<G4int> m_FCalSampling {this, "FCalSampling", 3};

  // geometry of ColdTC: overall
  Gaudi::Property<G4double> m_phiModuleStart {this, "phiModuleStart", 90.*Units::deg};
  Gaudi::Property<G4double> m_phiModuleEnd {this, "phiModuleEnd", 180.*Units::deg};
  Gaudi::Property<G4double> m_fullModuleDepth {this, "fullModuleDepth", 3.5*8*Units::cm};

  // geometry of ColdTC: active argon
  Gaudi::Property<G4double> m_fullActiveDepth {this, "fullActiveDepth", 0.2*Units::cm};
  Gaudi::Property<G4double> m_innerActiveRadius {this, "innerActiveRadius", 8.6*Units::cm};
  Gaudi::Property<G4double> m_outerActiveRadius {this, "outerActiveRadius", 45.05*Units::cm};
  Gaudi::Property<G4double> m_areaActive {this, "areaActive", 95.994*Units::cm2};

  // Channel map
  LArG4H6COLDTCMod0ChannelMap m_channelMap;
};
#endif
