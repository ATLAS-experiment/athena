/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/// Tests of the event-record helpers of HepMCHelpers.h, using small HepMC3 events built by hand.
#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

#include <cstdlib>
#include <iostream>
#include <set>
#include <deque>
#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenParticle.h"
#include "AtlasHepMC/GenVertex.h"
#include "TruthUtils/HepMCHelpers.h"

#define CHECK(cond) do { if (!(cond)) { std::cerr << __FILE__ << ":" << __LINE__ << " check failed: " #cond << std::endl; ++nfail; } } while (0)

namespace {
  int nfail = 0;

  const HepMC::FourVector someMomentum(0, 0, 10, 10);
}

// Status and PDG-ID predicates of HepMCHelpers.h on a beam -> Z -> mu mu, nu event with a simulation step.
void testStatusPredicates() {
  HepMC3::GenEvent evt(HepMC3::Units::GEV, HepMC3::Units::MM);
  auto beam = HepMC::newGenParticlePtr(someMomentum, 2212, 4);
  auto z = HepMC::newGenParticlePtr(someMomentum, 23, 2);
  auto mup = HepMC::newGenParticlePtr(someMomentum, -13, 2);                        // decays to a simulated-level muon with an end vertex
  auto mum = HepMC::newGenParticlePtr(someMomentum, 13, 1);
  auto nu = HepMC::newGenParticlePtr(someMomentum, 14, 1);
  auto simmu = HepMC::newGenParticlePtr(someMomentum, -13, HepMC::SIM_STATUS_INCREMENT + 1);     // stable in simulation
  auto simdec = HepMC::newGenParticlePtr(someMomentum, -13, HepMC::SIM_STATUS_INCREMENT + 2);    // decayed in simulation, no end vertex
  auto zero = HepMC::newGenParticlePtr(HepMC::FourVector::ZERO_VECTOR(), 22, 1);

  auto v0 = HepMC::newGenVertexPtr();
  v0->add_particle_in(beam); v0->add_particle_out(z);
  evt.add_vertex(v0);
  auto v1 = HepMC::newGenVertexPtr();
  v1->add_particle_in(z); v1->add_particle_out(mup); v1->add_particle_out(mum); v1->add_particle_out(nu);
  evt.add_vertex(v1);
  auto v2 = HepMC::newGenVertexPtr();
  v2->add_particle_in(mup); v2->add_particle_out(simmu);
  v2->set_status(HepMC::SIM_STATUS_THRESHOLD + 1);
  evt.add_vertex(v2);
  evt.add_particle(simdec); evt.add_particle(zero);

  CHECK(MC::isBeam(beam));
  CHECK(!MC::isBeam(z));
  CHECK(MC::isDecayed(z) && MC::isDecayed(mup));
  CHECK(MC::isStable(mum) && !MC::isStable(z));
  CHECK(MC::isPhysical(z) && MC::isPhysical(mum) && !MC::isPhysical(beam));
  CHECK(MC::isFinalState(mum));
  CHECK(!MC::isFinalState(mup));
  CHECK(MC::isGenStable(mum));
  CHECK(!MC::isGenStable(simmu));                // simulation particle
  CHECK(MC::isSimStable(simmu));
  CHECK(!MC::isSimStable(mum));
  CHECK(MC::isStable(simmu));                    // status is taken modulo SIM_STATUS_THRESHOLD
  CHECK(MC::isStableOrSimDecayed(mum));
  CHECK(MC::isStableOrSimDecayed(simdec));       // decayed without end vertex
  CHECK(MC::isStableOrSimDecayed(mup));          // decayed in a simulation vertex
  CHECK(!MC::isStableOrSimDecayed(z));           // decayed in a generator vertex

  CHECK(MC::isSpecialNonInteracting(nu));
  CHECK(MC::isSpecialNonInteracting(HepMC::newGenParticlePtr(someMomentum, -12, 1)));
  CHECK(MC::isSpecialNonInteracting(HepMC::newGenParticlePtr(someMomentum, 1000022, 1)));
  CHECK(MC::isSpecialNonInteracting(HepMC::newGenParticlePtr(someMomentum, 1000039, 1)));
  CHECK(MC::isSpecialNonInteracting(HepMC::newGenParticlePtr(someMomentum, 9000003, 1)));
  CHECK(!MC::isSpecialNonInteracting(mum));
  CHECK(!MC::isInteracting(nu));
  CHECK(MC::isInteracting(mum));
  CHECK(MC::isSimInteracting(mum));
  CHECK(!MC::isSimInteracting(nu));
  CHECK(MC::isChargedNonShowering(mum));
  CHECK(!MC::isChargedNonShowering(zero));
}

// Navigation helpers on  q qbar -> Z -> mu+ mu-  and  g g -> b bbar, B+ -> mu+ nu.
void testEventNavigation() {
  HepMC3::GenEvent evt(HepMC3::Units::GEV, HepMC3::Units::MM);
  auto q = HepMC::newGenParticlePtr(someMomentum, 2, 21), qb = HepMC::newGenParticlePtr(someMomentum, -2, 21);
  auto z = HepMC::newGenParticlePtr(someMomentum, 23, 2);
  auto mup = HepMC::newGenParticlePtr(someMomentum, -13, 1), mum = HepMC::newGenParticlePtr(someMomentum, 13, 1);
  auto g1 = HepMC::newGenParticlePtr(someMomentum, 21, 21), g2 = HepMC::newGenParticlePtr(someMomentum, 21, 21);
  auto b = HepMC::newGenParticlePtr(someMomentum, 5, 23), bb = HepMC::newGenParticlePtr(someMomentum, -5, 23);
  auto B = HepMC::newGenParticlePtr(someMomentum, 521, 2);
  auto mu_b = HepMC::newGenParticlePtr(someMomentum, -13, 1), nu_b = HepMC::newGenParticlePtr(someMomentum, 14, 1);

  auto vz = HepMC::newGenVertexPtr();
  vz->add_particle_in(q); vz->add_particle_in(qb); vz->add_particle_out(z);
  auto vd = HepMC::newGenVertexPtr();
  vd->add_particle_in(z); vd->add_particle_out(mup); vd->add_particle_out(mum);
  auto vg = HepMC::newGenVertexPtr();
  vg->add_particle_in(g1); vg->add_particle_in(g2); vg->add_particle_out(b); vg->add_particle_out(bb);
  auto vh = HepMC::newGenVertexPtr();         // hadronisation  b -> B+
  vh->add_particle_in(b); vh->add_particle_out(B);
  auto vb = HepMC::newGenVertexPtr();
  vb->add_particle_in(B); vb->add_particle_out(mu_b); vb->add_particle_out(nu_b);
  for (auto& v : {vz, vd, vg, vh, vb}) evt.add_vertex(v);

  // hard scattering vertices: two quarks/gluons in
  CHECK(MC::isHardScatteringVertex(vz));
  CHECK(MC::isHardScatteringVertex(vg));
  CHECK(MC::isHardScatteringVertex(vd));                // single-parent chain is followed back to the Z production vertex
  auto vlone = HepMC::newGenVertexPtr();   // photon conversion with no hard-scatter ancestry
  vlone->add_particle_in(HepMC::newGenParticlePtr(someMomentum, 22, 2)); vlone->add_particle_out(HepMC::newGenParticlePtr(someMomentum, 11, 1)); vlone->add_particle_out(HepMC::newGenParticlePtr(someMomentum, -11, 1));
  evt.add_vertex(vlone);
  CHECK(!MC::isHardScatteringVertex(vlone));
  CHECK(!MC::isHardScatteringVertex(HepMC::GenVertexPtr()));

  // ancestors and stable descendants
  std::set<HepMC::GenParticlePtr> anc;
  MC::findParticleAncestors(mup, anc);
  CHECK(anc.size() == 3 && anc.count(z) && anc.count(q) && anc.count(qb));
  std::set<HepMC::GenParticlePtr> stable;
  MC::findParticleStableDescendants(z, stable);
  CHECK(stable.size() == 2 && stable.count(mup) && stable.count(mum));
  stable.clear();
  MC::findParticleStableDescendants(b, stable);
  CHECK(stable.size() == 2 && stable.count(mu_b) && stable.count(nu_b));

  // mothers
  CHECK(MC::findMother(mup) == z);
  CHECK(MC::findMother(z) != nullptr);               // one of the incoming quarks
  CHECK(MC::findMother(q) == nullptr);               // no production vertex

  // simulated end vertex: follows 1->1 same-particle chains only
  CHECK(MC::findSimulatedEndVertex(z) == vd);
  CHECK(MC::findSimulatedEndVertex(mum) == nullptr);

  // hadron ancestry: the muon from B+ comes from a hadron, the Z muon does not
  HepMC::GenParticlePtr hadron;
  bool fromTau = false, fromBSM = false;
  CHECK(MC::isFromHadron(mu_b, hadron, fromTau, fromBSM));
  CHECK(hadron == B);
  CHECK(!fromTau && !fromBSM);
  HepMC::GenParticlePtr none;
  CHECK(!MC::isFromHadron(mum, none, fromTau, fromBSM));
  CHECK(none == nullptr);

  // unit scaling
  const double e0 = mup->momentum().e();
  MC::GeVToMeV(&evt);
  CHECK(std::abs(mup->momentum().e() - 1000 * e0) < 1e-6);
  MC::MeVToGeV(&evt);
  CHECK(std::abs(mup->momentum().e() - e0) < 1e-9);
}

int main() {
  testStatusPredicates();
  testEventNavigation();
  if (nfail) { std::cerr << nfail << " check(s) failed" << std::endl; return 1; }
  std::cout << "All checks passed" << std::endl;
  return 0;
}
