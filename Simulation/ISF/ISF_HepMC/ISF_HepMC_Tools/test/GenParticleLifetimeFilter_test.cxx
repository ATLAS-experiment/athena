/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @brief Tests for ISF::GenParticleLifetimeFilter.
 */

#undef NDEBUG

// Google Test
#include "gtest/gtest.h"
#include "GoogleTestTools/InitGaudiGoogleTest.h"

// tested AthAlgTool
#include "../src/GenParticleLifetimeFilter.h"

// Truth related includes
#include "AtlasHepMC/GenParticle.h"
#include "AtlasHepMC/GenVertex.h"
#include "AtlasHepMC/SimpleVector.h"


namespace ISFTesting {

// Test fixture specifically for SimKernelMT AthAlgorithm
class GenParticleLifetimeFilter_test: public Athena_test::InitGaudiGoogleTest {

protected:
  virtual void SetUp() override {
    // the tested tool
    IAlgTool* tool = nullptr;
    EXPECT_TRUE( toolSvc->retrieveTool("ISF::GenParticleLifetimeFilter/TestGenParticleLifetimeFilter", tool).isSuccess() );
    m_filterTool = dynamic_cast<ISF::GenParticleLifetimeFilter*>(tool);
  }

  virtual void TearDown() override {
    for (size_t refCount = m_filterTool->refCount(); refCount>0; refCount--) {
      StatusCode sc = toolSvc->releaseTool(m_filterTool);
      ASSERT_TRUE( sc.isSuccess() );
    }
  }

  // the tested AthAlgTool
  ISF::GenParticleLifetimeFilter* m_filterTool = nullptr;

};  // GenParticleLifetimeFilter_test fixture


TEST_F(GenParticleLifetimeFilter_test, allPropertiesUnset_expectPass) {
  EXPECT_TRUE( m_filterTool->initialize().isSuccess() );

#ifdef HEPMC3
   auto part = HepMC::newGenParticlePtr();
#else
  const HepMC::GenParticle part{};
#endif
  ASSERT_TRUE( m_filterTool->pass(part) ); // will pass as no end vertex
}


TEST_F(GenParticleLifetimeFilter_test, setMinimumLifetime_expectPass) {
  EXPECT_TRUE( m_filterTool->setProperty("MinimumLifetime", "1.3").isSuccess() );
  EXPECT_TRUE( m_filterTool->initialize().isSuccess() );

#ifdef HEPMC3
   auto part = HepMC::newGenParticlePtr();
#else
  const HepMC::GenParticle part{};
#endif
  ASSERT_TRUE( m_filterTool->pass(part) ); // will pass as no end vertex
}


TEST_F(GenParticleLifetimeFilter_test, addProdVtx_expectPass) {
  EXPECT_TRUE( m_filterTool->setProperty("MinimumLifetime", "1.3").isSuccess() );
  EXPECT_TRUE( m_filterTool->setProperty("OutputLevel", "3").isSuccess() );
  EXPECT_TRUE( m_filterTool->initialize().isSuccess() );

  const HepMC::FourVector prodPos(0., 0., 0., 0.);
#ifdef HEPMC3
  HepMC::GenVertexPtr prodVtx = HepMC::newGenVertexPtr(prodPos);
  auto part = HepMC::newGenParticlePtr(); // need dynamic allocation as GenVertex takes ownership
  prodVtx->add_particle_out(part);
  ASSERT_TRUE( m_filterTool->pass(part) ); // will pass as no end vertex
#else  
  HepMC::GenVertex prodVtx(prodPos);
  auto part = HepMC::newGenParticlePtr(); // need dynamic allocation as GenVertex takes ownership
  prodVtx.add_particle_out(part);

  ASSERT_TRUE( m_filterTool->pass(*part) ); // will pass as no end vertex
#endif
}


TEST_F(GenParticleLifetimeFilter_test, minLifetimeGreaterThanParticleLifetime_expectNoPass) {
  EXPECT_TRUE( m_filterTool->setProperty("MinimumLifetime", "1.3").isSuccess() );
  EXPECT_TRUE( m_filterTool->initialize().isSuccess() );

  const HepMC::FourVector prodPos(0., 0., 0., 0.);
  const HepMC::FourVector endPos(0., 0., 0., 1.);

#ifdef HEPMC3
  HepMC::GenVertexPtr prodVtx=HepMC::newGenVertexPtr(prodPos);
  auto part = HepMC::newGenParticlePtr(); // need dynamic allocation as GenVertex takes ownership
  prodVtx->add_particle_out(part);
  HepMC::GenVertexPtr endVtx = HepMC::newGenVertexPtr (endPos);
  endVtx->add_particle_in(part);
  ASSERT_FALSE( m_filterTool->pass(part) ); // will fail as particle lifetime is only 1.0
#else
  HepMC::GenVertex prodVtx(prodPos);
  auto part = HepMC::newGenParticlePtr(); // need dynamic allocation as GenVertex takes ownership
  prodVtx.add_particle_out(part);
  HepMC::GenVertex endVtx(endPos);
  endVtx.add_particle_in(part);
  ASSERT_FALSE( m_filterTool->pass(*part) ); // will fail as particle lifetime is only 1.0
//AV: Memory leak as part is not deallocated?
#endif
}


TEST_F(GenParticleLifetimeFilter_test, minLifetimeLessThanParticleLifetime_expectPass) {
  EXPECT_TRUE( m_filterTool->setProperty("MinimumLifetime", "1.3").isSuccess() );
  EXPECT_TRUE( m_filterTool->initialize().isSuccess() );

  const HepMC::FourVector prodPos(0., 0., 0., 0.);
  const HepMC::FourVector endPos(0., 0., 0., 2.);
#ifdef HEPMC3
  HepMC::GenVertexPtr prodVtx =  HepMC::newGenVertexPtr(prodPos);
  auto part = HepMC::newGenParticlePtr(); // need dynamic allocation as GenVertex takes ownership
  prodVtx->add_particle_out(part);
  HepMC::GenVertexPtr endVtx = HepMC::newGenVertexPtr(endPos);
  endVtx->add_particle_in(part);
  ASSERT_TRUE( m_filterTool->pass(part) ); // will pass as particle lifetime is 2.0
#else
  HepMC::GenVertex prodVtx(prodPos);
  auto part = HepMC::newGenParticlePtr(); // need dynamic allocation as GenVertex takes ownership
  prodVtx.add_particle_out(part);
  HepMC::GenVertex endVtx(endPos);
  endVtx.add_particle_in(part);
  ASSERT_TRUE( m_filterTool->pass(*part) ); // will pass as particle lifetime is 2.0
//AV: Memory leak as part is not deallocated?
#endif
}


TEST_F(GenParticleLifetimeFilter_test, endVtxButNoProdVtx_expectNoPass) {
  EXPECT_TRUE( m_filterTool->setProperty("MinimumLifetime", "1.3").isSuccess() );
  EXPECT_TRUE( m_filterTool->initialize().isSuccess() );

  auto part = HepMC::newGenParticlePtr(); // need dynamic allocation as GenVertex takes ownership
  const HepMC::FourVector endPos(0., 0., 0., 2.);
#ifdef HEPMC3
  HepMC::GenVertexPtr endVtx = HepMC::newGenVertexPtr(endPos);
  endVtx->add_particle_in(part);
  ASSERT_FALSE( m_filterTool->pass(part) ); // will fail as prodVtx undefined
#else
  HepMC::GenVertex endVtx(endPos);
  endVtx.add_particle_in(part);
  ASSERT_FALSE( m_filterTool->pass(*part) ); // will fail as prodVtx undefined
#endif
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
