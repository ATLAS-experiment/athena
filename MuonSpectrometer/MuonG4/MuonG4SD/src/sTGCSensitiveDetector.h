/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef sTGCSensitiveDetector_H
#define sTGCSensitiveDetector_H

#include "G4VSensitiveDetector.hh"
#include "MuonSimEvent/sTGCSimHitCollection.h"
#include "MuonSimEvent/sTgcHitIdHelper.h"
#include "AthenaBaseComps/AthMessaging.h"
#include <string>
#include <gtest/gtest_prod.h>

class sTgcHitIdHelper;

class sTGCSensitiveDetector : public G4VSensitiveDetector,
                              public AthMessaging {
FRIEND_TEST( sTGCSensitiveDetectortest, Initialize );
FRIEND_TEST( sTGCSensitiveDetectortest, ProcessHits );    

public:
    /** construction/destruction */
    sTGCSensitiveDetector(const std::string& name, 
                          const std::string& hitCollectionName,
                          unsigned baseDepth);
    
    /** member functions */
    void   Initialize(G4HCofThisEvent* HCE) override final;
    G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist) override final;
    
private:

    std::string m_hitCollectionName;
    sTGCSimHitCollection* m_sTGCSimHitCollection{nullptr};
    const sTgcHitIdHelper* m_muonHelper{sTgcHitIdHelper::GetHelper()};
    /** @brief basic depth to travel along the G4 history. For jobs run with the legacy geometry database,
     *         it's zero. Otherwise, in the new sqlite workflow it's 1 */
    unsigned m_baseDepth{0};

};

#endif
