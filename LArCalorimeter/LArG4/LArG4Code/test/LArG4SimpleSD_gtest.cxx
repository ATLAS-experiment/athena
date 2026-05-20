/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

#include "LArG4Code/LArG4SimpleSD.h"

#include "gtest/gtest.h"

#include "TestTools/initGaudi.h"

#include "G4HCofThisEvent.hh"
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

#include "CaloIdentifier/LArEM_ID.h"
#include "CaloIdentifier/LArFCAL_ID.h"
#include "CaloIdentifier/LArHEC_ID.h"
#include "CaloIdentifier/CaloDM_ID.h"

#include "G4AtlasTools/DerivedG4PhysicalVolume.h"
#include "LArG4Code/DerivedILArCalculatorSvcForTest.h"
#include "LArG4Code/LArHitContainerBuilder.h"

#include <memory>

//set environment
class GaudiEnvironment : public ::testing::Environment {
  protected:
  virtual void SetUp() override {
    Athena_test::initGaudi("LArG4Code/optionForTest.txt", m_svcLoc);
  }
  ISvcLocator* m_svcLoc = nullptr;
};
class LArG4SimpleSDtest : public ::testing::Test {	
  protected:
    virtual void SetUp() override {
    }

    virtual void TearDown() override {
    }

// Here I initialize 3 Identifier helper class objects, and they are used to convert a set of numbers stored in a LArG4Identifier object into a Identifier object.
// they can be used by all the TEST_F
  LArEM_ID m_EM;
  LArFCAL_ID m_FCAL;
  LArHEC_ID m_HEC;
};
//end

TEST_F( LArG4SimpleSDtest, ProcessHits )
{
  G4Step* aStep = new G4Step();//define actual parameter for the tested member function ProcessHits
  double totEnergyDeposit = 1.0;
  aStep->SetTotalEnergyDeposit(totEnergyDeposit);//set total energy deposit value for the G4Step object
  G4TouchableHistory* th = new G4TouchableHistory();//define actual parameter for the tested member function ProcessHits

  DerivedILArCalculatorSvcForTest* calc = new DerivedILArCalculatorSvcForTest();//use the derived ILArCalculatorSvc class since ILArCalculatorSvc is abstact and can not be instantiated
  LArG4SimpleSD sd1("name1", calc, "LArHitTest");//instantiate the tested class LArG4SimpleSD
  sd1.setupHelpers(&m_EM, &m_FCAL, &m_HEC);//add helpers(&m_EM, &m_FCAL, &m_HEC) which can convert a set of numbers stored in LArG4Identifier object into a compact number stored in a Identifier object 
  ASSERT_FALSE(sd1.ProcessHits(aStep, th));
}

TEST_F( LArG4SimpleSDtest, SimpleHit )
{
// Add a bunch of numbers into the object a_ident and this kind of number setting is intented to run the "if(a_ident[0]==4) {if(a_ident[1]==1) ...}" block when the member function invokes the member function ConvertID.
  LArG4Identifier a_ident;
  a_ident.add(4);
  a_ident.add(2);
  a_ident.add(2);
  a_ident.add(3);
  a_ident.add(4);
  a_ident.add(5);
  a_ident.add(6);
  double time = 1.0;//define actual parameter for the tested function SimpleHit, the same below
  double energy = 1.0;

  DerivedILArCalculatorSvcForTest* calc = new DerivedILArCalculatorSvcForTest();//use the derived ILArCalculatorSvc class since ILArCalculatorSvc is abstact and can not be instantiated
  LArG4SimpleSD sd2("name2", calc, "LArHitTest");//instantiate the tested class LArG4SimpleSD
  sd2.setupHelpers(&m_EM, &m_FCAL, &m_HEC);//add helpers(&m_EM, &m_FCAL, &m_HEC), which can convert a set of numbers stored in LArG4Identifier object into a compact number stored in a Identifier object 
  ASSERT_FALSE(sd2.SimpleHit(a_ident, time, energy));
}

TEST_F( LArG4SimpleSDtest, LArHitContainerBuilder )
{
  LArHitContainerBuilder builder("name");
  builder.AddHit("", std::make_unique<LArHit>(Identifier(), 1.0, 1.0), 0);
  builder.Finalize();
  LArHit* a = *(builder.begin());
  ASSERT_EQ(1.0, a->energy());//test if the energy is 1.0 as the value we just set
  ASSERT_EQ(1.0, a->time());//test if the time is 1.0 as the value we just set
}

TEST_F( LArG4SimpleSDtest, LArHitContainerBuilderPartitions )
{
  LArHitContainerBuilder builder("name");
  builder.RegisterSource("sd1");
  builder.RegisterSource("sd2");
  builder.AddHit("sd1", std::make_unique<LArHit>(Identifier(), 1.0, 1.0), 0);
  builder.AddHit("sd1", std::make_unique<LArHit>(Identifier(), 2.0, 1.0), 0);
  builder.AddHit("sd2", std::make_unique<LArHit>(Identifier(), 3.0, 1.0), 0);
  builder.AddHit("", std::make_unique<LArHit>(Identifier(), 4.0, 1.0), 0);
  builder.Finalize();

  ASSERT_EQ(3u, builder.size());
}

TEST_F( LArG4SimpleSDtest, ConvertID )
{
  LArG4Identifier a_ident;
  a_ident.add(4);
  a_ident.add(2);
  a_ident.add(2);
  a_ident.add(3);
  a_ident.add(4);
  a_ident.add(5);
  a_ident.add(6);
//here I decorate the LArG4Identifier object a_ident with a set of numbers to make it complete.
//This kind of setting for a_ident will make the "else if(a_ident[1]==2)" block of the member function ConvertID be tested. you can change the setting to test other block

  DerivedILArCalculatorSvcForTest* calc = new DerivedILArCalculatorSvcForTest();//use the derived ILArCalculatorSvc class since ILArCalculatorSvc is abstact and can not be instantiated
  LArG4SimpleSD sd4("name4", calc, "LArHitTest");//instantiate the tested class LArG4SimpleSD
  sd4.setupHelpers(&m_EM, &m_FCAL, &m_HEC);//add helpers(&m_EM, &m_FCAL, &m_HEC), which can convert a set of numbers stored in LArG4Identifier object into a compact number stored in a Identifier object 
  Identifier id = sd4.ConvertID(a_ident); //generally speaking, a set of number was compact into a single number stored in id

  unsigned long long compact_num = id.get_compact();
  ASSERT_TRUE(id.is_valid()); //test if the Identifier object id is valid
  ASSERT_EQ(compact_num, 7u); //Based on previous number setting, the compact number should equal to 15. Test it here
}

TEST_F( LArG4SimpleSDtest, getTimeBin )
{
  double aTime = 75.0;//set actual parameter for the tested member funtion getTimeBin (G4double time)
  
  DerivedILArCalculatorSvcForTest* calc = new DerivedILArCalculatorSvcForTest();//use the derived ILArCalculatorSvc class since ILArCalculatorSvc is abstact and can not be instantiated
  LArG4SimpleSD sd5("name5", calc, "LArHitTest");//instantiate the tested class LArG4SimpleSD
  sd5.setupHelpers(&m_EM, &m_FCAL, &m_HEC);//add helpers(&m_EM, &m_FCAL, &m_HEC), which can convert a set of numbers stored in LArG4Identifier object into a compact number stored in a Identifier object
  double output = sd5.getTimeBin(aTime);//invoke the tested member function, and its output should be 13 according to the member function definition

  ASSERT_EQ(output, 13);//test if the output is as we expected


}

int main( int argc, char** argv ) {

  auto *g=new GaudiEnvironment;
  ::testing::AddGlobalTestEnvironment(g);
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();

}
