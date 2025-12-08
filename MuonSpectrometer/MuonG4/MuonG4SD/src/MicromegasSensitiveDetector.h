/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MicromegasSensitiveDetector_H
#define MicromegasSensitiveDetector_H

#include "G4VSensitiveDetector.hh"
#include "MuonSimEvent/MMSimHitCollection.h"
#include <string>
#include <gtest/gtest_prod.h>

class MicromegasHitIdHelper;

class MicromegasSensitiveDetector : public G4VSensitiveDetector {
FRIEND_TEST( MicromegasSensitiveDetectortest, Initialize );
FRIEND_TEST( MicromegasSensitiveDetectortest, ProcessHits );

public:
    /** construction/destruction */
    MicromegasSensitiveDetector(const std::string& name, const std::string& hitCollectionName);
    
    /** member functions */
    void   Initialize(G4HCofThisEvent* HCE) override final;
    G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist) override final;
    
private:

    std::string m_hitCollectionName;
    MMSimHitCollection* m_MMSimHitCollection{nullptr};
    const MicromegasHitIdHelper* m_muonHelper;

};

#endif
