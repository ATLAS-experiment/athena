/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#undef NDEBUG

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

#include "xAODMuonSimHit/MuonSimHitContainer.h"
#include "xAODMuonSimHit/MuonSimHitAuxContainer.h"

#include "GaudiKernel/SystemOfUnits.h"
#include "TestTools/initGaudi.h"
#include "StoreGate/StoreGateSvc.h"
#include "StoreGate/WriteHandle.h"

#include "gtest/gtest.h"

#include <stdlib.h>
#include <stdio.h>
#include <climits>


#define COMPARE_VEC(v1, v2)                 \
    for (std::size_t i = 0 ; i < 3; ++i) {  \
        ASSERT_FLOAT_EQ(v1[i], v2[i]);      \
    }

#define TEST_PROP(hit, GETTER, SETTER, TEST_VAL)     \
    hit->SETTER(TEST_VAL);                           \
    ASSERT_FLOAT_EQ(TEST_VAL, hit->GETTER());

using namespace xAOD;

namespace SimHitTesting {

    struct ContainerHolder{
        public:
            ContainerHolder(){
                m_container->setStore(m_auxContainer.get());
            }

            bool empty() const { return m_container->empty(); }
            std::size_t size() const {return m_container->size(); }
            MuonSimHit* newHit() {return m_container->push_back(std::make_unique<MuonSimHit>()); }

            MuonSimHitContainer* get() const { return m_container.get();}
        private:
            std::unique_ptr<MuonSimHitContainer> m_container{std::make_unique<MuonSimHitContainer>()};
            std::unique_ptr<MuonSimHitAuxContainer> m_auxContainer{std::make_unique<MuonSimHitAuxContainer>()};
    };


    // needed every time an AthAlgorithm, AthAlgTool or AthService is instantiated
    ISvcLocator* g_svcLoc = nullptr;

    // global test environment takes care of setting up Gaudi
    class GaudiEnvironment : public ::testing::Environment {
    protected:
      virtual void SetUp() override {
        Athena_test::initGaudi(SimHitTesting::g_svcLoc);
      }
    };

    class SimHitContainerTest : public ::testing::Test {
        protected:
          virtual void SetUp() override {
          }
      
          virtual void TearDown() override {
            SmartIF<StoreGateSvc> pStore(SimHitTesting::g_svcLoc->service("StoreGateSvc"));
            ASSERT_TRUE(pStore.isValid());
            pStore->clearStore(true).ignore(); // forceRemove=true to remove all proxies
          }
    };

     HepMC::GenParticlePtr populateGenEvent(HepMC::GenEvent & ge, const int ev)
  {
    HepMC::FourVector myPos( 0.0, 0.0, 0.0, 0.0);
    HepMC::GenVertexPtr myVertex = HepMC::newGenVertexPtr( myPos, -1 );
    HepMC::FourVector fourMomentum1( 0.0, 0.0, 1.0, 1.0*Gaudi::Units::TeV);
    HepMC::GenParticlePtr inParticle1 = HepMC::newGenParticlePtr(fourMomentum1, 2, 10);
    myVertex->add_particle_in(inParticle1);
    HepMC::FourVector fourMomentum2( 0.0, 0.0, -1.0, 1.0*Gaudi::Units::TeV);
    HepMC::GenParticlePtr inParticle2 = HepMC::newGenParticlePtr(fourMomentum2, -2, 10);
    myVertex->add_particle_in(inParticle2);
    HepMC::FourVector fourMomentum3( 0.0, 1.0, 0.0, 1.0*Gaudi::Units::TeV);
    HepMC::GenParticlePtr inParticle3 = HepMC::newGenParticlePtr(fourMomentum3, 13, 10);
    myVertex->add_particle_out(inParticle3);
    HepMC::FourVector fourMomentum4( 0.0, -1.0, 0.0, 1.0*Gaudi::Units::TeV);
    HepMC::GenParticlePtr inParticle4 = HepMC::newGenParticlePtr(fourMomentum4, -13, 10);
    myVertex->add_particle_out(inParticle4);
    ge.add_vertex( myVertex );
    HepMC::suggest_barcode(inParticle1,1 + 4*ev);
    HepMC::suggest_barcode(inParticle2,2 + 4*ev);
    HepMC::suggest_barcode(inParticle3,3 + 4*ev);
    HepMC::suggest_barcode(inParticle4,4 + 4*ev);
    HepMC::set_signal_process_vertex(&ge, myVertex );
    ge.set_beam_particles(std::move(inParticle1),std::move(inParticle2));
    return inParticle3;
  }

  


TEST_F(SimHitContainerTest, testHitConstruction) {
    ContainerHolder newContainer{};
    
    ASSERT_TRUE(newContainer.empty());

    MuonSimHit* testHit = newContainer.newHit();
    ASSERT_EQ(newContainer.size(), 1u);

    ASSERT_NE(testHit, nullptr);
    ASSERT_EQ(testHit->container(), newContainer.get());
    
    MeasVector<3> locPos {2.f, 3.f, 4.f};
    testHit->setLocalPosition(locPos);
        
    COMPARE_VEC(locPos, testHit->localPosition());

    MeasVector<3> locDir{MeasVector<3>{1., 2., 3.}.normalized()};
    testHit->setLocalDirection(locDir);
    COMPARE_VEC(locDir, testHit->localDirection());

    TEST_PROP(testHit, stepLength, setStepLength, 42.f);
    TEST_PROP(testHit, mass, setMass, 511.f);
    TEST_PROP(testHit, kineticEnergy, setKineticEnergy, 123456.f); 
    TEST_PROP(testHit, globalTime, setGlobalTime, 321.f); 
    TEST_PROP(testHit, pdgId, setPdgId, -13); 
    TEST_PROP(testHit, energyDeposit, setEnergyDeposit, 987.f); 
    
    Identifier id{5213};
    testHit->setIdentifier(id);
    ASSERT_EQ(testHit->identify(), id);
}

TEST_F(SimHitContainerTest, copyAssignment) {
    ContainerHolder container1{};
    ContainerHolder container2{};

    MuonSimHit* testHit1 = container1.newHit();
    
    testHit1->setLocalPosition(MeasVector<3>{300,400.,500.});
    testHit1->setLocalDirection(MeasVector<3>{1.,0.,1.}.normalized());
    testHit1->setStepLength(77.);
    testHit1->setMass(666.);
    testHit1->setKineticEnergy(64156.);
    testHit1->setGlobalTime(52.);
    testHit1->setPdgId(13);
    testHit1->setEnergyDeposit(1305);
    testHit1->setIdentifier(Identifier{1404});

    MuonSimHit* testHit2 = container2.newHit();
    (*testHit2) = (*testHit1);

    ASSERT_NE(testHit1->container(), testHit2->container());
   
    COMPARE_VEC(testHit1->localPosition(), testHit2->localPosition());
    COMPARE_VEC(testHit1->localDirection(), testHit2->localDirection());

    ASSERT_FLOAT_EQ(testHit1->stepLength(), testHit2->stepLength());
    ASSERT_FLOAT_EQ(testHit1->mass(), testHit2->mass());
    ASSERT_FLOAT_EQ(testHit1->kineticEnergy(), testHit2->kineticEnergy());
    ASSERT_FLOAT_EQ(testHit1->globalTime(), testHit2->globalTime());
    ASSERT_FLOAT_EQ(testHit1->pdgId(), testHit2->pdgId());
    ASSERT_FLOAT_EQ(testHit1->energyDeposit(), testHit2->energyDeposit());
    ASSERT_FLOAT_EQ(testHit1->identify().get_compact(), testHit2->identify().get_compact());
}

TEST_F(SimHitContainerTest, HepMCLinkAssignment) {


    ContainerHolder newContainer{};    
    ASSERT_TRUE(newContainer.empty());


    // create dummy input McEventCollection with a name that
    // HepMcParticleLink knows about
    SG::WriteHandle<McEventCollection> inputTestDataHandle{"TruthEvent"};
    inputTestDataHandle = std::make_unique<McEventCollection>();
    // Add a dummy GenEvent
    constexpr int process_id(52);
    constexpr int event_number1(100);
    constexpr int event_number2(2500);
    constexpr int event_number3(17);
   
 
    HepMC::GenEvent& genEvent1{*inputTestDataHandle->push_back(HepMC::newGenEvent(process_id, event_number1))};
    constexpr HepMcParticleLink::index_type dummyIndex1 = 0;
    const auto refEvtNum1 = static_cast<HepMcParticleLink::index_type>(event_number1);
  
    auto particle1 = populateGenEvent(genEvent1, 0);

    HepMcParticleLink testLink1{particle1, dummyIndex1,
                                 HepMcParticleLink::IS_POSITION};
    ASSERT_TRUE( testLink1.isValid() );
    ASSERT_EQ(HepMC::barcode(particle1), testLink1.barcode());
    ASSERT_EQ(HepMC::uniqueID(particle1), testLink1.id());
    ASSERT_EQ( refEvtNum1, testLink1.eventIndex());
    ASSERT_EQ(dummyIndex1, testLink1.getEventPositionInCollection(SG::CurrentEventStore::store()));
    ASSERT_EQ(particle1, testLink1.cptr());

    MuonSimHit* testHit1 = newContainer.newHit();

    ASSERT_NE(testHit1, nullptr);
    ASSERT_EQ(testHit1->genParticleLink().cptr(), nullptr);
    
    testHit1->setGenParticleLink(testLink1);

    ASSERT_TRUE(testHit1->genParticleLink().isValid());
    ASSERT_EQ(testHit1->genParticleLink().cptr(), particle1);

    // Add a second dummy GenEvent
    constexpr HepMcParticleLink::index_type dummyIndex2 = 1;
    const auto refEvtNum2 = static_cast<HepMcParticleLink::index_type>(event_number2);
    HepMC::GenEvent& genEvent2{*inputTestDataHandle->push_back(HepMC::newGenEvent(process_id, event_number2))};
    auto particle2 = populateGenEvent(genEvent2, 1);


    HepMcParticleLink testLink2{particle2, dummyIndex2,
                                 HepMcParticleLink::IS_POSITION};
    ASSERT_TRUE( testLink2.isValid() );
    ASSERT_EQ(HepMC::barcode(particle2), testLink2.barcode());
    ASSERT_EQ(HepMC::uniqueID(particle2), testLink2.id());
    ASSERT_EQ( refEvtNum2, testLink2.eventIndex());
    ASSERT_EQ(dummyIndex2, testLink2.getEventPositionInCollection(SG::CurrentEventStore::store()));
    ASSERT_EQ(particle2, testLink2.cptr());

    MuonSimHit* testHit2 = newContainer.newHit();
    ASSERT_EQ(testHit2->genParticleLink().cptr(), nullptr);
    ASSERT_NE(testHit2, nullptr);
    ASSERT_NE(testHit1, testHit2);

    testHit2->setGenParticleLink(testLink2);

    ASSERT_TRUE(testHit2->genParticleLink().isValid());
    ASSERT_EQ(testHit2->genParticleLink().cptr(), particle2);

    
    // Add a third dummy GenEvent
    constexpr HepMcParticleLink::index_type dummyIndex3 = 2;
    const auto refEvtNum3 = static_cast<HepMcParticleLink::index_type>(event_number3);
    HepMC::GenEvent& genEvent3{*inputTestDataHandle->push_back(HepMC::newGenEvent(process_id, event_number3))};

    auto particle3 = populateGenEvent(genEvent3, 2);
    HepMcParticleLink testLink3{particle3, dummyIndex3,
                                 HepMcParticleLink::IS_POSITION};
    ASSERT_TRUE( testLink3.isValid() );
    ASSERT_EQ(HepMC::barcode(particle3), testLink3.barcode());
    ASSERT_EQ(HepMC::uniqueID(particle3), testLink3.id());
    ASSERT_EQ( refEvtNum3, testLink3.eventIndex());
    ASSERT_EQ(dummyIndex3, testLink3.getEventPositionInCollection(SG::CurrentEventStore::store()));
    ASSERT_EQ(particle3, testLink3.cptr());

    MuonSimHit* testHit3 = newContainer.newHit();
    ASSERT_EQ(testHit3->genParticleLink().cptr(), nullptr);

    ASSERT_NE(testHit3, nullptr);
    ASSERT_NE(testHit1, testHit2);
    ASSERT_NE(testHit2, testHit3);    
    ASSERT_NE(testHit1, testHit3);


    testHit3->setGenParticleLink(testLink3);

    ASSERT_TRUE(testHit3->genParticleLink().isValid());
    ASSERT_EQ(testHit3->genParticleLink().cptr(), particle3);
   
    testHit1->releaseParticleLink();
    testHit2->releaseParticleLink();
    testHit3->releaseParticleLink();

    ASSERT_EQ(testHit1->genParticleLink().cptr(), particle1);
    ASSERT_EQ(testHit2->genParticleLink().cptr(), particle2);    
    ASSERT_EQ(testHit3->genParticleLink().cptr(), particle3);
}

TEST_F(SimHitContainerTest, HepMCLinkCopy) {

    ContainerHolder newContainer1{};    
    ContainerHolder newContainer2{};    
    
    ASSERT_TRUE(newContainer1.empty());
    ASSERT_TRUE(newContainer2.empty());
    // create dummy input McEventCollection with a name that
    // HepMcParticleLink knows about
    SG::WriteHandle<McEventCollection> inputTestDataHandle{"TruthEvent"};
    inputTestDataHandle = std::make_unique<McEventCollection>();
    // Add a dummy GenEvent
    constexpr int process_id = 52;
    for (std::uint32_t e =0 ; e< 31 ; ++e) {
        const std::uint32_t evtNum = e + (1<<e);
        HepMC::GenEvent& genEvent{*inputTestDataHandle->push_back(HepMC::newGenEvent(process_id, evtNum))};

        auto particle = populateGenEvent(genEvent, e);

        HepMcParticleLink testLink{particle, evtNum, HepMcParticleLink::IS_EVENTNUM};
        ASSERT_TRUE( testLink.isValid() );
        ASSERT_EQ(HepMC::barcode(particle), testLink.barcode());
        ASSERT_EQ(HepMC::uniqueID(particle), testLink.id());
        ASSERT_EQ(evtNum, testLink.eventIndex());
        ASSERT_EQ(e, testLink.getEventPositionInCollection(SG::CurrentEventStore::store()));
        ASSERT_EQ(particle, testLink.cptr());

        MuonSimHit* hit1 = newContainer1.newHit();
        ASSERT_NE(hit1, nullptr);
        ASSERT_EQ(hit1->index(), e);
        ASSERT_EQ(newContainer1.size(), e+1);

        hit1->setGenParticleLink(testLink);
        ASSERT_EQ(hit1->genParticleLink().cptr(), particle);

        MuonSimHit* hit2 = newContainer2.newHit();
        ASSERT_NE(hit2, nullptr);
        ASSERT_NE(hit1, hit2);
        ASSERT_EQ(hit2->index(), e);
        ASSERT_EQ(newContainer2.size(), e+1);
        ASSERT_EQ(hit2->genParticleLink().cptr(), nullptr);
      
        (*hit2) = (*hit1);
        ASSERT_EQ(hit2->genParticleLink().cptr(), particle);
    }
}
}

int main(int argc, char *argv[]){
 
    ::testing::InitGoogleTest( &argc, argv );
    ::testing::AddGlobalTestEnvironment( new SimHitTesting::GaudiEnvironment );

    return RUN_ALL_TESTS();
}