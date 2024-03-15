/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file GeneratorObjectsTPCnv/test/HepMcParticleLinkCnv_p1_test.cxx
 * @brief Tests for HepMcParticleLinkCnv_p1.
 */


#undef NDEBUG
#include <cassert>
#include <iostream>
#include <vector>
// HepMC includes
#include "AtlasHepMC/GenEvent.h"
#include "TruthUtils/MagicNumbers.h"

// CLHEP includes
#include "CLHEP/Units/SystemOfUnits.h"

#include "GeneratorObjectsTPCnv/HepMcParticleLinkCnv_p1.h"
#include "StoreGate/WriteHandle.h"
#include "GeneratorObjects/McEventCollection.h"

#include "TestTools/initGaudi.h"

void compare (const HepMcParticleLink& p1,
              const HepMcParticleLink& p2)
{
  assert ( p1.isValid() == p2.isValid() );
  assert ( p1.barcode() == p2.barcode() );
  assert ( p1.eventIndex() == p2.eventIndex() );
  assert ( p1.cptr() == p2.cptr() );
  assert ( p1 == p2 );
}

int maximumBarcode(std::vector<HepMC::GenParticlePtr>& genPartList)
{
  int maxBarcode{0};
  if (genPartList.empty()) { return maxBarcode; }
  for (HepMC::GenParticlePtr genPart : genPartList)
    {
      maxBarcode = std::max(maxBarcode, HepMC::barcode(genPart));
    }
  return maxBarcode;
}

void populateGenEvent(HepMC::GenEvent & ge, int pdgid1, int pdgid2, std::vector<HepMC::GenParticlePtr>& genPartList)
{
  int maxBarcode = maximumBarcode(genPartList);
  HepMC::FourVector myPos( 0.0, 0.0, 0.0, 0.0);
  HepMC::GenVertexPtr myVertex = HepMC::newGenVertexPtr( myPos, -1 );
  HepMC::FourVector fourMomentum1( 0.0, 0.0, 1.0, 1.0*CLHEP::TeV);
  HepMC::GenParticlePtr inParticle1 = HepMC::newGenParticlePtr(fourMomentum1, pdgid1, 2);
  myVertex->add_particle_in(inParticle1);
  HepMC::FourVector fourMomentum2( 0.0, 0.0, -1.0, 1.0*CLHEP::TeV);
  HepMC::GenParticlePtr inParticle2 = HepMC::newGenParticlePtr(fourMomentum2, pdgid2, 2);
  myVertex->add_particle_in(inParticle2);
  HepMC::FourVector fourMomentum3( 0.0, 1.0, 0.0, 1.0*CLHEP::TeV);
  HepMC::GenParticlePtr inParticle3 = HepMC::newGenParticlePtr(fourMomentum3, pdgid1, 1);
  myVertex->add_particle_out(inParticle3);
  genPartList.push_back(inParticle3);
  HepMC::FourVector fourMomentum4( 0.0, -1.0, 0.0, 1.0*CLHEP::TeV);
  HepMC::GenParticlePtr inParticle4 = HepMC::newGenParticlePtr(fourMomentum4, pdgid2, 1);
  myVertex->add_particle_out(inParticle4);
  genPartList.push_back(inParticle4);
  ge.add_vertex( myVertex );
  HepMC::suggest_barcode(inParticle1,maxBarcode+1);
  HepMC::suggest_barcode(inParticle2,maxBarcode+2);
  HepMC::suggest_barcode(inParticle3,maxBarcode+3);
  HepMC::suggest_barcode(inParticle4,maxBarcode+4);
  HepMC::set_signal_process_vertex(&ge, myVertex );
  ge.set_beam_particles(inParticle1,inParticle2);
}

void populateFilteredGenEvent(HepMC::GenEvent & ge, std::vector<HepMC::GenParticlePtr>& genPartList)
{
  //.......Create new particle (geantino) to link  hits from pileup
  HepMC::GenParticlePtr genPart=HepMC::newGenParticlePtr();
  genPart->set_pdg_id(999); //Geantino
  genPart->set_status(1); //!< set decay status
  HepMC::suggest_barcode(genPart, HepMC::SUPPRESSED_PILEUP_BARCODE);

  HepMC::GenVertexPtr genVertex=HepMC::newGenVertexPtr();
  genVertex->add_particle_out(genPart);
  genPartList.push_back(genPart);

  //to set geantino vertex as a truth primary vertex
  HepMC::GenVertexPtr hScatVx = HepMC::barcode_to_vertex(&ge,-3);
  if(hScatVx!=nullptr) {
    HepMC::FourVector pmvxpos=hScatVx->position();
    genVertex->set_position(pmvxpos);
    //to set geantino kinematic phi=eta=0, E=p=E_hard_scat
#ifdef HEPMC3
    auto itrp =hScatVx->particles_in().cbegin();
    if (hScatVx->particles_in().size()==2){
      HepMC::FourVector mom1=(*itrp)->momentum();
      HepMC::FourVector mom2=(*(++itrp))->momentum();
      HepMC::FourVector vxmom;
      vxmom.setPx(mom1.e()+mom2.e());
      vxmom.setPy(0.);
      vxmom.setPz(0.);
      vxmom.setE(mom1.e()+mom2.e());
      genPart->set_momentum(vxmom);
    }
#else
    HepMC::GenVertex::particles_in_const_iterator itrp =hScatVx->particles_in_const_begin();
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
#endif
  }

#ifdef HEPMC3
  if(!ge.vertices().empty()){
    std::vector<HepMC::GenVertexPtr> vtxvec;
    for (const auto& vtx: ge.vertices()) {
      vtxvec.push_back(vtx);
      ge.remove_vertex(vtx);
    }
    vtxvec.clear();
  }
#else
  if(!ge.vertices_empty()){
    std::vector<HepMC::GenVertexPtr> vtxvec;
    HepMC::GenEvent::vertex_iterator itvtx = ge.vertices_begin();
    for (;itvtx != ge.vertices_end(); ++itvtx ) {
      ge.remove_vertex(*itvtx);
      vtxvec.push_back((*itvtx));
      //fix me: delete vertex pointer causes crash
      //delete (*itvtx);
    }
    for(unsigned int i=0;i<vtxvec.size();i++)  delete vtxvec[i];
  }
#endif

  //.....add new vertex with geantino
  ge.add_vertex(genVertex);
  HepMC::suggest_barcode(genPart, HepMC::SUPPRESSED_PILEUP_BARCODE );
}

void createMcEventCollectionInStoreGate(std::vector<HepMC::GenParticlePtr>& genPartList)
{
  // create dummy input McEventCollection with a name that
  // HepMcParticleLink knows about
  SG::WriteHandle<McEventCollection> inputTestDataHandle{"TruthEvent"};
  inputTestDataHandle = std::make_unique<McEventCollection>();
  // create a dummy EventContext
  EventContext ctx;
  ctx.setExtension( Atlas::ExtendedEventContext( SG::CurrentEventStore::store() ) );
  Gaudi::Hive::setCurrentContext( ctx );

  // Add a dummy GenEvent
  const int process_id1(20);
  const int event_number1(2147483647);
  // 2147483647 = 2^31 -1 is the largest supported event number by
  // HepMcParticleLink as the 32nd bit of the unsigned int used to
  // hold the eventIndex is used to flag whether the it represents the
  // position of the GenEvent in the McEventCollection or the
  // GenEvent::event_number.
  const int event_number2(std::numeric_limits<unsigned short>::max());
  // 2^16 -1 is the largest event number supported by
  // HepMcParticleLink_p1. A workaround is used to suppport larger
  // values for the first event in the McEventCollection.
  const int event_number3(64);
  const int event_number4(89);
  inputTestDataHandle->push_back(HepMC::newGenEvent(process_id1, event_number1));
  HepMC::GenEvent& ge1 = *(inputTestDataHandle->at(0));
  populateGenEvent(ge1,-11,11,genPartList);
  populateGenEvent(ge1,-13,13,genPartList);
  inputTestDataHandle->push_back(HepMC::newGenEvent(process_id1, event_number2));
  HepMC::GenEvent& ge2 = *(inputTestDataHandle->at(1));
  populateGenEvent(ge2,-11,11,genPartList);
  populateGenEvent(ge2,-13,13,genPartList);
  inputTestDataHandle->push_back(HepMC::newGenEvent(process_id1, event_number3));
  HepMC::GenEvent& ge3 = *(inputTestDataHandle->at(2));
  populateGenEvent(ge3,-11,11,genPartList);
  populateGenEvent(ge3,22,22,genPartList);
  inputTestDataHandle->push_back(new HepMC::GenEvent(ge1));
  HepMC::GenEvent& ge4 = *(inputTestDataHandle->at(3));
  ge4.set_event_number(event_number4);
  populateFilteredGenEvent(ge4,genPartList);
}

void testit (const HepMcParticleLink& trans1)
{
  MsgStream log (nullptr, "test");
  HepMcParticleLinkCnv_p1 cnv;
  HepMcParticleLink_p1 pers;
  cnv.transToPers (&trans1, &pers, log);
  HepMcParticleLink trans2;
  cnv.persToTrans (&pers, &trans2, log);

  compare (trans1, trans2);
}

void test1()
{
  std::cout << "test1\n";
  std::vector<HepMC::GenParticlePtr> genPartList;
  createMcEventCollectionInStoreGate(genPartList);
  assert ( genPartList.size() == 13 );
  // HepMcParticleLinks pointing at GenParticles in the first GenEvent in the McEventCollection
  HepMC::ConstGenParticlePtr particle1 = genPartList.at(0);
  // By ConstGenParticlePtr + event_number
  HepMcParticleLink trans1a(particle1,particle1->parent_event()->event_number());
  testit (trans1a);
  // By barcode + event_number
  HepMcParticleLink trans1b(HepMC::barcode(particle1),particle1->parent_event()->event_number(),HepMcParticleLink::IS_EVENTNUM,HepMcParticleLink::IS_BARCODE);
  testit (trans1b);
  // By id + event_number
  HepMcParticleLink trans1c(HepMC::uniqueID(particle1),particle1->parent_event()->event_number(),HepMcParticleLink::IS_EVENTNUM,HepMcParticleLink::IS_ID);
  testit (trans1c);
  // By ConstGenParticlePtr + position
  HepMcParticleLink trans1d(particle1,0,HepMcParticleLink::IS_POSITION);
  testit (trans1d);
  // By barcode + position
  HepMcParticleLink trans1e(HepMC::barcode(particle1),0,HepMcParticleLink::IS_POSITION,HepMcParticleLink::IS_BARCODE);
  testit (trans1e);
  // By id + position
  HepMcParticleLink trans1f(HepMC::uniqueID(particle1),0,HepMcParticleLink::IS_POSITION,HepMcParticleLink::IS_ID);
  testit (trans1f);

  // HepMcParticleLinks pointing at GenParticles in other GenEvents in the McEventCollection
  HepMC::ConstGenParticlePtr particle2 = genPartList.at(7);
  // By ConstGenParticlePtr + event_number
  HepMcParticleLink trans2a(particle2,particle2->parent_event()->event_number());
  testit (trans2a);
  // By barcode + event_number
  HepMcParticleLink trans2b(HepMC::barcode(particle2),particle2->parent_event()->event_number(),HepMcParticleLink::IS_EVENTNUM,HepMcParticleLink::IS_BARCODE);
  testit (trans2b);
  // By id + event_number
  HepMcParticleLink trans2c(HepMC::uniqueID(particle2),particle2->parent_event()->event_number(),HepMcParticleLink::IS_EVENTNUM,HepMcParticleLink::IS_ID);
  testit (trans2c);
  // By ConstGenParticlePtr + position
  HepMcParticleLink trans2d(particle2,1,HepMcParticleLink::IS_POSITION);
  testit (trans2d);
  // By barcode + position
  HepMcParticleLink trans2e(HepMC::barcode(particle2),1,HepMcParticleLink::IS_POSITION,HepMcParticleLink::IS_BARCODE);
  testit (trans2e);
  // By id + position
  HepMcParticleLink trans2f(HepMC::uniqueID(particle2),1,HepMcParticleLink::IS_POSITION,HepMcParticleLink::IS_ID);
  testit (trans2f);
  //---------------------------------------------
  HepMC::ConstGenParticlePtr particle3 = genPartList.at(8);
  // By ConstGenParticlePtr + event_number
  HepMcParticleLink trans3a(particle3,particle3->parent_event()->event_number());
  testit (trans3a);
  // By barcode + event_number
  HepMcParticleLink trans3b(HepMC::barcode(particle3),particle3->parent_event()->event_number(),HepMcParticleLink::IS_EVENTNUM,HepMcParticleLink::IS_BARCODE);
  testit (trans3b);
  // By id + event_number
  HepMcParticleLink trans3c(HepMC::uniqueID(particle3),particle3->parent_event()->event_number(),HepMcParticleLink::IS_EVENTNUM,HepMcParticleLink::IS_ID);
  testit (trans3c);
  // By ConstGenParticlePtr + position
  HepMcParticleLink trans3d(particle3,2,HepMcParticleLink::IS_POSITION);
  testit (trans3d);
  // By barcode + position
  HepMcParticleLink trans3e(HepMC::barcode(particle3),2,HepMcParticleLink::IS_POSITION,HepMcParticleLink::IS_BARCODE);
  testit (trans3e);
  // By id + position
  HepMcParticleLink trans3f(HepMC::uniqueID(particle3),2,HepMcParticleLink::IS_POSITION,HepMcParticleLink::IS_ID);
  testit (trans3f);

  // HepMcParticleLinks pointing at filtered pileup truth
  HepMC::ConstGenParticlePtr particle4 = genPartList.at(12);
  // By ConstGenParticlePtr + event_number
  HepMcParticleLink trans4a(particle4,particle4->parent_event()->event_number());
  testit (trans4a);
  // By barcode + event_number
  HepMcParticleLink trans4b(HepMC::barcode(particle4),particle4->parent_event()->event_number(),HepMcParticleLink::IS_EVENTNUM,HepMcParticleLink::IS_BARCODE);
  testit (trans4b);
  // By id + event_number
  HepMcParticleLink trans4c(HepMC::uniqueID(particle4),particle4->parent_event()->event_number(),HepMcParticleLink::IS_EVENTNUM,HepMcParticleLink::IS_ID);
  testit (trans4c);
  // By ConstGenParticlePtr + position
  HepMcParticleLink trans4d(particle4,3,HepMcParticleLink::IS_POSITION);
  testit (trans4d);
  // By barcode + position
  HepMcParticleLink trans4e(HepMC::barcode(particle4),3,HepMcParticleLink::IS_POSITION,HepMcParticleLink::IS_BARCODE);
  testit (trans4e);
  // By id + position
  HepMcParticleLink trans4f(HepMC::uniqueID(particle4),3,HepMcParticleLink::IS_POSITION,HepMcParticleLink::IS_ID);
  testit (trans4f);

  // HepMcParticleLinks pointing at delta-ray (barcode/id=0 - not recorded in McEventCollection) using event number
  const int deltaRayBarcode(0);
  const int deltaRayId(0);
  const HepMcParticleLink::index_type deltaRayEventNumber = static_cast<HepMcParticleLink::index_type>(particle1->parent_event()->event_number());
  // By barcode + event_number
  HepMcParticleLink trans5a(deltaRayBarcode,deltaRayEventNumber,HepMcParticleLink::IS_EVENTNUM,HepMcParticleLink::IS_BARCODE);
  testit (trans5a);
  // By id + event_number
  HepMcParticleLink trans5b(deltaRayId,deltaRayEventNumber,HepMcParticleLink::IS_EVENTNUM,HepMcParticleLink::IS_ID);
  testit (trans5b);
  // By barcode + position
  HepMcParticleLink trans5c(deltaRayBarcode,0,HepMcParticleLink::IS_POSITION,HepMcParticleLink::IS_BARCODE);
  testit (trans5c);
  // By id + position
  HepMcParticleLink trans5d(deltaRayId,0,HepMcParticleLink::IS_POSITION,HepMcParticleLink::IS_ID);
  testit (trans5d);

  // Link to a GenParticle which was not recorded to the
  // McEventCollection, even though other parts of the same GenEvent
  // were recorded.
  const int cutParticleBarcode(210001);
  const int cutParticleId(5);
  const HepMcParticleLink::index_type refEventNumber = static_cast<HepMcParticleLink::index_type>(particle1->parent_event()->event_number());
  // By barcode + event_number
  HepMcParticleLink trans6a(cutParticleBarcode,refEventNumber,HepMcParticleLink::IS_EVENTNUM,HepMcParticleLink::IS_BARCODE);
  testit (trans6a);
  // By id + event_number
  HepMcParticleLink trans6b(cutParticleId,refEventNumber,HepMcParticleLink::IS_EVENTNUM,HepMcParticleLink::IS_ID);
  testit (trans6b);
  // By barcode + position
  HepMcParticleLink trans6c(cutParticleBarcode,0,HepMcParticleLink::IS_POSITION,HepMcParticleLink::IS_BARCODE);
  testit (trans6c);
  // By id + position
  HepMcParticleLink trans6d(cutParticleId,0,HepMcParticleLink::IS_POSITION,HepMcParticleLink::IS_ID);
  testit (trans6d);

  // Link to a GenEvent which was not recorded to the McEventCollection
  const HepMcParticleLink::index_type missingEvtNum = static_cast<HepMcParticleLink::index_type>(460);
  // By barcode + event_number
  HepMcParticleLink trans7a(cutParticleBarcode, missingEvtNum,HepMcParticleLink::IS_EVENTNUM,HepMcParticleLink::IS_BARCODE);
  testit (trans7a);
  // By barcode + position - NOT SUPPORTED
  // Position is meaningless if the GenEvent is not in the McEventCollection
  // By id + position - NOT SUPPORTED
  // No way to get back to id after TP conversion if the GenParticle itself is not present
  // Position is meaningless if the GenEvent is not in the McEventCollection
  // By id + event_number - NOT SUPPORTED
  // No way to get back to id after TP conversion if the GenParticle itself is not present
}


int main()
{
  setlinebuf(stdout);
  setlinebuf(stderr);
  ISvcLocator* pSvcLoc = nullptr;
  if (!Athena_test::initGaudi(pSvcLoc)) {
    std::cerr << "This test can not be run" << std::endl;
    return 0;
  }

  test1();
  return 0;
}
