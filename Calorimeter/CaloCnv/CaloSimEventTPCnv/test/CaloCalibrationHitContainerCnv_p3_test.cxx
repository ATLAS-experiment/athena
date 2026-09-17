/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file CaloSimEventTPCnv/test/CaloCalibrationHitContainerCnv_p3_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2015
 * @brief Regression tests.
 */

#undef NDEBUG
#include "CaloSimEventTPCnv/CaloCalibrationHitContainerCnv_p3.h"
#include "TestTools/leakcheck.h"
#include "CaloSimEvent/CaloCalibrationHit.h"
#include "CxxUtils/checker_macros.h"
#include "GaudiKernel/MsgStream.h"
#include <cassert>
#include <iostream>


void compare (const CaloCalibrationHit& trans1,
              const CaloCalibrationHit& trans2)
{
  assert (trans1.Equals (&trans2));
  for (int i=0; i<4; i++) {
    assert (trans1.energy(i) == trans2.energy(i));
  }
  assert (trans1.particleID() == trans2.particleID());
  //assert (trans1.particleUID() == trans2.particleUID()); // Need a valid McEventCollection to test valid GenParticle::id() here
}


void test1 ATLAS_NOT_THREAD_SAFE ()
{
  std::cout << "test1\n";
  MsgStream log (0, "test");

  // Don't run leakcheck over this; it writes log messages.
  CaloCalibrationHitContainer trans1;
  trans1.push_back (new CaloCalibrationHit (Identifier(987),
                                            45.5,
                                            55.5,
                                            65.5,
                                            75.5,
                                            333,
                                            HepMC::INVALID_PARTICLE_ID,
                                            HepMC::INVALID_PARTICLE_ID)); // Need a valid McEventCollection to test valid GenParticle::id() here
  trans1.push_back (new CaloCalibrationHit (Identifier(234),
                                            145.5,
                                            155.5,
                                            165.5,
                                            175.5,
                                            444,
                                            HepMC::INVALID_PARTICLE_ID,
                                            HepMC::INVALID_PARTICLE_ID)); // Need a valid McEventCollection to test valid GenParticle::id() here
  trans1.push_back (new CaloCalibrationHit (Identifier(345),
                                            245.5,
                                            255.5,
                                            265.5,
                                            275.5,
                                            555,
                                            HepMC::INVALID_PARTICLE_ID,
                                            HepMC::INVALID_PARTICLE_ID)); // Need a valid McEventCollection to test valid GenParticle::id() here

  Athena_test::Leakcheck check;

  CaloCalibrationHitContainerCnv_p3 cnv;
  CaloCalibrationHitContainer_p3 pers;
  cnv.transToPers (&trans1, &pers, log);

  CaloCalibrationHitContainer trans2;
  cnv.persToTrans (&pers, &trans2, log);

  assert (trans1.size() == trans2.size());
  compare (*trans1[0], *trans2[2]);
  compare (*trans1[1], *trans2[0]);
  compare (*trans1[2], *trans2[1]);
}


int main ATLAS_NOT_THREAD_SAFE ()
{
  test1();
  return 0;
}
