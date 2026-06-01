/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

#include "src/ZDC_FiberSD.h"
#include "gtest/gtest.h"

#include <vector>

#include "TestTools/initGaudi.h"

#include "G4Step.hh"
#include "G4TouchableHistory.hh"


#include "G4Track.hh"
#include "G4StepPoint.hh"
#include "G4DynamicParticle.hh"
#include "G4ThreeVector.hh"
#include "G4Box.hh"
#include "G4NistManager.hh"
#include "G4Material.hh"
#include "G4VPhysicalVolume.hh"
#include "G4SystemOfUnits.hh"

#include "G4AtlasTools/DerivedG4PhysicalVolume.h"
#include "G4AtlasTools/DerivedG4SensitiveDetectorTestSetting.h"
#include "src/ZDC_HitCollectionBuilders.h"

//set environment
class GaudiEnvironment : public ::testing::Environment {
  protected:
    virtual void SetUp() override {
   Athena_test::initGaudi("ZDC_SD/optionForTest.txt", m_svcLoc);
  }
  ISvcLocator* m_svcLoc = nullptr;
};
class ZDC_FiberSDtest : public ::testing::Test {	
  protected:
    virtual void SetUp() override {
    }

    virtual void TearDown() override {
    }
};
//end of environment setting

TEST_F( ZDC_FiberSDtest, ProcessHits )
{
  G4Step sp;
  G4TouchableHistory th;
  ZDC_SimFiberHitCollectionBuilder hitCollection("name2");

  G4double totalenergydeposit = 0.8;
  std::vector<G4String> physicalname = {"physicsTDQuarticBar[9]"};
  G4String logicalname = "BBBBBBBBBFiberCore";
  std::vector<G4int> copynos = {-333424640};
  G4ThreeVector preStepPos = G4ThreeVector(0,0,1);
  G4ThreeVector postStepPos = G4ThreeVector(0,0,2);
  G4double globaltime0 = 0.5;
  G4double kineticenergy0 = 1.5;
  G4double velocity0 = 2500;
  G4double globaltime = 5.0;
  G4double kineticenergy = 0.5;
  G4double globaltime1 = 0.5;
  G4double kineticenergy1 = 0.5;
  G4double velocity1 = 2500;
  G4double steplength = 1.0;
  G4double charge = 1.0;
  G4int encoding = 22;
  G4int antiencoding = 22;
  G4String astring = "Cerenkov";
  G4ProcessType atype = (G4ProcessType)0;
  G4String nop1 = "opticalphoton";
  G4String nop2 = "opticalphoton";
  G4String nop3 = "photon";
  DerivedG4SensitiveDetectorTestSetting(sp, totalenergydeposit, physicalname, logicalname, copynos, preStepPos, postStepPos, globaltime0, kineticenergy0, velocity0, globaltime, kineticenergy, globaltime1, kineticenergy1, velocity1, steplength, charge, encoding, antiencoding, astring, atype, nop1, nop2, nop3);

  ZDC_FiberSD sd2("name2", "name2", 511.8);
  sd2.m_HitColl = &hitCollection;
  sd2.ProcessHits(&sp, &th);

  ASSERT_NE(sd2.m_HitColl, nullptr);
}

int main( int argc, char** argv ) {

  auto g=new GaudiEnvironment;
  ::testing::AddGlobalTestEnvironment(g);
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();

}
