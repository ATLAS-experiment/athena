/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef  LARG4H6SD_LARG4TBECBEAMCHSENSITIVEDETECTOR_H
#define  LARG4H6SD_LARG4TBECBEAMCHSENSITIVEDETECTOR_H

#include "LArG4TBECBeamChCalculator.h"
#include "G4VSensitiveDetector.hh"
#include <vector>
#include <memory>

// Forward declarations.
class LArG4TBECBeamChHit;

class LArG4TBECBeamChSensitiveDetector : public G4VSensitiveDetector
{
public:
  LArG4TBECBeamChSensitiveDetector(const G4String& name);
  ~LArG4TBECBeamChSensitiveDetector() = default;
  LArG4TBECBeamChSensitiveDetector (const LArG4TBECBeamChSensitiveDetector&) = delete;
  LArG4TBECBeamChSensitiveDetector& operator= (const LArG4TBECBeamChSensitiveDetector&) = delete;

  // The required functions for all sensitive detectors:
  virtual void Initialize(G4HCofThisEvent* HCE) override;
  virtual G4bool ProcessHits(G4Step* step, G4TouchableHistory* ROhist) override;
  virtual void EndOfEvent(G4HCofThisEvent* HCE) override;

private:
  // Pointer to a calculator class.
  std::unique_ptr<LArG4TBECBeamChCalculator> m_calculator{};

  // The name of the sensitive detector.
  G4String m_detectorName;

  // The name associated the hit collection of this sensitive
  // detector.
  G4String m_HCname;

  std::vector< std::unique_ptr<LArG4TBECBeamChHit>> m_Hits;
};

#endif // LARG4H6SD_LARG4TBECBEAMCHSENSITIVEDETECTOR_H
