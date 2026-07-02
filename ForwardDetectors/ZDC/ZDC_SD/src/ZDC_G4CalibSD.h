/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDCG4CALIBSD_H
#define ZDCG4CALIBSD_H

#include "G4VSensitiveDetector.hh"
#include "ZDC_EscapedEnergyProcessing.h"
#include "ZDC_HitCollectionBuilders.h"

#include "CaloSimEvent/CaloCalibrationHit.h"
#include "CaloG4Sim/SimulationEnergies.h"
#include "Identifier/Identifier.h"
#include "ZdcIdentifier/ZdcID.h"

#include <gtest/gtest_prod.h>

class G4Step;
class G4Track;
class CaloCalibrationHitContainer;

class ZDC_G4CalibSD : public G4VSensitiveDetector
{
  FRIEND_TEST( ZDC_G4CalibSDtest, ProcessHits );
  FRIEND_TEST( ZDC_G4CalibSDtest, SpecialHit );
  FRIEND_TEST( ZDC_G4CalibSDtest, SimpleHit );
public:

  // Constructor
  ZDC_G4CalibSD(const G4String &a_name, const G4String& hitCollectionName, bool doPID=false);

  // Destructor
  virtual ~ZDC_G4CalibSD();

  ZDC_G4CalibSD(const ZDC_G4CalibSD&) = delete;
  ZDC_G4CalibSD& operator=(const ZDC_G4CalibSD&) = delete;

  // Main processing method
  G4bool ProcessHits(G4Step* a_step,G4TouchableHistory*) override;
  // For other classes that need to call into us...
  G4bool SpecialHit(G4Step* a_step, const std::vector<G4double>& a_energies);
protected:
  //Add hit either from ProcessHits or SpecialHit to the collection
  G4bool SimpleHit( const Identifier& id, const std::vector<double>& energies, const G4Track* track = nullptr );
  
 private:
  ZDC_CalibrationHitContainerBuilder* getHitCollection() const;

  std::string m_hitCollectionName;
  // Non-owning cache; HitCollectionMap owns the collection.
  ZDC_CalibrationHitContainerBuilder* m_HitColl{};
  std::vector<G4double> m_energies;

  // Count the number of invalid hits.
  G4int m_numberInvalidHits;
  // Are we set up to run with PID hits?
  G4bool m_doPID;
  Identifier m_id;
  CaloG4::SimulationEnergies *m_simulationEnergies;
  ZDC_EscapedEnergyProcessing *m_zdc_eep = nullptr;
};

#endif
