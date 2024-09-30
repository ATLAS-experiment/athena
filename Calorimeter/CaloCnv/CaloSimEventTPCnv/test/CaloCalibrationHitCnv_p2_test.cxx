/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file CaloSimEventTPCnv/test/CaloCalibrationHitCnv_p2_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2015
 * @brief Regression tests.
 */

#undef NDEBUG
#include "CaloSimEventTPCnv/CaloCalibrationHitCnv_p2.h"
#include "TestTools/leakcheck.h"
#include "CaloSimEvent/CaloCalibrationHit.h"
#include "CxxUtils/checker_macros.h"
#include "GaudiKernel/MsgStream.h"
#include <cassert>
#include <iostream>


void test1 ATLAS_NOT_THREAD_SAFE ()
{
  std::cout << "test1\n";
  Athena_test::Leakcheck check;
  MsgStream log (0, "test");
  const int barcode{9876};
  const int uniqueID{HepMC::INVALID_PARTICLE_ID};
  CaloCalibrationHit trans1 (Identifier(1234),
                             45.5,
                             55.5,
                             65.5,
                             75.5,
                             barcode,
                             uniqueID);
  CaloCalibrationHitCnv_p2 cnv;
  CaloCalibrationHit_p2 pers;
  cnv.transToPers (&trans1, &pers, log);

  CaloCalibrationHit trans2;
  cnv.persToTrans (&pers, &trans2, log);

  assert (trans1.Equals (&trans2));
  for (int i=0; i<4; i++) {
    assert (trans1.energy(i) == trans2.energy(i));
  }
  assert (trans1.particleID() == trans2.particleID());
  assert (trans1.particleUID() == trans2.particleUID());
}


int main ATLAS_NOT_THREAD_SAFE ()
{
  test1();
  return 0;
}
