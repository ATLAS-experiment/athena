/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @author Elmar Ritsch <Elmar.Ritsch@cern.ch>
 * @date October, 2016
 * @brief Tests for ISF::GenParticleInteractingFilter.
 */

#undef NDEBUG

// Google Test
#include "gtest/gtest.h"
#include "GoogleTestTools/InitGaudiGoogleTest.h"

// tested AthAlgTool
#include "../src/GenParticleInteractingFilter.h"

// Truth related includes
#include "AtlasHepMC/GenParticle.h"
#include "AtlasHepMC/GenVertex.h"
#include "AtlasHepMC/SimpleVector.h"


namespace ISFTesting {

// Test fixture specifically for SimKernelMT AthAlgorithm
class GenParticleInteractingFilter_test: public Athena_test::InitGaudiGoogleTest {

protected:
  virtual void SetUp() override {
    // the tested tool
    IAlgTool* tool = nullptr;
    EXPECT_TRUE( toolSvc->retrieveTool("ISF::GenParticleInteractingFilter/TestGenParticleInteractingFilter", tool).isSuccess() );
    m_filterTool = dynamic_cast<ISF::GenParticleInteractingFilter*>(tool);
  }

  virtual void TearDown() override {
    for (size_t refCount = m_filterTool->refCount(); refCount>0; refCount--) {
      StatusCode sc = toolSvc->releaseTool(m_filterTool);
      ASSERT_TRUE( sc.isSuccess() );
    }
  }

  // the tested AthAlgTool
  ISF::GenParticleInteractingFilter* m_filterTool = nullptr;

};  // GenParticleInteractingFilter_test fixture


TEST_F(GenParticleInteractingFilter_test, allPropertiesUnset_stdParticle_expectPass) {
  EXPECT_TRUE( m_filterTool->initialize().isSuccess() );

  const HepMC::FourVector mom4(1.0*sin(150.*M_PI/180.), 0.0, 1.0*cos(150.*M_PI/180.), 1.0); // rho=1, eta=-1.32
#ifdef HEPMC3
  HepMC::GenParticlePtr part=HepMC::newGenParticlePtr(mom4, /*pdg id=*/11);
#else
  const HepMC::GenParticle part(mom4, /*pdg id=*/11);
#endif
  ASSERT_TRUE( m_filterTool->pass(part) );
}


TEST_F(GenParticleInteractingFilter_test, allPropertiesUnset_stdParticle_expectNoPass) {
  EXPECT_TRUE( m_filterTool->initialize().isSuccess() );

  const HepMC::FourVector mom4(1.0*sin(150.*M_PI/180.), 0.0, 1.0*cos(150.*M_PI/180.), 1.0); // rho=1, eta=-1.32
#ifdef HEPMC3
  HepMC::GenParticlePtr part=HepMC::newGenParticlePtr(mom4, /*pdg id=*/12);
#else
  const HepMC::GenParticle part(mom4, /*pdg id=*/12);
#endif
  ASSERT_FALSE( m_filterTool->pass(part) );
}


 // Temporarily commenting out as after MR!74633 Monopoles count as
 // intercting without any special settings - need to use a different
 // kind of exotic particle for this test - see ATLASSIM-7301

// TEST_F(GenParticleInteractingFilter_test, allPropertiesUnset_exoticParticle_expectNoPass) {
//   EXPECT_TRUE( m_filterTool->initialize().isSuccess() );

//   const HepMC::FourVector mom4(1.0*sin(150.*M_PI/180.), 0.0, 1.0*cos(150.*M_PI/180.), 1.0); // rho=1, eta=-1.32
// #ifdef HEPMC3
//   HepMC::GenParticlePtr part=HepMC::newGenParticlePtr(mom4, /*pdg id=*/4110000);
// #else
//   const HepMC::GenParticle part(mom4, /*pdg id=*/4110000);
// #endif

//   ASSERT_FALSE( m_filterTool->pass(part) );
// }


TEST_F(GenParticleInteractingFilter_test, stdParticle_expectPass) {
  EXPECT_TRUE( m_filterTool->setProperty("AdditionalInteractingParticleTypes", "[4110000]").isSuccess() );
  EXPECT_TRUE( m_filterTool->initialize().isSuccess() );

  const HepMC::FourVector mom4(1.0*sin(150.*M_PI/180.), 0.0, 1.0*cos(150.*M_PI/180.), 1.0); // rho=1, eta=-1.32
#ifdef HEPMC3
  HepMC::GenParticlePtr part=HepMC::newGenParticlePtr(mom4, /*pdg id=*/11);
#else
  const HepMC::GenParticle part(mom4, /*pdg id=*/11);
#endif
  ASSERT_TRUE( m_filterTool->pass(part) );
}


TEST_F(GenParticleInteractingFilter_test, stdParticle_expectNoPass) {
  EXPECT_TRUE( m_filterTool->setProperty("AdditionalInteractingParticleTypes", "[4110000]").isSuccess() );
  EXPECT_TRUE( m_filterTool->initialize().isSuccess() );

  const HepMC::FourVector mom4(1.0*sin(150.*M_PI/180.), 0.0, 1.0*cos(150.*M_PI/180.), 1.0); // rho=1, eta=-1.32
#ifdef HEPMC3
  HepMC::GenParticlePtr part=HepMC::newGenParticlePtr(mom4, /*pdg id=*/12);
#else
  const HepMC::GenParticle part(mom4, /*pdg id=*/12);
#endif
  ASSERT_FALSE( m_filterTool->pass(part) );
}


TEST_F(GenParticleInteractingFilter_test, exoticParticle_expectPass) {
  EXPECT_TRUE( m_filterTool->setProperty("AdditionalInteractingParticleTypes", "[4110000]").isSuccess() );
  EXPECT_TRUE( m_filterTool->initialize().isSuccess() );

  const HepMC::FourVector mom4(1.0*sin(150.*M_PI/180.), 0.0, 1.0*cos(150.*M_PI/180.), 1.0); // rho=1, eta=-1.32
#ifdef HEPMC3
  HepMC::GenParticlePtr part=HepMC::newGenParticlePtr(mom4, /*pdg id=*/4110000);
#else
  const HepMC::GenParticle part(mom4, /*pdg id=*/4110000);
#endif

  ASSERT_TRUE( m_filterTool->pass(part) );
}

} // namespace ISFTesting


int main(int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
  // if the above gets stuck forever while trying to finalize Boost stuff
  // inside SGTools, try to use that:
  //  skips proper finalization:
  //std::quick_exit( RUN_ALL_TESTS() );
}
