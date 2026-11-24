/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file EvtGen_i/test/EvtGenBarcodes_test.cxx
 * @brief Tests the barcodes of new decay products in an event read from a file.
 *
 * EvtInclusiveDecay with readExisting removes decay trees from an event that has the
 * barcodes stored in the file, and adds new decays. The test does the same with a small
 * event. With barcodes from EvtGenBarcodes::findBarcodeRange, given as in
 * EvtInclusiveDecay::addEvtGenDecayTree, all barcodes are unique, and the decay graph is the
 * same after a write and read with McEventCollectionCnv_p6.
 */

#undef NDEBUG
#include <cassert>
#include <cmath>
#include <iostream>
#include <memory>
#include <set>
#include <utility>
#include <vector>

#include "AthenaKernel/proxyDictFromEventContext.h"
#include "AtlasHepMC/AttributeNames.h"
#include "AtlasHepMC/Barcode.h"
#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenParticle.h"
#include "AtlasHepMC/GenVertex.h"
#include "EvtGen_i/EvtGenBarcodes.h"
#include "GeneratorObjects/McEventCollection.h"
#include "GeneratorObjectsTPCnv/McEventCollectionCnv_p6.h"
#include "GeneratorObjectsTPCnv/McEventCollection_p6.h"
#include "HepMC3/GenRunInfo.h"
#include "SGTools/TestStore.h"
#include "TestTools/initGaudi.h"

namespace {

  HepMC::GenParticlePtr makeParticle(int pdgId, double pz, int status) {
    return HepMC::newGenParticlePtr(HepMC::FourVector(0., 0., pz, std::abs(pz) + 1.), pdgId, status);
  }

  // True if two particles or two vertices of the event have the same barcode.
  bool hasSharedBarcodes(const HepMC::GenEvent& event) {
    std::set<int> particles;
    for (const auto& p : event.particles()) {
      if (!particles.insert(HepMC::barcode(p)).second) {
        return true;
      }
    }
    std::set<int> vertices;
    for (const auto& v : event.vertices()) {
      if (!vertices.insert(HepMC::barcode(v)).second) {
        return true;
      }
    }
    return false;
  }

  // Barcodes for new decay products, as EvtInclusiveDecay keeps them for one event.
  struct NewBarcodes {
    int maxParticle = 0;
    int minVertex = 0;
  };

  // Replaces the decay of a particle by a new decay, in the same order as
  // EvtInclusiveDecay::decayParticle and addEvtGenDecayTree.
  void redecay(HepMC::GenEvent& event, const HepMC::GenParticlePtr& mother, const std::vector<int>& daughters,
               NewBarcodes& barcodes) {
    event.remove_vertex(mother->end_vertex());
    auto vertex = HepMC::newGenVertexPtr();
    event.add_vertex(vertex);
    HepMC::suggest_barcode(vertex, --barcodes.minVertex);
    vertex->add_particle_in(mother);
    for (const int pdgId : daughters) {
      auto daughter = makeParticle(pdgId, 10., 1);
      vertex->add_particle_out(daughter);
      HepMC::suggest_barcode(daughter, ++barcodes.maxParticle);
    }
    HepMC::fillBarcodesAttribute(&event);
  }

  // For every vertex, the barcodes of its incoming and outgoing particles.
  std::vector<std::pair<std::set<int>, std::set<int>>> decayGraph(const HepMC::GenEvent& event) {
    std::vector<std::pair<std::set<int>, std::set<int>>> graph;
    for (const auto& v : event.vertices()) {
      std::set<int> in;
      for (const auto& p : v->particles_in()) {
        in.insert(HepMC::barcode(p));
      }
      std::set<int> out;
      for (const auto& p : v->particles_out()) {
        out.insert(HepMC::barcode(p));
      }
      graph.emplace_back(in, out);
    }
    return graph;
  }

  // Replaces the B+ decay and then the D0 decay of an event as read from a file. After the
  // first decay the kept D decays move to lower ids, and the new particles and vertices get
  // ids that are barcodes of the kept ones. After the second decay, the new B+ decay vertex
  // gets the id that is the barcode of the kept D+ decay vertex.
  void run(SGTest::TestStore& store) {
    Atlas::setProxyDictInEventContext(&store);
    auto runInfo = std::make_shared<HepMC3::GenRunInfo>();
    runInfo->set_weight_names({"weight1"});
    McEventCollection collection;
    collection.push_back(HepMC::newGenEvent(1, 1));
    HepMC::GenEvent& event = *collection.back();
    event.set_run_info(std::move(runInfo));
    event.weights() = {1.};

    auto beam1 = makeParticle(2212, 6800., 4);
    auto beam2 = makeParticle(2212, -6800., 4);
    auto bPlus = makeParticle(521, 50., 2);
    auto dPlus = makeParticle(411, 40., 2);
    auto dZero = makeParticle(421, 35., 2);
    auto hard = HepMC::newGenVertexPtr();
    hard->add_particle_in(beam1);
    hard->add_particle_in(beam2);
    hard->add_particle_out(makeParticle(21, 30., 1));
    hard->add_particle_out(bPlus);
    hard->add_particle_out(makeParticle(21, 20., 1));
    hard->add_particle_out(dPlus);
    hard->add_particle_out(makeParticle(21, 10., 1));
    hard->add_particle_out(dZero);
    event.add_vertex(std::move(hard));
    auto bDecay = HepMC::newGenVertexPtr();
    bDecay->add_particle_in(bPlus);
    bDecay->add_particle_out(makeParticle(-421, 25., 2));
    bDecay->add_particle_out(makeParticle(211, 25., 1));
    event.add_vertex(std::move(bDecay));
    auto dPlusDecay = HepMC::newGenVertexPtr();
    dPlusDecay->add_particle_in(std::move(dPlus));
    dPlusDecay->add_particle_out(makeParticle(-321, 20., 1));
    dPlusDecay->add_particle_out(makeParticle(211, 20., 1));
    event.add_vertex(std::move(dPlusDecay));
    auto dZeroDecay = HepMC::newGenVertexPtr();
    dZeroDecay->add_particle_in(dZero);
    dZeroDecay->add_particle_out(makeParticle(-321, 17., 1));
    dZeroDecay->add_particle_out(makeParticle(211, 17., 1));
    event.add_vertex(std::move(dZeroDecay));
    event.set_beam_particles(std::move(beam1), std::move(beam2));

    // The barcodes as the EVNT reader sets them.
    for (const auto& p : event.particles()) {
      HepMC::suggest_barcode(p, p->id());
    }
    for (const auto& v : event.vertices()) {
      HepMC::suggest_barcode(v, v->id());
    }
    HepMC::fillBarcodesAttribute(&event);
    assert(!hasSharedBarcodes(event));

    NewBarcodes barcodes;
    EvtGenBarcodes::findBarcodeRange(event, barcodes.maxParticle, barcodes.minVertex);
    assert(barcodes.maxParticle == 14);
    assert(barcodes.minVertex == -4);

    redecay(event, bPlus, {-421, 211, 111}, barcodes);
    assert(!hasSharedBarcodes(event));
    redecay(event, dZero, {-321, 211, 111}, barcodes);
    assert(!hasSharedBarcodes(event));
    assert(HepMC::barcode(bPlus->end_vertex()) == -5);
    assert(HepMC::barcode(dZero->end_vertex()) == -6);
    assert(barcodes.maxParticle == 20);

    MsgStream log(nullptr, "test");
    McEventCollectionCnv_p6 converter;
    McEventCollection_p6 persistent;
    converter.transToPers(&collection, &persistent, log);
    McEventCollection readBack;
    converter.persToTrans(&persistent, &readBack, log);
    const HepMC::GenEvent& event2 = *readBack.at(0);
    assert(event.particles().size() == event2.particles().size());
    assert(event.vertices().size() == event2.vertices().size());
    assert(decayGraph(event) == decayGraph(event2));
    // xAODTruthCnvAlg converts the particles in the barcode map, so it must hold all of them.
    const auto map = event2.attribute<HepMC::GenEventBarcodes>(HepMCStr::barcodes);
    assert(map);
    assert(map->barcode_to_particle_map().size() == event2.particles().size());
    assert(map->barcode_to_vertex_map().size() == event2.vertices().size());
    std::cout << "particles " << event2.particles().size() << ", vertices " << event2.vertices().size() << "\n";
  }

}

//coverity[root_function]
int main() {
  std::cout << "EvtGen_i/EvtGenBarcodes_test\n";
  ISvcLocator* pSvcLoc;
  if (!Athena_test::initGaudi("GeneratorObjectsTPCnv/GeneratorObjectsTPCnv_test.txt", pSvcLoc)) {
    std::cerr << "This test can not be run\n";
    return 0;
  }
  std::unique_ptr<SGTest::TestStore> store = SGTest::getTestStore();
  run(*store);
  return 0;
}
