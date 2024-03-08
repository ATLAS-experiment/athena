/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @author John Chapman
 * @date June, 2016
 */

#undef NDEBUG

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

#include <iostream>
#include <limits>
#include <list>
// Framework
#include "TestTools/initGaudi.h"

// Google Test
#include "gtest/gtest.h"

// HepMC includes
#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/Operators.h"

// CLHEP includes
#include "CLHEP/Units/SystemOfUnits.h"

// Athena headers
#include "StoreGate/WriteHandle.h"
#include "StoreGate/StoreGateSvc.h"
#include "GeneratorObjects/McEventCollection.h"
#include "GeneratorObjects/HepMcParticleLink.h"


namespace MCTesting {

  // needed every time an AthAlgorithm, AthAlgTool or AthService is instantiated
  ISvcLocator* g_svcLoc = nullptr;

  // global test environment takes care of setting up Gaudi
  class GaudiEnvironment : public ::testing::Environment {
  protected:
    virtual void SetUp() override {
      Athena_test::initGaudi(MCTesting::g_svcLoc);
    }
  };

  class HepMcParticleLink_test : public ::testing::Test {

  protected:
    virtual void SetUp() override {
    }

    virtual void TearDown() override {
      StoreGateSvc* pStore(nullptr);
      ASSERT_TRUE(MCTesting::g_svcLoc->service("StoreGateSvc", pStore).isSuccess());
      pStore->clearStore(true).ignore(); // forceRemove=true to remove all proxies
    }
  };

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

  class is_photon {
  public:
    bool operator() ( const HepMC::ConstGenParticlePtr& p ) {
      return ( p && p->pdg_id() == 22 );
    }
  };


  HepMC::GenParticlePtr populateGenEvent(HepMC::GenEvent & ge)
  {
    HepMC::FourVector myPos( 0.0, 0.0, 0.0, 0.0);
    HepMC::GenVertexPtr myVertex = HepMC::newGenVertexPtr( myPos, -1 );
    HepMC::FourVector fourMomentum1( 0.0, 0.0, 1.0, 1.0*CLHEP::TeV);
    HepMC::GenParticlePtr inParticle1 = HepMC::newGenParticlePtr(fourMomentum1, 2, 10);
    myVertex->add_particle_in(inParticle1);
    HepMC::FourVector fourMomentum2( 0.0, 0.0, -1.0, 1.0*CLHEP::TeV);
    HepMC::GenParticlePtr inParticle2 = HepMC::newGenParticlePtr(fourMomentum2, -2, 10);
    myVertex->add_particle_in(inParticle2);
    HepMC::FourVector fourMomentum3( 0.0, 1.0, 0.0, 1.0*CLHEP::TeV);
    HepMC::GenParticlePtr inParticle3 = HepMC::newGenParticlePtr(fourMomentum3, 2, 10);
    myVertex->add_particle_out(inParticle3);
    HepMC::FourVector fourMomentum4( 0.0, -1.0, 0.0, 1.0*CLHEP::TeV);
    HepMC::GenParticlePtr inParticle4 = HepMC::newGenParticlePtr(fourMomentum4, -2, 10);
    myVertex->add_particle_out(inParticle4);
    ge.add_vertex( myVertex );
    HepMC::suggest_barcode(inParticle1,1);
    HepMC::suggest_barcode(inParticle2,2);
    HepMC::suggest_barcode(inParticle3,3);
    HepMC::suggest_barcode(inParticle4,4);
    HepMC::set_signal_process_vertex(&ge, myVertex );
    ge.set_beam_particles(inParticle1,inParticle2);
    return inParticle3;
  }

  HepMC::GenParticlePtr populateGenEvent2(HepMC::GenEvent & ge)
  {
    HepMC::FourVector myPos( 0.0, 0.0, 0.0, 0.0);
    HepMC::GenVertexPtr myVertex = HepMC::newGenVertexPtr( myPos, -1 );
    HepMC::FourVector fourMomentum1( 0.0, 0.0, 1.0, 1.0*CLHEP::TeV);
    HepMC::GenParticlePtr inParticle1 = HepMC::newGenParticlePtr(fourMomentum1, 2, 10);
    myVertex->add_particle_in(inParticle1);
    HepMC::FourVector fourMomentum2( 0.0, 0.0, -1.0, 1.0*CLHEP::TeV);
    HepMC::GenParticlePtr inParticle2 = HepMC::newGenParticlePtr(fourMomentum2, -2, 10);
    myVertex->add_particle_in(inParticle2);
    HepMC::FourVector fourMomentum3( 0.0, 1.0, 0.0, 1.0*CLHEP::TeV);
    HepMC::GenParticlePtr inParticle3 = HepMC::newGenParticlePtr(fourMomentum3, 2, 10);
    myVertex->add_particle_out(inParticle3);
    HepMC::FourVector fourMomentum4( 0.0, -1.0, 0.0, 1.0*CLHEP::TeV);
    HepMC::GenParticlePtr inParticle4 = HepMC::newGenParticlePtr(fourMomentum4, -2, 10);
    myVertex->add_particle_out(inParticle4);
    ge.add_vertex( myVertex );
    HepMC::suggest_barcode(inParticle1,1);
    HepMC::suggest_barcode(inParticle2,2);
    HepMC::suggest_barcode(inParticle3,3);
    HepMC::suggest_barcode(inParticle4,4);
    HepMC::set_signal_process_vertex(&ge, myVertex );
    ge.set_beam_particles(inParticle1,inParticle2);
    return inParticle4;
  }

  HepMC::GenParticlePtr populateFilteredGenEvent(HepMC::GenEvent & ge)
  {
    //.......Create new particle (geantino) to link  hits from pileup
    HepMC::GenParticlePtr genPart=HepMC::newGenParticlePtr();
    genPart->set_pdg_id(999); //Geantino
    genPart->set_status(1); //!< set decay status
    HepMC::suggest_barcode(genPart, std::numeric_limits<int32_t>::max() );

    HepMC::GenVertexPtr genVertex=HepMC::newGenVertexPtr ();
    genVertex->add_particle_out(genPart);

    //to set geantino vertex as a truth primary vertex
    HepMC::GenVertexPtr hScatVx = HepMC::barcode_to_vertex(&ge,-3);
    if(hScatVx!=nullptr) {
      HepMC::FourVector pmvxpos=hScatVx->position();
      genVertex->set_position(pmvxpos);
      //to set geantino kinematic phi=eta=0, E=p=E_hard_scat
#ifdef HEPMC3
      auto itrp =hScatVx->particles_in().begin();
#else
      HepMC::GenVertex::particles_in_const_iterator itrp =hScatVx->particles_in_const_begin();
#endif
      if (hScatVx->particles_in_size()==2){
        HepMC::FourVector mom1=(*itrp)->momentum();
        HepMC::FourVector mom2=(*(++itrp))->momentum();
        HepMC::FourVector vxmom;
        vxmom.setPx(mom1.e()+mom2.e());
        vxmom.setPy(0.);
        vxmom.setPz(0.);
        vxmom.setE(mom1.e()+mom2.e());

        genPart->set_momentum(vxmom);
      }
    }

#ifdef HEPMC3
    for (const auto& vtx: ge.vertices())  ge.remove_vertex(vtx);
#else
    if(!ge.vertices_empty()){
      HepMC::GenEvent::vertex_iterator itvtx = ge.vertices_begin();
      while (itvtx != ge.vertices_end()) {
        HepMC::GenVertex* vtx = *itvtx++;
        ge.remove_vertex(vtx);
        delete vtx;
      }
    }
#endif

    //.....add new vertex with geantino
    ge.add_vertex(genVertex);
    HepMC::suggest_barcode(genPart, std::numeric_limits<int32_t>::max());

    return genPart;
  }

  // cppcheck-suppress syntaxError
  TEST_F(HepMcParticleLink_test, old_test) {
    std::cout << "*** HepMcParticleLink_test starts ***" <<std::endl;

    SG::WriteHandle<McEventCollection> inputTestDataHandle{"TruthEvent"};
    inputTestDataHandle = std::make_unique<McEventCollection>();

    HepMC::GenEvent* pEvent(buildEvent());
    inputTestDataHandle->push_back(pEvent);
    inputTestDataHandle->push_back(buildEvent());  //a copy
    std::list<HepMC::GenParticlePtr> theGammas;
#ifdef HEPMC3
    std::copy_if( pEvent->particles().begin(), pEvent->particles().end(), std::back_inserter(theGammas), is_photon() );
#else
    std::copy_if( pEvent->particles_begin(), pEvent->particles_end(),
                  back_inserter(theGammas), is_photon() );
#endif
    ASSERT_EQ(1u, theGammas.size());
    HepMC::ConstGenParticlePtr pGamma(theGammas.front());
#ifdef GENP_DEBUG
    std::cout << pGamma << std::endl;
    std::cout << "gamma barcode " << HepMC::barcode(pGamma) << std::endl;
#endif

    HepMcParticleLink gammaLink1(pGamma, 0,
                                 HepMcParticleLink::IS_EVENTNUM);
    HepMcParticleLink gammaLink3(HepMC::barcode(pGamma), 0,
                                 HepMcParticleLink::IS_EVENTNUM, HepMcParticleLink::IS_BARCODE);
    HepMcParticleLink gammaLink11(pGamma, 1,
                                  HepMcParticleLink::IS_EVENTNUM);
    HepMcParticleLink gammaLink13(HepMC::barcode(pGamma), 1,
                                  HepMcParticleLink::IS_EVENTNUM, HepMcParticleLink::IS_BARCODE);

    std::stringstream out;
    out << "Testing HepMcParticleLink streamer "
        << gammaLink1 << " --- " << gammaLink11 <<std::endl;
#ifdef HEPMC3
    ASSERT_EQ(out.str(),"Testing HepMcParticleLink streamer Event index 0, Barcode 10005, McEventCollection CollectionNotSet --- Event index 1, Barcode 10005, McEventCollection CollectionNotSet\n");
#else
    ASSERT_EQ(out.str(),"Testing HepMcParticleLink streamer Event index 0, Barcode 10005, McEventCollection CollectionNotSet --- Event index 1, Barcode 10005, McEventCollection CollectionNotSet\n");
#endif

#ifdef GENP_DEBUG
    std::cout << "link barcode " << gammaLink1.barcode() << std::endl;
    std::cout << "link index " << gammaLink1.eventIndex() << std::endl;
    std::cout << "link id " << gammaLink1.id() << std::endl;
    std::cout << "link1 barcode " << gammaLink11.barcode() << std::endl;
    std::cout << "link1 index " << gammaLink11.eventIndex() << std::endl;
    std::cout << "link1 id " << gammaLink1.id() << std::endl;
#endif
    ASSERT_EQ(*gammaLink1, *gammaLink3);
    ASSERT_EQ(gammaLink1, gammaLink3);
    ASSERT_EQ(gammaLink1.barcode(), HepMC::barcode(pGamma));

    ASSERT_EQ(gammaLink1, gammaLink3);
    ASSERT_EQ(gammaLink11, gammaLink13);

    ASSERT_EQ(*gammaLink1, *gammaLink11);
    ASSERT_NE( gammaLink1, gammaLink11 ); //FIXME weird! Can't check ptr...
    ASSERT_LT( gammaLink1, gammaLink11 );  //FIXME weird! Can't check ptr...
    std::cout << "*** HepMcParticleLink_test OK ***" <<std::endl;
  }

  TEST_F(HepMcParticleLink_test, broken_event_link) {
    // create dummy input McEventCollection with a name that
    // HepMcParticleLink does not know about
    SG::WriteHandle<McEventCollection> inputTestDataHandle{"MISNAMED_EVENT"};
    inputTestDataHandle = std::make_unique<McEventCollection>();
    // Fill it with a dummy GenEvent
    inputTestDataHandle->push_back(HepMC::newGenEvent(20,1));
    HepMC::GenEvent& ge1 = *(inputTestDataHandle->at(0));
    HepMC::ConstGenParticlePtr particle1 = populateGenEvent(ge1);
    // A HepMcParticleLink built using a GenParticle pointer should
    // still work.
    HepMcParticleLink testLink1a(particle1,0,
                                 HepMcParticleLink::IS_EVENTNUM);
    ASSERT_TRUE( testLink1a.isValid() );
    // A HepMcParticleLink built using the barcode and the position of
    // the GenEvent in the McEventCollection will not work.
    HepMcParticleLink testLink1b(HepMC::barcode(particle1),0,
                                 HepMcParticleLink::IS_EVENTNUM, HepMcParticleLink::IS_BARCODE);
    ASSERT_FALSE( testLink1b.isValid() );
    // A HepMcParticleLink built using a GenParticle pointer and the position of
    // the GenEvent in the McEventCollection should still work.
    HepMcParticleLink testLink1c(particle1,0,HepMcParticleLink::IS_POSITION);
    ASSERT_TRUE( testLink1c.isValid() );
  }

  TEST_F(HepMcParticleLink_test, truth_event_link_first_event) {
    // create dummy input McEventCollection with a name that
    // HepMcParticleLink knows about
    SG::WriteHandle<McEventCollection> inputTestDataHandle{"TruthEvent"};
    inputTestDataHandle = std::make_unique<McEventCollection>();

    // Add a dummy GenEvent
    const int process_id1(20);
    const int event_number1(17);
    inputTestDataHandle->push_back(HepMC::newGenEvent(process_id1, event_number1));
    const HepMcParticleLink::index_type dummyIndex1(0);
    const HepMcParticleLink::index_type refEvtNum1 = static_cast<HepMcParticleLink::index_type>(event_number1);
    HepMC::GenEvent& ge1 = *(inputTestDataHandle->at(0));
    HepMC::ConstGenParticlePtr particle1 = populateGenEvent(ge1);
    // Add a second dummy GenEvent
    const int process_id2(20);
    const int event_number2(25);
    inputTestDataHandle->push_back(HepMC::newGenEvent(process_id2, event_number2));
    HepMC::GenEvent& ge2 = *(inputTestDataHandle->at(1));
    (void)populateGenEvent2(ge2);
    // Add a third dummy GenEvent (identical to the first)
    const int process_id3(20);
    const int event_number3(17);
    inputTestDataHandle->push_back(HepMC::newGenEvent(process_id3, event_number3));
    HepMC::GenEvent& ge3 = *(inputTestDataHandle->at(2));
    (void)populateGenEvent(ge3);

    const int event_number4(89);
    inputTestDataHandle->push_back(new HepMC::GenEvent(ge1));
    HepMC::GenEvent& ge4 = *(inputTestDataHandle->at(3));
    ge4.set_event_number(event_number4);
    (void)populateFilteredGenEvent(ge4);

    const IProxyDict* sg = SG::CurrentEventStore::store();

    //Testing links to the first dummy GenEvent

    // HepMcParticleLink built using a GenParticle pointer and the
    // position of the GenEvent.
    HepMcParticleLink testLink1a(particle1,dummyIndex1,
                                 HepMcParticleLink::IS_POSITION);
    ASSERT_TRUE( testLink1a.isValid() );
    ASSERT_EQ(HepMC::barcode(particle1), testLink1a.barcode());
    ASSERT_EQ( refEvtNum1, testLink1a.eventIndex());
    ASSERT_EQ( dummyIndex1, testLink1a.getEventPositionInCollection(sg));
    ASSERT_EQ(particle1,testLink1a.cptr());
    // A HepMcParticleLink built using the barcode and the position of
    // the GenEvent.
    HepMcParticleLink testLink1b(HepMC::barcode(particle1),dummyIndex1,
                                 HepMcParticleLink::IS_POSITION, HepMcParticleLink::IS_BARCODE);
    ASSERT_TRUE( testLink1b.isValid() );
    ASSERT_EQ( HepMC::barcode(particle1), testLink1b.barcode());
    ASSERT_EQ( refEvtNum1, testLink1b.eventIndex());
    ASSERT_EQ( dummyIndex1, testLink1b.getEventPositionInCollection(sg));
    ASSERT_EQ(*(testLink1a.cptr()),*(testLink1b.cptr()));
    // HepMcParticleLink built using a GenParticle pointer and the
    // event_number of the GenEvent.
    HepMcParticleLink testLink1d(particle1,event_number1);
    ASSERT_TRUE( testLink1d.isValid() );
    ASSERT_EQ( HepMC::barcode(particle1), testLink1d.barcode());
    ASSERT_NE( dummyIndex1, testLink1d.eventIndex());
    ASSERT_EQ( refEvtNum1, testLink1d.eventIndex());
    ASSERT_EQ( dummyIndex1, testLink1d.getEventPositionInCollection(sg));
    ASSERT_EQ(particle1,testLink1d.cptr());
    // A HepMcParticleLink built using the barcode and the event_number of
    // the GenEvent.
    HepMcParticleLink testLink1e(HepMC::barcode(particle1),event_number1,
                                 HepMcParticleLink::IS_EVENTNUM, HepMcParticleLink::IS_BARCODE);
    ASSERT_TRUE( testLink1e.isValid() );
    ASSERT_EQ( HepMC::barcode(particle1), testLink1e.barcode());
    ASSERT_NE( dummyIndex1, testLink1e.eventIndex());
    ASSERT_EQ( refEvtNum1, testLink1e.eventIndex());
    ASSERT_EQ( dummyIndex1, testLink1e.getEventPositionInCollection(sg));
    ASSERT_EQ(particle1,testLink1e.cptr());
  }

  TEST_F(HepMcParticleLink_test, truth_event_link_second_event) {
    // create dummy input McEventCollection with a name that
    // HepMcParticleLink knows about
    SG::WriteHandle<McEventCollection> inputTestDataHandle{"TruthEvent"};
    inputTestDataHandle = std::make_unique<McEventCollection>();

    // Add a dummy GenEvent
    const int process_id1(20);
    const int event_number1(17);
    inputTestDataHandle->push_back(HepMC::newGenEvent(process_id1, event_number1));
    HepMC::GenEvent& ge1 = *(inputTestDataHandle->at(0));
    (void)populateGenEvent(ge1);
    // Add a second dummy GenEvent
    const int process_id2(20);
    const int event_number2(25);
    inputTestDataHandle->push_back(HepMC::newGenEvent(process_id2, event_number2));
    const HepMcParticleLink::index_type dummyIndex2(1);
    const HepMcParticleLink::index_type refEvtNum2 = static_cast<HepMcParticleLink::index_type>(event_number2);
    HepMC::GenEvent& ge2 = *(inputTestDataHandle->at(1));
    HepMC::ConstGenParticlePtr particle2 = populateGenEvent2(ge2);
    // Add a third dummy GenEvent (identical to the first)
    const int process_id3(20);
    const int event_number3(17);
    inputTestDataHandle->push_back(HepMC::newGenEvent(process_id3, event_number3));
    HepMC::GenEvent& ge3 = *(inputTestDataHandle->at(2));
    (void)populateGenEvent(ge3);

    const int event_number4(89);
    inputTestDataHandle->push_back(new HepMC::GenEvent(ge1));
    HepMC::GenEvent& ge4 = *(inputTestDataHandle->at(3));
    ge4.set_event_number(event_number4);
    (void)populateFilteredGenEvent(ge4);

    const IProxyDict* sg = SG::CurrentEventStore::store();

    //Testing links to the second dummy GenEvent

    // HepMcParticleLink built using a GenParticle pointer and the
    // position of the GenEvent.
    HepMcParticleLink testLink2a(particle2, dummyIndex2,
                                 HepMcParticleLink::IS_POSITION);
    ASSERT_TRUE( testLink2a.isValid() );
    ASSERT_EQ( HepMC::barcode(particle2), testLink2a.barcode());
    ASSERT_EQ( refEvtNum2, testLink2a.eventIndex());
    ASSERT_EQ( dummyIndex2, testLink2a.getEventPositionInCollection(sg));
    ASSERT_EQ(particle2,testLink2a.cptr());
    // A HepMcParticleLink built using the barcode and the position of
    // the GenEvent.
    HepMcParticleLink testLink2b(HepMC::barcode(particle2), dummyIndex2,
                                 HepMcParticleLink::IS_POSITION, HepMcParticleLink::IS_BARCODE);
    ASSERT_TRUE( testLink2b.isValid() );
    ASSERT_EQ( HepMC::barcode(particle2), testLink2b.barcode());
    ASSERT_EQ( refEvtNum2, testLink2b.eventIndex());
    ASSERT_EQ( dummyIndex2, testLink2b.getEventPositionInCollection(sg));
    ASSERT_EQ(particle2,testLink2b.cptr());
    // HepMcParticleLink built using a GenParticle pointer and the
    // event_number of the GenEvent.
    HepMcParticleLink testLink2d(particle2,event_number2);
    ASSERT_TRUE( testLink2d.isValid() );
    ASSERT_EQ( HepMC::barcode(particle2), testLink2d.barcode());
    ASSERT_NE( dummyIndex2, testLink2d.eventIndex());
    ASSERT_EQ( refEvtNum2, testLink2d.eventIndex());
    ASSERT_EQ( dummyIndex2, testLink2d.getEventPositionInCollection(sg));
    ASSERT_EQ(particle2,testLink2d.cptr());
    // A HepMcParticleLink built using the barcode and the event_number of
    // the GenEvent.
    HepMcParticleLink testLink2e(HepMC::barcode(particle2),event_number2,
                                 HepMcParticleLink::IS_EVENTNUM, HepMcParticleLink::IS_BARCODE);
    ASSERT_TRUE( testLink2e.isValid() );
    ASSERT_EQ( HepMC::barcode(particle2), testLink2e.barcode());
    ASSERT_NE( dummyIndex2, testLink2e.eventIndex());
    ASSERT_EQ( refEvtNum2, testLink2e.eventIndex());
    ASSERT_EQ( dummyIndex2, testLink2e.getEventPositionInCollection(sg));
    ASSERT_EQ(particle2,testLink2e.cptr());
  }

  TEST_F(HepMcParticleLink_test, truth_event_link_third_event) {
    // create dummy input McEventCollection with a name that
    // HepMcParticleLink knows about
    SG::WriteHandle<McEventCollection> inputTestDataHandle{"TruthEvent"};
    inputTestDataHandle = std::make_unique<McEventCollection>();

    // Add a dummy GenEvent
    const int process_id1(20);
    const int event_number1(17);
    inputTestDataHandle->push_back(HepMC::newGenEvent(process_id1, event_number1));
    const HepMcParticleLink::index_type dummyIndex1(0);
    HepMC::GenEvent& ge1 = *(inputTestDataHandle->at(0));
    HepMC::ConstGenParticlePtr particle1 = populateGenEvent(ge1);
    // Add a second dummy GenEvent
    const int process_id2(20);
    const int event_number2(25);
    inputTestDataHandle->push_back(HepMC::newGenEvent(process_id2, event_number2));
    HepMC::GenEvent& ge2 = *(inputTestDataHandle->at(1));
    (void)populateGenEvent2(ge2);
    // Add a third dummy GenEvent (identical to the first)
    const int process_id3(20);
    const int event_number3(17);
    inputTestDataHandle->push_back(HepMC::newGenEvent(process_id3, event_number3));
    const HepMcParticleLink::index_type dummyIndex3(2);
    const HepMcParticleLink::index_type refEvtNum3 = static_cast<HepMcParticleLink::index_type>(event_number3);
    HepMC::GenEvent& ge3 = *(inputTestDataHandle->at(2));
    HepMC::ConstGenParticlePtr particle3 = populateGenEvent(ge3);

    const int event_number4(89);
    inputTestDataHandle->push_back(new HepMC::GenEvent(ge1));
    HepMC::GenEvent& ge4 = *(inputTestDataHandle->at(3));
    ge4.set_event_number(event_number4);
    (void)populateFilteredGenEvent(ge4);

    const IProxyDict* sg = SG::CurrentEventStore::store();

    //Testing links to the third dummy GenEvent

    // HepMcParticleLink built using a GenParticle pointer and the
    // position of the GenEvent.
    HepMcParticleLink testLink3a(particle3, dummyIndex3,
                                 HepMcParticleLink::IS_POSITION);
    ASSERT_TRUE( testLink3a.isValid() );
    ASSERT_EQ( HepMC::barcode(particle3), testLink3a.barcode());
    ASSERT_EQ( refEvtNum3, testLink3a.eventIndex());
    ASSERT_NE( dummyIndex3, testLink3a.getEventPositionInCollection(sg)); // Returns the position of the first event with a given event number
    ASSERT_EQ( dummyIndex1, testLink3a.getEventPositionInCollection(sg));
    ASSERT_EQ(particle3,testLink3a.cptr());
    // A HepMcParticleLink built using the barcode and the position of
    // the GenEvent.
    HepMcParticleLink testLink3b(HepMC::barcode(particle3), dummyIndex3,
                                 HepMcParticleLink::IS_POSITION, HepMcParticleLink::IS_BARCODE);
    ASSERT_TRUE( testLink3b.isValid() );
    ASSERT_EQ( HepMC::barcode(particle3), testLink3b.barcode());
    ASSERT_EQ( refEvtNum3, testLink3b.eventIndex());
    ASSERT_NE( dummyIndex3, testLink3b.getEventPositionInCollection(sg)); // Returns the position of the first event with a given event number
    ASSERT_EQ( dummyIndex1, testLink3a.getEventPositionInCollection(sg));
    ASSERT_EQ(particle3,testLink3b.cptr());
    // HepMcParticleLink built using a GenParticle pointer and the
    // event_number of the GenEvent.
    HepMcParticleLink testLink3d(particle3,event_number3);
    ASSERT_TRUE( testLink3d.isValid() );
    ASSERT_EQ( HepMC::barcode(particle3), testLink3d.barcode());
    ASSERT_NE( dummyIndex3, testLink3d.eventIndex());
    ASSERT_EQ( refEvtNum3, testLink3d.eventIndex());
    ASSERT_NE( dummyIndex3, testLink3d.getEventPositionInCollection(sg)); // Returns the position of the first event with a given event number
    ASSERT_EQ( dummyIndex1, testLink3a.getEventPositionInCollection(sg));
    ASSERT_EQ(particle3,testLink3d.cptr());
    // A HepMcParticleLink built using the barcode and the event_number of
    // the GenEvent. BROKEN
    HepMcParticleLink testLink3e(HepMC::barcode(particle3),event_number3,
                                 HepMcParticleLink::IS_EVENTNUM, HepMcParticleLink::IS_BARCODE);
    ASSERT_TRUE( testLink3e.isValid() );
    ASSERT_EQ( HepMC::barcode(particle3), testLink3e.barcode());
    ASSERT_NE( dummyIndex3, testLink3e.eventIndex());
    ASSERT_EQ( refEvtNum3, testLink3e.eventIndex());
    ASSERT_NE( dummyIndex3, testLink3e.getEventPositionInCollection(sg)); // Returns the position of the first event with a given event number
    ASSERT_EQ( dummyIndex1, testLink3a.getEventPositionInCollection(sg));
    ASSERT_NE(particle3,testLink3e.cptr());
    ASSERT_EQ(particle1,testLink3e.cptr()); //POINTS AT THE IDENTICAL PARTICLE IN FIRST GenEvent!!
  }

  TEST_F(HepMcParticleLink_test, truth_event_link_fourth_event) {
    // create dummy input McEventCollection with a name that
    // HepMcParticleLink knows about
    SG::WriteHandle<McEventCollection> inputTestDataHandle{"TruthEvent"};
    inputTestDataHandle = std::make_unique<McEventCollection>();

    // Add a dummy GenEvent
    const int process_id1(20);
    const int event_number1(17);
    inputTestDataHandle->push_back(HepMC::newGenEvent(process_id1, event_number1));
    HepMC::GenEvent& ge1 = *(inputTestDataHandle->at(0));
    (void)populateGenEvent(ge1);
    // Add a second dummy GenEvent
    const int process_id2(20);
    const int event_number2(25);
    inputTestDataHandle->push_back(HepMC::newGenEvent(process_id2, event_number2));
    HepMC::GenEvent& ge2 = *(inputTestDataHandle->at(1));
    (void)populateGenEvent2(ge2);
    // Add a third dummy GenEvent (identical to the first)
    const int process_id3(20);
    const int event_number3(17);
    inputTestDataHandle->push_back(HepMC::newGenEvent(process_id3, event_number3));
    HepMC::GenEvent& ge3 = *(inputTestDataHandle->at(2));
    (void)populateGenEvent(ge3);

    const int event_number4(89);
    inputTestDataHandle->push_back(new HepMC::GenEvent(ge1));
    const HepMcParticleLink::index_type dummyIndex4(3);
    const HepMcParticleLink::index_type refEvtNum4 = static_cast<HepMcParticleLink::index_type>(event_number4);
    HepMC::GenEvent& ge4 = *(inputTestDataHandle->at(3));
    ge4.set_event_number(event_number4);
    HepMC::ConstGenParticlePtr particle4 = populateFilteredGenEvent(ge4);

    const IProxyDict* sg = SG::CurrentEventStore::store();

    // Testing links to the fourth GenEvent - filtered pile-up truth

    // HepMcParticleLink built using a GenParticle pointer and the
    // event_number of the GenEvent.
    HepMcParticleLink testLink4a(particle4,event_number4);
    ASSERT_TRUE( testLink4a.isValid() );
    ASSERT_EQ( HepMC::barcode(particle4), testLink4a.barcode());
    ASSERT_EQ( refEvtNum4, testLink4a.eventIndex());
    ASSERT_EQ( dummyIndex4, testLink4a.getEventPositionInCollection(sg));
    ASSERT_EQ( particle4, testLink4a.cptr());
    // A HepMcParticleLink built using the barcode and the position of
    // the GenEvent.
    HepMcParticleLink testLink4c(HepMC::barcode(particle4),event_number4,
                                 HepMcParticleLink::IS_EVENTNUM, HepMcParticleLink::IS_BARCODE);
    ASSERT_TRUE( testLink4c.isValid() );
    ASSERT_EQ( HepMC::barcode(particle4), testLink4c.barcode());
    ASSERT_EQ( refEvtNum4, testLink4c.eventIndex());
    ASSERT_EQ( dummyIndex4, testLink4c.getEventPositionInCollection(sg));
    ASSERT_EQ( particle4, testLink4c.cptr());
    // HepMcParticleLink built using a GenParticle pointer and the
    // position of the GenEvent.
    HepMcParticleLink testLink4d(particle4, dummyIndex4,
                                 HepMcParticleLink::IS_POSITION);
    ASSERT_TRUE( testLink4d.isValid() );
    ASSERT_EQ( HepMC::barcode(particle4), testLink4d.barcode());
    ASSERT_EQ( refEvtNum4, testLink4d.eventIndex());
    ASSERT_EQ( dummyIndex4, testLink4d.getEventPositionInCollection(sg));
    ASSERT_EQ( particle4, testLink4d.cptr());
    // A HepMcParticleLink built using the barcode and the position of
    // the GenEvent.
    HepMcParticleLink testLink4f(HepMC::barcode(particle4), dummyIndex4,
                                 HepMcParticleLink::IS_POSITION, HepMcParticleLink::IS_BARCODE);
    ASSERT_TRUE( testLink4f.isValid() );
    ASSERT_EQ( HepMC::barcode(particle4), testLink4f.barcode());
    ASSERT_EQ( refEvtNum4, testLink4f.eventIndex());
    ASSERT_EQ( dummyIndex4, testLink4f.getEventPositionInCollection(sg));
    ASSERT_EQ( particle4, testLink4f.cptr());
  }

  TEST_F(HepMcParticleLink_test, truth_event_link_delta_rays) {
    // create dummy input McEventCollection with a name that
    // HepMcParticleLink knows about
    SG::WriteHandle<McEventCollection> inputTestDataHandle{"TruthEvent"};
    inputTestDataHandle = std::make_unique<McEventCollection>();

    // Add a dummy GenEvent
    const int process_id1(20);
    const int event_number1(17);
    inputTestDataHandle->push_back(HepMC::newGenEvent(process_id1, event_number1));
    const HepMcParticleLink::index_type dummyIndex1(0);
    const HepMcParticleLink::index_type refEvtNum1 = static_cast<HepMcParticleLink::index_type>(event_number1);
    HepMC::GenEvent& ge1 = *(inputTestDataHandle->at(0));
    (void)populateGenEvent(ge1);
    // Add a second dummy GenEvent
    const int process_id2(20);
    const int event_number2(25);
    inputTestDataHandle->push_back(HepMC::newGenEvent(process_id2, event_number2));
    const HepMcParticleLink::index_type dummyIndex2(1);
    const HepMcParticleLink::index_type refEvtNum2 = static_cast<HepMcParticleLink::index_type>(event_number2);
    HepMC::GenEvent& ge2 = *(inputTestDataHandle->at(1));
    (void)populateGenEvent2(ge2);
    // Add a third dummy GenEvent (identical to the first)
    const int process_id3(20);
    const int event_number3(17);
    inputTestDataHandle->push_back(HepMC::newGenEvent(process_id3, event_number3));
    HepMC::GenEvent& ge3 = *(inputTestDataHandle->at(2));
    (void)populateGenEvent(ge3);

    const int event_number4(89);
    inputTestDataHandle->push_back(new HepMC::GenEvent(ge1));
    HepMC::GenEvent& ge4 = *(inputTestDataHandle->at(3));
    ge4.set_event_number(event_number4);
    (void)populateFilteredGenEvent(ge4);

    const IProxyDict* sg = SG::CurrentEventStore::store();

    // Testing Links to delta-rays (barcode=0) - not recorded in McEventCollection
    const int deltaRayBarcode(0);
    HepMC::GenParticlePtr deltaRayPtr{};

    // HepMcParticleLink built using a delta-ray barcode and the event_number of
    // the GenEvent.
    HepMcParticleLink testLink5a(deltaRayBarcode, refEvtNum1,
                                 HepMcParticleLink::IS_EVENTNUM, HepMcParticleLink::IS_BARCODE);
    ASSERT_EQ( refEvtNum1, testLink5a.eventIndex());
    ASSERT_FALSE( testLink5a.isValid() );
    ASSERT_EQ( deltaRayBarcode, testLink5a.barcode());
    ASSERT_EQ( dummyIndex1, testLink5a.getEventPositionInCollection(sg));
    ASSERT_EQ( deltaRayPtr, testLink5a.cptr());
   // HepMcParticleLink built using a delta-ray barcode and the
    // position of the GenEvent.
    HepMcParticleLink testLink5b(deltaRayBarcode, dummyIndex1,
                                 HepMcParticleLink::IS_POSITION, HepMcParticleLink::IS_BARCODE);
    ASSERT_FALSE( testLink5b.isValid() );
    ASSERT_EQ( deltaRayBarcode, testLink5b.barcode());
    ASSERT_EQ( refEvtNum1, testLink5b.eventIndex());
    ASSERT_EQ( dummyIndex1, testLink5b.getEventPositionInCollection(sg));
    ASSERT_EQ( deltaRayPtr, testLink5b.cptr());

    // HepMcParticleLink built using a delta-ray barcode and the
    // position of the GenEvent (2nd event).
    HepMcParticleLink testLink5e(deltaRayBarcode, dummyIndex2,
                                 HepMcParticleLink::IS_POSITION, HepMcParticleLink::IS_BARCODE);
    ASSERT_FALSE( testLink5e.isValid() );
    ASSERT_EQ( deltaRayBarcode, testLink5e.barcode());
    ASSERT_EQ( refEvtNum2, testLink5e.eventIndex());
    ASSERT_EQ( dummyIndex2, testLink5e.getEventPositionInCollection(sg));
    ASSERT_EQ( deltaRayPtr, testLink5e.cptr());

    // HepMcParticleLink built using a delta-ray barcode and the event_number of
    // the GenEvent. GenEvent not recorded
    const int event_number5(460);
    const HepMcParticleLink::index_type refEvtNum5 = static_cast<HepMcParticleLink::index_type>(event_number5);
    HepMcParticleLink testLink5f(deltaRayBarcode, refEvtNum5,
                                 HepMcParticleLink::IS_EVENTNUM, HepMcParticleLink::IS_BARCODE);
    ASSERT_FALSE( testLink5f.isValid() );
    ASSERT_EQ( deltaRayBarcode, testLink5f.barcode());
    ASSERT_EQ( refEvtNum5, testLink5f.eventIndex());
    ASSERT_EQ( HepMcParticleLink:: ExtendedBarCode::UNDEFINED, testLink5f.getEventPositionInCollection(sg));
    ASSERT_EQ( deltaRayPtr, testLink5f.cptr());

   }

  TEST_F(HepMcParticleLink_test, truth_event_link_cut_events) {
    // create dummy input McEventCollection with a name that
    // HepMcParticleLink knows about
    SG::WriteHandle<McEventCollection> inputTestDataHandle{"TruthEvent"};
    inputTestDataHandle = std::make_unique<McEventCollection>();

    // Add a dummy GenEvent
    const int process_id1(20);
    const int event_number1(17);
    inputTestDataHandle->push_back(HepMC::newGenEvent(process_id1, event_number1));
    const HepMcParticleLink::index_type dummyIndex1(0);
    const HepMcParticleLink::index_type refEvtNum1 = static_cast<HepMcParticleLink::index_type>(event_number1);
    HepMC::GenEvent& ge1 = *(inputTestDataHandle->at(0));
    (void)populateGenEvent(ge1);
    // Add a second dummy GenEvent
    const int process_id2(20);
    const int event_number2(25);
    inputTestDataHandle->push_back(HepMC::newGenEvent(process_id2, event_number2));
    HepMC::GenEvent& ge2 = *(inputTestDataHandle->at(1));
    (void)populateGenEvent2(ge2);
    // Add a third dummy GenEvent (identical to the first)
    const int process_id3(20);
    const int event_number3(17);
    inputTestDataHandle->push_back(HepMC::newGenEvent(process_id3, event_number3));
    HepMC::GenEvent& ge3 = *(inputTestDataHandle->at(2));
    (void)populateGenEvent(ge3);

    const int event_number4(89);
    inputTestDataHandle->push_back(new HepMC::GenEvent(ge1));
    HepMC::GenEvent& ge4 = *(inputTestDataHandle->at(3));
    ge4.set_event_number(event_number4);
    (void)populateFilteredGenEvent(ge4);

    //Testing links to the first dummy GenEvent
    const IProxyDict* sg = SG::CurrentEventStore::store();

    // Links to GenParticles which were not copied to the current McEventCollection

    // Info about a GenParticle/GenEvent which is not part of the
    // McEventCollection. For example if pile-up truth was not
    // recorded.
    const int event_number5(460);
    const HepMcParticleLink::index_type refEvtNum5 = static_cast<HepMcParticleLink::index_type>(event_number5);
    const int cutBarcode(210001);
    HepMC::ConstGenParticlePtr cutParticlePtr{};

    // Link to a GenParticle which was not recorded to the
    // McEventCollection, even though other parts of the same GenEvent
    // were recorded.
    HepMcParticleLink testLink6a(cutBarcode, refEvtNum1, HepMcParticleLink::IS_EVENTNUM, HepMcParticleLink::IS_BARCODE);
    ASSERT_FALSE( testLink6a.isValid() );
    ASSERT_EQ( cutBarcode, testLink6a.barcode());
    ASSERT_EQ( refEvtNum1, testLink6a.eventIndex());
    ASSERT_EQ( dummyIndex1, testLink6a.getEventPositionInCollection(sg));
    ASSERT_EQ( cutParticlePtr, testLink6a.cptr());

    // Link to a GenEvent which was not recorded to the McEventCollection
    HepMcParticleLink testLink6b(cutBarcode, refEvtNum5, HepMcParticleLink::IS_EVENTNUM, HepMcParticleLink::IS_BARCODE);
    ASSERT_FALSE( testLink6b.isValid() );
    ASSERT_EQ( cutBarcode, testLink6b.barcode());
    ASSERT_EQ( refEvtNum5, testLink6b.eventIndex());
    ASSERT_EQ( HepMcParticleLink:: ExtendedBarCode::UNDEFINED, testLink6b.getEventPositionInCollection(sg));
    ASSERT_EQ( cutParticlePtr, testLink6b.cptr());
  }

  TEST_F(HepMcParticleLink_test, less_than_operator) {
    // create dummy input McEventCollection with a name that
    // HepMcParticleLink knows about
    SG::WriteHandle<McEventCollection> inputTestDataHandle{"TruthEvent"};
    inputTestDataHandle = std::make_unique<McEventCollection>();

    // Add a dummy GenEvent
    const int process_id1(20);
    const int event_number1(17);
    inputTestDataHandle->push_back(HepMC::newGenEvent(process_id1, event_number1));
    const HepMcParticleLink::index_type dummyIndex1(0);
    HepMC::GenEvent& ge1 = *(inputTestDataHandle->at(0));
    HepMC::ConstGenParticlePtr particle1 = populateGenEvent(ge1);
    // Add a second dummy GenEvent
    const int process_id2(20);
    const int event_number2(25);
    inputTestDataHandle->push_back(HepMC::newGenEvent(process_id2, event_number2));
    const HepMcParticleLink::index_type dummyIndex2(1);
    HepMC::GenEvent& ge2 = *(inputTestDataHandle->at(1));
    HepMC::ConstGenParticlePtr particle2 = populateGenEvent2(ge2);
    // Add a third dummy GenEvent (identical to the first)
    const int process_id3(20);
    const int event_number3(17);
    inputTestDataHandle->push_back(HepMC::newGenEvent(process_id3, event_number3));
    HepMC::GenEvent& ge3 = *(inputTestDataHandle->at(2));
    (void)populateGenEvent(ge3);

    const int event_number4(89);
    inputTestDataHandle->push_back(new HepMC::GenEvent(ge1));
    HepMC::GenEvent& ge4 = *(inputTestDataHandle->at(3));
    ge4.set_event_number(event_number4);
    (void)populateFilteredGenEvent(ge4);

    //Testing links to the first dummy GenEvent

    // HepMcParticleLink built using a GenParticle pointer and the
    // position of the GenEvent.
    HepMcParticleLink testLink1a(particle1,dummyIndex1,
                                 HepMcParticleLink::IS_POSITION);
    ASSERT_TRUE( testLink1a.isValid() );
    // A HepMcParticleLink built using the barcode and the position of
    // the GenEvent.
    HepMcParticleLink testLink1b(HepMC::barcode(particle1),dummyIndex1,
                                 HepMcParticleLink::IS_POSITION, HepMcParticleLink::IS_BARCODE);
    ASSERT_TRUE( testLink1b.isValid() );
    // HepMcParticleLink built using a GenParticle pointer and the
    // event_number of the GenEvent.
    HepMcParticleLink testLink1d(particle1,event_number1);
    ASSERT_TRUE( testLink1d.isValid() );
    // A HepMcParticleLink built using the barcode and the event_number of
    // the GenEvent.
    HepMcParticleLink testLink1e(HepMC::barcode(particle1),event_number1,
                                 HepMcParticleLink::IS_EVENTNUM, HepMcParticleLink::IS_BARCODE);
    ASSERT_TRUE( testLink1e.isValid() );
    // Comparing links to the same particle in the first event
    ASSERT_FALSE( testLink1a < testLink1b );
    ASSERT_FALSE( testLink1a < testLink1d );
    ASSERT_FALSE( testLink1a < testLink1e );
    ASSERT_FALSE( testLink1b < testLink1a );
    ASSERT_FALSE( testLink1b < testLink1d );
    ASSERT_FALSE( testLink1b < testLink1e );
    ASSERT_FALSE( testLink1d < testLink1a );
    ASSERT_FALSE( testLink1d < testLink1b );
    ASSERT_FALSE( testLink1d < testLink1e );
    ASSERT_FALSE( testLink1e < testLink1a );
    ASSERT_FALSE( testLink1e < testLink1b );
    ASSERT_FALSE( testLink1e < testLink1d );
   // HepMcParticleLink built using a GenParticle pointer and the
    // position of the GenEvent.
    HepMcParticleLink testLink2a(particle2, dummyIndex2,
                                 HepMcParticleLink::IS_POSITION);
    ASSERT_TRUE( testLink2a.isValid() );
    // A HepMcParticleLink built using the barcode and the position of
    // the GenEvent.
    HepMcParticleLink testLink2b(HepMC::barcode(particle2), dummyIndex2,
                                 HepMcParticleLink::IS_POSITION, HepMcParticleLink::IS_BARCODE);
    ASSERT_TRUE( testLink2b.isValid() );
    // HepMcParticleLink built using a GenParticle pointer and the
    // event_number of the GenEvent.
    HepMcParticleLink testLink2d(particle2,event_number2);
    ASSERT_TRUE( testLink2d.isValid() );
    // A HepMcParticleLink built using the barcode and the event_number of
    // the GenEvent.
    HepMcParticleLink testLink2e(HepMC::barcode(particle2),event_number2,
                                 HepMcParticleLink::IS_EVENTNUM, HepMcParticleLink::IS_BARCODE);
    ASSERT_TRUE( testLink2e.isValid() );
    // Comparing links to the same particle in the second event
    ASSERT_FALSE( testLink2a < testLink2b );
    ASSERT_FALSE( testLink2a < testLink2d );
    ASSERT_FALSE( testLink2a < testLink2e );
    ASSERT_FALSE( testLink2b < testLink2a );
    ASSERT_FALSE( testLink2b < testLink2d );
    ASSERT_FALSE( testLink2b < testLink2e );
    ASSERT_FALSE( testLink2d < testLink2a );
    ASSERT_FALSE( testLink2d < testLink2b );
    ASSERT_FALSE( testLink2d < testLink2e );
    ASSERT_FALSE( testLink2e < testLink2a );
    ASSERT_FALSE( testLink2e < testLink2b );
    ASSERT_FALSE( testLink2e < testLink2d );
    // Comparing links in two different events
    ASSERT_TRUE( testLink1a < testLink2a );
    ASSERT_TRUE( testLink1a < testLink2b );
    ASSERT_TRUE( testLink1a < testLink2d );
    ASSERT_TRUE( testLink1a < testLink2e );
    ASSERT_TRUE( testLink1b < testLink2a );
    ASSERT_TRUE( testLink1b < testLink2b );
    ASSERT_TRUE( testLink1b < testLink2d );
    ASSERT_TRUE( testLink1b < testLink2e );
    ASSERT_TRUE( testLink1d < testLink2a );
    ASSERT_TRUE( testLink1d < testLink2b );
    ASSERT_TRUE( testLink1d < testLink2d );
    ASSERT_TRUE( testLink1d < testLink2e );
    ASSERT_TRUE( testLink1e < testLink2a );
    ASSERT_TRUE( testLink1e < testLink2b );
    ASSERT_TRUE( testLink1e < testLink2d );
    ASSERT_TRUE( testLink1e < testLink2e );

    // Make links to another GenParticle from the first GenEvent
#ifdef HEPMC3
    HepMC::ConstGenParticlePtr particle3 = ge1.particles().at(3);
 #else
    HepMC::ConstGenParticlePtr particle3 = HepMC::barcode_to_particle(&ge1, 4);
#endif
    // HepMcParticleLink built using a GenParticle pointer and the
    // position of the GenEvent.
    HepMcParticleLink testLink3a(particle3,dummyIndex1,
                                 HepMcParticleLink::IS_POSITION);
    ASSERT_TRUE( testLink3a.isValid() );
    // A HepMcParticleLink built using the barcode and the position of
    // the GenEvent.
    HepMcParticleLink testLink3b(HepMC::barcode(particle3),dummyIndex1,
                                 HepMcParticleLink::IS_POSITION, HepMcParticleLink::IS_BARCODE);
    ASSERT_TRUE( testLink3b.isValid() );
    // HepMcParticleLink built using a GenParticle pointer and the
    // event_number of the GenEvent.
    HepMcParticleLink testLink3d(particle3,event_number1);
    ASSERT_TRUE( testLink3d.isValid() );
    // A HepMcParticleLink built using the barcode and the event_number of
    // the GenEvent.
    HepMcParticleLink testLink3e(HepMC::barcode(particle3),event_number1,
                                 HepMcParticleLink::IS_EVENTNUM, HepMcParticleLink::IS_BARCODE);
    ASSERT_TRUE( testLink3e.isValid() );
    // Comparing links to two different particles in the same GenEvent
    ASSERT_TRUE( testLink1a < testLink3a );
    ASSERT_TRUE( testLink1a < testLink3b );
    ASSERT_TRUE( testLink1a < testLink3d );
    ASSERT_TRUE( testLink1a < testLink3e );
    ASSERT_TRUE( testLink1b < testLink3a );
    ASSERT_TRUE( testLink1b < testLink3b );
    ASSERT_TRUE( testLink1b < testLink3d );
    ASSERT_TRUE( testLink1b < testLink3e );
    ASSERT_TRUE( testLink1d < testLink3a );
    ASSERT_TRUE( testLink1d < testLink3b );
    ASSERT_TRUE( testLink1d < testLink3d );
    ASSERT_TRUE( testLink1d < testLink3e );
    ASSERT_TRUE( testLink1e < testLink3a );
    ASSERT_TRUE( testLink1e < testLink3b );
    ASSERT_TRUE( testLink1e < testLink3d );
    ASSERT_TRUE( testLink1e < testLink3e );
  }

  TEST_F(HepMcParticleLink_test, max_event_number) {
    // create dummy input McEventCollection with a name that
    // HepMcParticleLink knows about
    SG::WriteHandle<McEventCollection> inputTestDataHandle{"TruthEvent"};
    inputTestDataHandle = std::make_unique<McEventCollection>();

    // Add a dummy GenEvent
    const int process_id1(20);
    const int event_number1(2147483647);
    // 2147483647 = 2^31 -1 is the largest supported event number by
    // HepMcParticleLink as the 32nd bit of the unsigned int used to
    // hold the eventIndex is used to flag whether the it represents the
    // position of the GenEvent in the McEventCollection or the
    // GenEvent::event_number.
    inputTestDataHandle->push_back(HepMC::newGenEvent(process_id1, event_number1));
    const HepMcParticleLink::index_type dummyIndex1(0);
    const HepMcParticleLink::index_type refEvtNum1 = static_cast<HepMcParticleLink::index_type>(event_number1);
    HepMC::GenEvent& ge1 = *(inputTestDataHandle->at(0));
    HepMC::ConstGenParticlePtr particle1 = populateGenEvent(ge1);
    // Add a second dummy GenEvent
    const int process_id2(20);
    const int event_number2(25);
    inputTestDataHandle->push_back(HepMC::newGenEvent(process_id2, event_number2));
    const HepMcParticleLink::index_type dummyIndex2(1);
    const HepMcParticleLink::index_type refEvtNum2 = static_cast<HepMcParticleLink::index_type>(event_number2);
    HepMC::GenEvent& ge2 = *(inputTestDataHandle->at(1));
    HepMC::ConstGenParticlePtr particle2 = populateGenEvent2(ge2);
    // Add a third dummy GenEvent
    const int process_id3(20);
    const int event_number3(17);
    inputTestDataHandle->push_back(HepMC::newGenEvent(process_id3, event_number3));

    //Testing links to the first dummy GenEvent
    const IProxyDict* sg = SG::CurrentEventStore::store();

    // HepMcParticleLink built using a GenParticle pointer and the
    // position of the GenEvent.
    HepMcParticleLink testLink1a(particle1, dummyIndex1,
                                 HepMcParticleLink::IS_POSITION);
    ASSERT_TRUE( testLink1a.isValid() );
    ASSERT_EQ( HepMC::barcode(particle1), testLink1a.barcode());
    ASSERT_EQ( refEvtNum1, testLink1a.eventIndex());
    ASSERT_EQ( dummyIndex1, testLink1a.getEventPositionInCollection(sg));
    ASSERT_EQ(particle1,testLink1a.cptr());
    // A HepMcParticleLink built using the barcode and the position of
    // the GenEvent.
    HepMcParticleLink testLink1b(HepMC::barcode(particle1), dummyIndex1,
                                 HepMcParticleLink::IS_POSITION, HepMcParticleLink::IS_BARCODE);
    ASSERT_TRUE( testLink1b.isValid() );
    ASSERT_EQ( HepMC::barcode(particle1), testLink1b.barcode());
    ASSERT_EQ( refEvtNum1, testLink1b.eventIndex());
    ASSERT_EQ( dummyIndex1, testLink1b.getEventPositionInCollection(sg));
    ASSERT_EQ(*(testLink1a.cptr()),*(testLink1b.cptr()));
    // HepMcParticleLink built using a GenParticle pointer and the
    // event_number of the GenEvent.
    HepMcParticleLink testLink1c(particle1,event_number1);
    ASSERT_TRUE( testLink1c.isValid() );
    ASSERT_EQ( HepMC::barcode(particle1), testLink1c.barcode());
    ASSERT_NE( dummyIndex1, testLink1c.eventIndex());
    ASSERT_EQ( refEvtNum1, testLink1c.eventIndex());
    ASSERT_EQ( dummyIndex1, testLink1c.getEventPositionInCollection(sg));
    ASSERT_EQ(particle1,testLink1c.cptr());

    //Testing links to the second dummy GenEvent

    // HepMcParticleLink built using a GenParticle pointer and the
    // position of the GenEvent.
    HepMcParticleLink testLink2a(particle2, dummyIndex2,
                                 HepMcParticleLink::IS_POSITION);
    ASSERT_TRUE( testLink2a.isValid() );
    ASSERT_EQ( HepMC::barcode(particle2), testLink2a.barcode());
    ASSERT_EQ( refEvtNum2, testLink2a.eventIndex());
    ASSERT_EQ( dummyIndex2, testLink2a.getEventPositionInCollection(sg));
    ASSERT_EQ(particle2,testLink2a.cptr());
    // A HepMcParticleLink built using the barcode and the position of
    // the GenEvent.
    HepMcParticleLink testLink2b(HepMC::barcode(particle2), dummyIndex2,
                                 HepMcParticleLink::IS_POSITION, HepMcParticleLink::IS_BARCODE);
    ASSERT_TRUE( testLink2b.isValid() );
    ASSERT_EQ( HepMC::barcode(particle2), testLink2b.barcode());
    ASSERT_EQ( refEvtNum2, testLink2b.eventIndex());
    ASSERT_EQ( dummyIndex2, testLink2b.getEventPositionInCollection(sg));
    ASSERT_EQ(particle2,testLink2b.cptr());
    // HepMcParticleLink built using a GenParticle pointer and the
    // event_number of the GenEvent.
    HepMcParticleLink testLink2c(particle2,event_number2);
    ASSERT_TRUE( testLink2c.isValid() );
    ASSERT_EQ( HepMC::barcode(particle2), testLink2c.barcode());
    ASSERT_NE( dummyIndex2, testLink2c.eventIndex());
    ASSERT_EQ( refEvtNum2, testLink2c.eventIndex());
    ASSERT_EQ( dummyIndex2, testLink2c.getEventPositionInCollection(sg));
    ASSERT_EQ(particle2,testLink2c.cptr());
  }

  TEST_F(HepMcParticleLink_test, size) {
    // We may potentially have a lot of these objects.
    // Make sure we know if the size increases.
    if (sizeof (void*) <= 8) {
      ASSERT_LE( sizeof(HepMcParticleLink), 40u );
    }
  }

} // <-- namespace MCTesting


int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest( &argc, argv );
  ::testing::AddGlobalTestEnvironment( new MCTesting::GaudiEnvironment );
  return RUN_ALL_TESTS();
}
