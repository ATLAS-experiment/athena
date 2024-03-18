/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file InDetSimEventTPCnv/test/SiHitCollectionCnv_p3_test.cxx
 * @date Feb, 2018
 * @brief Tests for SiHitCollectionCnv_p3.
 */


#undef NDEBUG
#include "InDetSimEventTPCnv/InDetHits/SiHitCollectionCnv_p3.h"
#include "CxxUtils/checker_macros.h"
#include "TestTools/leakcheck.h"
#include <cassert>
#include <iostream>

#include "TruthUtils/MagicNumbers.h"
#include "GeneratorObjectsTPCnv/initMcEventCollection.h"
#include "AtlasHepMC/GenParticle.h"
#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/Operators.h"


void compare (const HepMcParticleLink& p1,
              const HepMcParticleLink& p2)
{
  assert ( p1.isValid() == p2.isValid() );
  assert ( HepMC::barcode(p1) == HepMC::barcode(p2) );
  assert ( p1.id() == p2.id() );
  assert ( p1.eventIndex() == p2.eventIndex() );
  assert ( p1.cptr() == p2.cptr() );
  assert ( p1 == p2 );
}

void compare (const SiHit& p1,
              const SiHit& p2)
{
  assert (p1.localStartPosition() == p2.localStartPosition());
  assert (p1.localEndPosition() == p2.localEndPosition());
  assert (p1.energyLoss() == p2.energyLoss());
  assert (p1.meanTime() == p2.meanTime());
  compare(p1.particleLink(), p2.particleLink());
  assert (p1.particleLink() == p2.particleLink());
  assert (p1.identify() == p2.identify());
}


void compare (const SiHitCollection& p1,
              const SiHitCollection& p2)
{
  //assert (p1.Name() == p2.Name());
  assert (p1.size() == p2.size());
  for (size_t i = 0; i < p1.size(); i++)
    compare (p1[i], p2[i]);
}


void testit (const SiHitCollection& trans1)
{
  MsgStream log (nullptr, "test");
  SiHitCollectionCnv_p3 cnv;
  SiHitCollection_p3 pers;
  cnv.transToPers (&trans1, &pers, log);
  SiHitCollection trans2;
  cnv.persToTrans (&pers, &trans2, log);

  compare (trans1, trans2);
}


void test1 ATLAS_NOT_THREAD_SAFE (std::vector<HepMC::GenParticlePtr>& genPartVector)
{
  std::cout << "test1\n";
  auto particle = genPartVector.at(0);
  // Create HepMcParticleLink outside of leak check.
  HepMcParticleLink dummyHMPL(HepMC::uniqueID(particle),particle->parent_event()->event_number(), HepMcParticleLink::IS_EVENTNUM, HepMcParticleLink::IS_ID);
  assert(dummyHMPL.cptr()==particle);
  // Create DVL info outside of leak check.
  SiHitCollection dum ("coll");
  Athena_test::Leakcheck check;

  SiHitCollection trans1 ("coll");
  for (int i=0; i < 10; i++) {
    auto pGenParticle = genPartVector.at(i);
    HepMcParticleLink trkLink(HepMC::uniqueID(pGenParticle),pGenParticle->parent_event()->event_number(), HepMcParticleLink::IS_EVENTNUM, HepMcParticleLink::IS_ID);
    const double angle = i*0.2*M_PI;
    std::vector< HepGeom::Point3D<double> > stepPoints(11);
    for (int j=0; j<11; ++j) {
      const double jd(j);
      const double r(30.+110.*jd);
      stepPoints.emplace_back(r*std::cos(angle),
                              r*std::sin(angle),
                              350.*jd);
    }
    const int o = i*100;
    trans1.Emplace (stepPoints.at(i), stepPoints.at(i+1),
                    16.5+o,
                    17.5+o,
                    trkLink,
                    19+o);

  }

  testit (trans1);
}


int main ATLAS_NOT_THREAD_SAFE ()
{
  ISvcLocator* pSvcLoc = nullptr;
  std::vector<HepMC::GenParticlePtr> genPartVector;
  if (!Athena_test::initMcEventCollection(pSvcLoc, genPartVector)) {
    std::cerr << "This test can not be run" << std::endl;
    return 0;
  }

  test1(genPartVector);
  return 0;
}
