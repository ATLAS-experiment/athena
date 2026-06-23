/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

#include "src/ZDC_G4CalibSD.h"

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
#include "G4RunManager.hh"


#include "G4AtlasTools/DerivedG4PhysicalVolume.h"
#include "G4AtlasTools/DerivedG4SensitiveDetectorTestSetting.h"
#include "CaloSimEvent/CaloCalibrationHitContainer.h"
#include "src/ZDC_HitCollectionBuilders.h"

//set environment
class GaudiEnvironment : public ::testing::Environment {
  protected:
  virtual void SetUp() override {
    Athena_test::initGaudi("ZDC_SD/optionForTest.txt", m_svcLoc);
  }
  ISvcLocator* m_svcLoc = nullptr;
};
class ZDC_G4CalibSDtest : public ::testing::Test {	
  protected:
    virtual void SetUp() override {
    }

    virtual void TearDown() override {
    }
};
//end

TEST_F( ZDC_G4CalibSDtest, ProcessHits )
{
  ZDC_CalibrationHitContainerBuilder hitCollection("name1");
  G4Step aStep;
  G4TouchableHistory th;

  ZDC_G4CalibSD sd1("name1", "name1", false);
  sd1.m_HitColl = &hitCollection;
  sd1.ProcessHits(&aStep, &th);
  
  //TODO: Create the actual tests
}

TEST_F( ZDC_G4CalibSDtest, SpecialHit )
{
  ZDC_CalibrationHitContainerBuilder hitCollection("name2");

// define aStep and energies as the actual parameters of the member function SpecialHit 
  G4Step aStep;
  std::vector<G4double> energies = {1., 2., 3., 4.};

  G4double totalenergydeposit = 0.8;
  std::vector<G4String> physicalname = {"ZDC::Strip 0x-13DFA800"};
  G4String logicalname = "ZDC::Strip_Logical";
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
  G4int encoding = 211;
  G4int antiencoding = -211;
  G4String astring = "Cerenkov";
  G4ProcessType atype = (G4ProcessType)0;
  G4String nop1 = "pion";
  G4String nop2 = "pion";
  G4String nop3 = "pion";
  DerivedG4SensitiveDetectorTestSetting(aStep, totalenergydeposit, physicalname, logicalname, copynos, preStepPos, postStepPos, globaltime0, kineticenergy0, velocity0, globaltime, kineticenergy, globaltime1, kineticenergy1, velocity1, steplength, charge, encoding, antiencoding, astring, atype, nop1, nop2, nop3);


  ZDC_G4CalibSD sd2("name2", "name2", false);
  sd2.m_HitColl = &hitCollection;
  sd2.SpecialHit(&aStep, energies);//this member function is intended to store a newly-generated hit in to the hit collection calibrationHits with a kind of specific order

  //TODO: Create the actual tests
}

TEST_F( ZDC_G4CalibSDtest, SimpleHit )
{
  ZDC_CalibrationHitContainerBuilder hitCollection("name4");
// the member function SimpleHit aims to generate a hit and store it in a hit container
// Add a bunch of numbers into the object a_ident and this kind of number setting is intented to run the "if(a_ident[0]==4) {if(a_ident[1]==1) ...}" block

// Set energies
  std::vector<double> energies = {1., 2., 3., 4.};
  Identifier a_ident;

  ZDC_G4CalibSD sd6("name4", "name4", false);
  sd6.m_HitColl = &hitCollection;
  sd6.SimpleHit(a_ident, energies); //this member function is intended to store a newly-generated hit in to the hit collection calibrationHits with a kind of specific order

  //TODO: Create the actual tests
}

int main( int argc, char** argv ) {

  auto g=new GaudiEnvironment;
  ::testing::AddGlobalTestEnvironment(g);
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();

}
