/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#undef NDEBUG

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;  // unit test

/// std::abs
#include <cmath>

// Framework
#include "TestTools/initGaudi.h"

// Google Test
#include "gtest/gtest.h"

#include "ISF_Event/ISFTruthIncident.h"

// inputs to truthincident
#include "ISF_Event/ISFParticle.h"

// children container typedefs
#include "ISF_Event/ISFParticleContainer.h"

#include "GeoPrimitives/GeoPrimitives.h"

#include "AtlasDetDescr/AtlasRegion.h"
#include "BarcodeEvent/Barcode.h"

// HepMC includes
#include "AtlasHepMC/GenParticle.h"
#include "AtlasHepMC/GenVertex.h"
#include "AtlasHepMC/GenEvent.h"

// Athena headers
#include "TruthUtils/MagicNumbers.h"
#include "StoreGate/WriteHandle.h"
#include "StoreGate/StoreGateSvc.h"
#include "GeneratorObjects/McEventCollection.h"
#include "GeneratorObjects/HepMcParticleLink.h"

namespace MCTesting {


  namespace test {

    // global mock data used by several TruthIncident tests
    const ISF::DetRegionSvcIDPair origin( AtlasDetDescr::fAtlasCalo, 2 );
   // set this to conversion
    Barcode::PhysicsProcessCode procBC = 14;
    // rounding
    double eps = pow(10,-8);

  } // end of test namespace

  // needed every time an AthAlgorithm, AthAlgTool or AthService is instantiated
  ISvcLocator* g_svcLoc = nullptr;

  // global test environment takes care of setting up Gaudi
  class GaudiEnvironment : public ::testing::Environment {
  protected:
    virtual void SetUp() override {
      Athena_test::initGaudi(MCTesting::g_svcLoc);
    }
  };

  class ISFTruthIncident_test : public ::testing::Test {

  protected:
    virtual void SetUp() override {
      SG::WriteHandle<McEventCollection> inputTestDataHandle{"TruthEvent"};
      inputTestDataHandle = std::make_unique<McEventCollection>();

      HepMC::GenEvent* pEvent(buildEvent());
      inputTestDataHandle->push_back(pEvent);

      buildMockData(pEvent);
    }

    //cut-and-paste from HepMC/examples
    HepMC::GenEvent* buildEvent() {
      //
      // In this example we will place the following event into HepMC "by hand"
      //
      //     name status pdg_id  parent Px       Py    Pz       Energy      Mass
      //  1  !p+!    3   2212    0,0    0.000    0.000 7000.000 7000.000    0.938
      //  2  !p+!    3   2212    0,0    0.000    0.000-7000.000 7000.000    0.938
      //=========================================================================
      //  3  !d!     3      1    1,1    0.750   -1.569   32.191   32.238    0.000
      //  4  !u~!    3     -2    2,2   -3.047  -19.000  -54.629   57.920    0.000
      //  5  !W-!    3    -24    1,2    1.517   -20.68  -20.605   85.925   80.799
      //  6  !gamma! 1     22    1,2   -3.813    0.113   -1.833    4.233    0.000
      //  7  !d!     1      1    5,5   -2.445   28.816    6.082   29.552    0.010
      //  8  !u~!    1     -2    5,5    3.962  -49.498  -26.687   56.373    0.006


      // First create the event container, with Signal Process 20, event number 1
      HepMC::GenEvent* evt = HepMC::newGenEvent( 20, 1 );
      //
      // create vertex 1 and vertex 2, together with their inparticles
      HepMC::GenVertexPtr v1 = HepMC::newGenVertexPtr();
      evt->add_vertex( v1 );
      HepMC::GenParticlePtr p1 =
        HepMC::newGenParticlePtr( HepMC::FourVector(0,0,7000,7000),
                                  2212, 3 );
      v1->add_particle_in(  p1 );
      HepMC::suggest_barcode(p1,10001);
      HepMC::GenVertexPtr v2 = HepMC::newGenVertexPtr();
      evt->add_vertex( v2 );
      HepMC::GenParticlePtr p2 = HepMC::newGenParticlePtr(  HepMC::FourVector(0,0,-7000,7000),
                                                            2212, 3 );
      v2->add_particle_in( p2 );
      HepMC::suggest_barcode(p2,10002);
      //
      // create the outgoing particles of v1 and v2
      HepMC::GenParticlePtr p3 =
        HepMC::newGenParticlePtr( HepMC::FourVector(.750,-1.569,32.191,32.238), 1, 3 );
      v1->add_particle_out( p3 );
      HepMC::suggest_barcode(p3,10003);
      HepMC::GenParticlePtr p4 =
        HepMC::newGenParticlePtr( HepMC::FourVector(-3.047,-19.,-54.629,57.920), -2, 3 );
      v2->add_particle_out( p4 );
      HepMC::suggest_barcode(p4,10004);
      //
      // create v3
      HepMC::GenVertexPtr v3 = HepMC::newGenVertexPtr();
      evt->add_vertex( v3 );
      v3->add_particle_in( p3 );
      v3->add_particle_in( p4 );
      HepMC::GenParticlePtr p5 =
        HepMC::newGenParticlePtr( HepMC::FourVector(-3.813,0.113,-1.833,4.233 ), 22, 1 );
      v3->add_particle_out( p5   );
      HepMC::suggest_barcode(p5,10005);
      HepMC::GenParticlePtr p6 =
        HepMC::newGenParticlePtr( HepMC::FourVector(1.517,-20.68,-20.605,85.925), -24,3);
      v3->add_particle_out( p6 );
      HepMC::suggest_barcode(p6,10006);
      //
      // create v4
      HepMC::GenVertexPtr v4 = HepMC::newGenVertexPtr();
      evt->add_vertex( v4 );
      v4->add_particle_in( p6 );
      HepMC::GenParticlePtr p7 =
        HepMC::newGenParticlePtr( HepMC::FourVector(-2.445,28.816,6.082,29.552), 1,1 );
      v4->add_particle_out( p7 );
      HepMC::suggest_barcode(p7,10007);
      HepMC::GenParticlePtr p8 =
        HepMC::newGenParticlePtr( HepMC::FourVector(3.962,-49.498,-26.687,56.373), -2,1 );
      v4->add_particle_out( p8 );
      HepMC::suggest_barcode(p8,10008);
      //
      // tell the event which vertex is the signal process vertex
      HepMC::set_signal_process_vertex(evt, v3 );
      // the event is complete, we now print it out to the screen
#ifdef GENP_DEBUG
      std::cout << evt << std::endl;
#endif

      // delete all particle data objects in the particle data table pdt
      //pdt.delete_all();
      return evt;
    }

    void buildMockData(HepMC::GenEvent* ) {
      // TODO Link ISFParticles to McEventCollection
      // generate ISFParticle parent and children
      Amg::Vector3D pos(1., 0., 2.);
      Amg::Vector3D mom(5., 4., 3.);
      double mass    = 0.;
      double charge  = 987.;
      int    pdgCode = 675;
      int status     =  200045;
      double time    = 923.;
      Barcode::ParticleBarcode partBC = 1;
      Barcode::ParticleBarcode partID = 1;
      ISF::TruthBinding *truth = 0;
      Barcode::ParticleBarcode part2BC = 2;
      Barcode::ParticleBarcode part2ID = 2;
      m_isp1 = std::make_unique<ISF::ISFParticle>(
                                                  pos,
                                                  mom,
                                                  mass,
                                                  charge,
                                                  pdgCode,
                                                  status,
                                                  time,
                                                  test::origin,
                                                  partID,
                                                  partBC,
                                                  truth );
      m_isp2 = std::make_unique<ISF::ISFParticle>(
                                                  pos,
                                                  mom,
                                                  mass,
                                                  charge,
                                                  pdgCode,
                                                  status,
                                                  time,
                                                  *(m_isp1.get()), // parent
                                                  part2ID,
                                                  part2BC,
                                                  truth );

      m_pvec_children = std::make_unique<ISF::ISFParticleVector>();
      m_pvec_children->push_back(m_isp2.get());
      // generate truth incident
      m_truthIncident = std::make_unique<ISF::ISFTruthIncident>(*(m_isp1.get()),
                                                                *(m_pvec_children.get()),
                                                                test::procBC,
                                                                test::origin.first);
    }
    virtual void TearDown() override {
      StoreGateSvc* pStore(nullptr);
      ASSERT_TRUE(MCTesting::g_svcLoc->service("StoreGateSvc", pStore).isSuccess());
      pStore->clearStore(true).ignore(); // forceRemove=true to remove all proxies
    }

    std::unique_ptr<ISF::ISFParticle> m_isp1{};
    std::unique_ptr<ISF::ISFParticle> m_isp2{};
    std::unique_ptr<ISF::ISFParticleVector> m_pvec_children{};
    std::unique_ptr<ISF::ISFTruthIncident> m_truthIncident{};
  };

  // cppcheck-suppress syntaxError
  TEST_F(ISFTruthIncident_test, testGeoID) {
    ASSERT_EQ(test::origin.first, m_truthIncident->geoID());
  }

  TEST_F(ISFTruthIncident_test, testParentP2) {
    ASSERT_EQ(m_isp1->momentum().mag2(), m_truthIncident->parentP2());
  }

  TEST_F(ISFTruthIncident_test, testParentPdgCode) {
    ASSERT_EQ(m_isp1->pdgCode(), m_truthIncident->parentPdgCode());
  }

  TEST_F(ISFTruthIncident_test, testParentBarcode) {
    ASSERT_EQ(m_isp1->barcode(), m_truthIncident->parentBarcode());

  }

  TEST_F(ISFTruthIncident_test, testParentUniqueID) {
     ASSERT_EQ(m_isp1->id(), m_truthIncident->parentUniqueID());
  }

  TEST_F(ISFTruthIncident_test, testNumberOfChildren) {
    const unsigned int nChildren = m_truthIncident->numberOfChildren(); // returns unsigned short
    ASSERT_EQ(m_pvec_children->size(), nChildren);
  }

  TEST_F(ISFTruthIncident_test, testPhysicsProcessCategory) {
    //ISF_TruthIncident has a dummy implementation
    int dummy = -1;
    ASSERT_EQ(dummy, m_truthIncident->physicsProcessCategory());
  }

  TEST_F(ISFTruthIncident_test, testPhysicsProcessCode) {
    // the process barcode is only set through constructor;
    // => test against the value used when constructing the incident
    ASSERT_EQ(m_truthIncident->physicsProcessCode(), test::procBC);
  }

  TEST_F(ISFTruthIncident_test, testChildP2) {
    ASSERT_EQ(m_truthIncident->childP2(0), m_isp2->momentum().mag2());
  }


  TEST_F(ISFTruthIncident_test, testChildPt2) {
    ASSERT_EQ(m_truthIncident->childPt2(0), m_isp2->momentum().perp2());
  }


  TEST_F(ISFTruthIncident_test, testChildEkin) {
    ASSERT_EQ(m_truthIncident->childEkin(0), m_isp2->ekin());
  }


  TEST_F(ISFTruthIncident_test, testChildPdgCode) {
    ASSERT_EQ(m_truthIncident->childPdgCode(0), m_isp2->pdgCode());
  }


  TEST_F(ISFTruthIncident_test, testChildBarcode) {
    ASSERT_EQ(m_truthIncident->childBarcode(0), m_isp2->barcode());
    const Barcode::ParticleBarcode undefBC = Barcode::fUndefinedBarcode;
    const unsigned int childIndexOutOfRange = 1;
    ASSERT_EQ(undefBC, m_truthIncident->childBarcode(childIndexOutOfRange));
  }


  TEST_F(ISFTruthIncident_test, testChildParticle) {

    // ChildParticle(index, bc):
    // - returns HepMC::GenParticle gP with bc for child with index = index,
    // - assigns the truthincident child barcode bc

    //--------------------------------------------------------------------
    // prepare test info:
    // used to test gP child: index and bc
    const unsigned int childIndex = 0;
    const Barcode::ParticleBarcode childBarcode = 3;

    // used to test TruthIncident: no change to properties, apart from BC
    // snapshot of original child properties
    const double originalChildPt2 = m_truthIncident->childPt2(0);
    const int originalChildPdgCode = m_truthIncident->childPdgCode(0);

    //--------------------------------------------------------------------
    // get gP:
    std::unique_ptr<HepMC::GenEvent> event(HepMC::newGenEvent(1,1));
    HepMC::GenVertexPtr vertex = HepMC::newGenVertexPtr();
    event->add_vertex(vertex);
    HepMC::GenParticlePtr gPP = m_truthIncident->childParticle(childIndex,childBarcode);
    vertex->add_particle_out(gPP);
#ifdef HEPMC3
    HepMC::suggest_barcode( gPP, childBarcode);
#endif
    //--------------------------------------------------------------------
    // run tests:
    // do gP properties match original child, apart from barcode?
    ASSERT_TRUE(test::eps >= std::fabs(gPP->momentum().perp2() - originalChildPt2));
    ASSERT_EQ(gPP->pdg_id(), originalChildPdgCode);
    ASSERT_EQ(HepMC::barcode(gPP), childBarcode);

    // truthIncident: no change to properties, apart from BC?
    ASSERT_EQ(m_truthIncident->childPt2(0), originalChildPt2);
    ASSERT_EQ(m_truthIncident->childPdgCode(0), originalChildPdgCode);
    ASSERT_EQ(m_truthIncident->childBarcode(0), childBarcode);

  }

  TEST_F(ISFTruthIncident_test, testSetAllChildrenBarcodes) {

    Barcode::ParticleBarcode newBarcode = 42;
    unsigned short numSec = m_truthIncident->numberOfChildren();
    m_truthIncident->setAllChildrenBarcodes(newBarcode);
    for (unsigned short index=0; index<numSec; index++) {
      ASSERT_EQ(m_truthIncident->childBarcode(index), newBarcode);
    }

  }

} // <-- namespace MCTesting


int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest( &argc, argv );
  ::testing::AddGlobalTestEnvironment( new MCTesting::GaudiEnvironment );
  return RUN_ALL_TESTS();
}
