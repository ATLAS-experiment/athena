/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDCG4CALIBSD_H
#define ZDCG4CALIBSD_H

#include "G4VSensitiveDetector.hh"
#include "ZDC_EscapedEnergyProcessing.h"

#include "CaloSimEvent/CaloCalibrationHit.h"
#include "CaloG4Sim/SimulationEnergies.h"
#include "StoreGate/WriteHandle.h"
#include "Identifier/Identifier.h"
#include "ZdcIdentifier/ZdcID.h"
#include "IdDict/IdDictDefs.h"

#include <gtest/gtest_prod.h>

class G4Step;
class CaloCalibrationHitContainer;

class ZDC_G4CalibSD : public G4VSensitiveDetector
{
  FRIEND_TEST( ZDC_G4CalibSDtest, ProcessHits );
  FRIEND_TEST( ZDC_G4CalibSDtest, EndOfAthenaEvent );
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
  // End of athena event processing
  void EndOfAthenaEvent();


protected:
  //Add hit either from ProcessHits or SpecialHit to the collection
  G4bool SimpleHit( const Identifier& id, const std::vector<double>& energies );
  
 private:
  SG::WriteHandle<CaloCalibrationHitContainer> m_HitColl;
  std::map< uint32_t, CaloCalibrationHit* > m_hitMap;
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
