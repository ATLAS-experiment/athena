/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/// Tests of the status-scheme and identity helpers of MagicNumbers.h, using small HepMC3 events built by hand.
#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

#include <cstdlib>
#include <iostream>
#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenParticle.h"
#include "AtlasHepMC/GenVertex.h"
#include "TruthUtils/MagicNumbers.h"

#define CHECK(cond) do { if (!(cond)) { std::cerr << __FILE__ << ":" << __LINE__ << " check failed: " #cond << std::endl; ++nfail; } } while (0)

namespace {
  int nfail = 0;

  const HepMC::FourVector someMomentum(0, 0, 10, 10);
}

// Status encoding: generations, simulation flags and conversion between the old and the new scheme.
void testStatusScheme() {
  auto g = HepMC::newGenParticlePtr(someMomentum, 11, 1);
  auto sec = HepMC::newGenParticlePtr(someMomentum, 11, HepMC::SIM_STATUS_THRESHOLD + 1);                          // simulation secondary, generation 0
  auto regen = HepMC::newGenParticlePtr(someMomentum, 11, 2 * HepMC::SIM_STATUS_INCREMENT + 1);                    // generator particle surviving 2 interactions
  auto regensec = HepMC::newGenParticlePtr(someMomentum, 11, HepMC::SIM_STATUS_INCREMENT + HepMC::SIM_STATUS_THRESHOLD + 1);

  CHECK(!HepMC::is_simulation_particle(g));
  CHECK(HepMC::is_simulation_particle(sec));
  CHECK(HepMC::is_simulation_particle(regen));
  CHECK(HepMC::generations(g) == 0);
  CHECK(HepMC::generations(sec) == 0);
  CHECK(HepMC::generations(regen) == 2);
  CHECK(HepMC::generations(regensec) == 1);
  CHECK(!HepMC::StatusBased::is_sim_secondary(g));
  CHECK(HepMC::StatusBased::is_sim_secondary(sec));
  CHECK(!HepMC::StatusBased::is_sim_secondary(regen));
  CHECK(HepMC::StatusBased::is_sim_secondary(regensec));

  // Old -> new -> old for particles: generator, secondary, regenerated generator particle
  CHECK(HepMC::new_particle_status_from_old(1, 5) == 1);
  CHECK(HepMC::new_particle_status_from_old(1, HepMC::SIM_BARCODE_THRESHOLD + 1) == HepMC::SIM_STATUS_THRESHOLD + 1);
  CHECK(HepMC::new_particle_status_from_old(1, 2 * HepMC::SIM_REGENERATION_INCREMENT + 5) == 2 * HepMC::SIM_STATUS_INCREMENT + 1);
  CHECK(HepMC::new_particle_status_from_old(2, HepMC::SIM_REGENERATION_INCREMENT + HepMC::SIM_BARCODE_THRESHOLD + 7) == HepMC::SIM_STATUS_INCREMENT + HepMC::SIM_STATUS_THRESHOLD + 2);
  for (int s : {1, 2, 4}) {
    for (int bc : {5, HepMC::SIM_BARCODE_THRESHOLD + 3, HepMC::SIM_REGENERATION_INCREMENT + 5}) {
      CHECK(HepMC::old_particle_status_from_new(HepMC::new_particle_status_from_old(s, bc)) % HepMC::SIM_STATUS_INCREMENT == s);
    }
  }
  CHECK(HepMC::old_particle_status_from_new(HepMC::SIM_STATUS_THRESHOLD + 1) == 1);

  // Vertices (non-zero old status)
  CHECK(HepMC::new_vertex_status_from_old(1, -5) == 1);
  CHECK(HepMC::new_vertex_status_from_old(1, -HepMC::SIM_BARCODE_THRESHOLD - 1) == HepMC::SIM_STATUS_THRESHOLD + 1);
  auto gv = HepMC::newGenVertexPtr(); gv->set_status(1);
  auto sv = HepMC::newGenVertexPtr(); sv->set_status(HepMC::new_vertex_status_from_old(1, -HepMC::SIM_BARCODE_THRESHOLD - 1));
  CHECK(!HepMC::is_simulation_vertex(gv));
  CHECK(HepMC::is_simulation_vertex(sv));
  CHECK(HepMC::old_vertex_status_from_new(sv->status()) == 1);
  CHECK(HepMC::old_vertex_status_from_new(gv->status()) == 1);
}

// Identity helpers: uniqueID, same particle/vertex, truth-link vetoes, same-generator-particle shortcuts.
void testIdentityHelpers() {
  HepMC3::GenEvent evt(HepMC3::Units::GEV, HepMC3::Units::MM);
  auto a = HepMC::newGenParticlePtr(someMomentum, 22, 1);
  auto b = HepMC::newGenParticlePtr(someMomentum, 22, 1);
  auto sim = HepMC::newGenParticlePtr(someMomentum, 22, HepMC::SIM_STATUS_THRESHOLD + 1);
  auto v = HepMC::newGenVertexPtr();
  v->add_particle_out(a); v->add_particle_out(b); v->add_particle_out(sim);
  evt.add_vertex(v);

  CHECK(HepMC::uniqueID(a) == a->id());
  CHECK(HepMC::uniqueID(a) != HepMC::uniqueID(b));
  CHECK(HepMC::uniqueID(7) == 7);
  CHECK(HepMC::status(sim) == HepMC::SIM_STATUS_THRESHOLD + 1);
  CHECK(HepMC::is_same_particle(a, a));
  CHECK(!HepMC::is_same_particle(a, b));
  CHECK(HepMC::is_same_vertex(v, v));

  CHECK(HepMC::no_truth_link(0));
  CHECK(!HepMC::no_truth_link(a));
  CHECK(HepMC::ignoreTruthLink(0, false));
  CHECK(HepMC::ignoreTruthLink(HepMC::SUPPRESSED_PILEUP_BARCODE, true));
  CHECK(!HepMC::ignoreTruthLink(HepMC::SUPPRESSED_PILEUP_BARCODE, false));
  CHECK(HepMC::is_truth_suppressed_pileup(HepMC::SUPPRESSED_PILEUP_BARCODE));
  CHECK(!HepMC::is_truth_suppressed_pileup(5));

  CHECK(HepMC::is_same_generator_particle(a, a));
  CHECK(HepMC::is_sim_descendant(a, a));
  CHECK(!HepMC::is_same_generator_particle(a, b));  // same generation, different IDs
  CHECK(!HepMC::is_sim_descendant(a, b));

  // History of a particle without regenerated parents is just the particle itself
  auto h = HepMC::simulation_history(a, -1);
  CHECK(h.size() == 1 && h.front() == a->id());
  h = HepMC::simulation_history(a, 1);
  CHECK(h.size() == 1 && h.front() == a->id());
}

int main() {
  testStatusScheme();
  testIdentityHelpers();
  if (nfail) { std::cerr << nfail << " check(s) failed" << std::endl; return 1; }
  std::cout << "All checks passed" << std::endl;
  return 0;
}
