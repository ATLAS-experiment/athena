/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file MuonSimEventTPCnv/test/TGCSimHitCollectionCnv_p2_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Mar, 2016
 * @brief Tests for TGCSimHitCollectionCnv_p2.
 */


#undef NDEBUG
#include "MuonSimEventTPCnv/TGCSimHitCollectionCnv_p2.h"
#include "CxxUtils/checker_macros.h"
#include "TestTools/leakcheck.h"
#include <cassert>
#include <iostream>

#include "GeneratorObjectsTPCnv/initMcEventCollection.h"
#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenParticle.h"
#include "AtlasHepMC/Operators.h"


void compare (const HepMcParticleLink& p1,
              const HepMcParticleLink& p2)
{
  assert ( p1.isValid() == p2.isValid() );
  assert ( HepMC::barcode(p1) == HepMC::barcode(p2) );
  assert ( p1.eventIndex() == p2.eventIndex() );
  assert ( p1.cptr() == p2.cptr() );
  assert ( p1 == p2 );
}

void compare (const TGCSimHit& p1,
              const TGCSimHit& p2)
{
  assert (p1.TGCid() == p2.TGCid());
  assert (p1.globalTime() == p2.globalTime());
  assert (p1.localPosition() == p2.localPosition());
  assert (p1.localDireCos() == p2.localDireCos());
  compare(p1.particleLink(), p2.particleLink());
  assert (p1.particleLink() == p2.particleLink());
  assert (p1.energyDeposit() == p2.energyDeposit());
  assert (p1.stepLength() == p2.stepLength());
  //assert (p1.particleEncoding() == p2.particleEncoding());
  //assert (p1.kineticEnergy() == p2.kineticEnergy());
}


void compare (const TGCSimHitCollection& p1,
              const TGCSimHitCollection& p2)
{
  //assert (p1.Name() == p2.Name());
  assert (p1.size() == p2.size());
  for (size_t i=0; i < p1.size(); i++)
    compare (p1[i], p2[i]);
}


void testit (const TGCSimHitCollection& trans1)
{
  MsgStream log (nullptr, "test");
  TGCSimHitCollectionCnv_p2 cnv;
  Muon::TGCSimHitCollection_p2 pers;
  cnv.transToPers (&trans1, &pers, log);
  TGCSimHitCollection trans2;
  cnv.persToTrans (&pers, &trans2, log);

  compare (trans1, trans2);
}


void test1 ATLAS_NOT_THREAD_SAFE (std::vector<HepMC::GenParticlePtr> genPartVector)
{
  std::cout << "test1\n";
  auto particle = genPartVector.at(0);
  // Create HepMcParticleLink outside of leak check.
  HepMcParticleLink dummyHMPL(HepMC::barcode(particle),particle->parent_event()->event_number(),HepMcParticleLink::IS_EVENTNUM,HepMcParticleLink::IS_BARCODE); // FIXME barcode-based
  assert(dummyHMPL.cptr()==particle);
  // Create DVL info outside of leak check.
  TGCSimHitCollection dum ("coll");
  Athena_test::Leakcheck check;

  TGCSimHitCollection trans1 ("coll");
  for (int i=0; i < 10; i++) {
    auto pGenParticle = genPartVector.at(i);
    HepMcParticleLink trkLink(HepMC::barcode(pGenParticle),pGenParticle->parent_event()->event_number(),HepMcParticleLink::IS_EVENTNUM,HepMcParticleLink::IS_BARCODE); // FIXME barcode-based
    trans1.Emplace (123, 10.5,
                    Amg::Vector3D (12.5, 13.5, 14.5),
                    Amg::Vector3D (16.5, 17.5, 18.5),
                    trkLink, 20.5, 21.5, pGenParticle->pdg_id(), 23.5);
  }

  testit (trans1);
}


int main ATLAS_NOT_THREAD_SAFE ()
{
  ISvcLocator* pSvcLoc = nullptr;
  std::vector<HepMC::GenParticlePtr> genPartVector;
  if (!Athena_test::initMcEventCollection(pSvcLoc,genPartVector)) {
    std::cerr << "This test can not be run" << std::endl;
    return 0;
  }

  test1(genPartVector);
  return 0;
}
