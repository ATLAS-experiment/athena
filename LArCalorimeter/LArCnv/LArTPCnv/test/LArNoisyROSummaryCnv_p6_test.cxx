/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// $Id$
/**
 * @file LArTPCnv/test/LArNoisyROSSummaryCnv_p6_test.cxx
 * @date Mar, 2024
 * @brief Tests for LArNoisyROSSummaryCnv_p6.
 */


#undef NDEBUG
#include "LArTPCnv/LArNoisyROSummaryCnv_p6.h"
#include <cassert>
#include <iostream>


void compare (const LArNoisyROSummary& p1,
              const LArNoisyROSummary& p2)
{
  assert (p1.get_noisy_febs() == p2.get_noisy_febs());
  assert (p1.get_noisy_preamps() == p2.get_noisy_preamps());
  assert (p1.BadFEBFlaggedPartitions() == p2.BadFEBFlaggedPartitions());
  assert (p1.BadFEB_WFlaggedPartitions() == p2.BadFEB_WFlaggedPartitions());
  assert (p1.SatMediumFlaggedPartitions() == p2.SatMediumFlaggedPartitions());
  assert (p1.SatTightFlaggedPartitions() == p2.SatTightFlaggedPartitions());
  assert (p1.MNBLooseFlaggedPartitions() == p2.MNBLooseFlaggedPartitions());
  assert (p1.MNBTightFlaggedPartitions() == p2.MNBTightFlaggedPartitions());
  assert (p1.MNBTight_PsVetoFlaggedPartitions() == p2.MNBTight_PsVetoFlaggedPartitions());
  assert (p1.get_MNBTight_febs() == p2.get_MNBTight_febs());
  assert (p1.get_MNBLoose_febs() == p2.get_MNBLoose_febs());
  assert (p1.get_noisy_hvlines() == p2.get_noisy_hvlines());
  assert (p1.HVlineFlaggedPartitions() == p2.HVlineFlaggedPartitions());
}


void testit (const LArNoisyROSummary& trans1)
{
  MsgStream log (0, "test");
  LArNoisyROSummaryCnv_p6 cnv;
  LArNoisyROSummary_p6 pers;
  cnv.transToPers (&trans1, &pers, log);
  LArNoisyROSummary trans2;
  cnv.persToTrans (&pers, &trans2, log);

  compare (trans1, trans2);
}


void test6()
{
  std::cout << "test6\n";

  LArNoisyROSummary trans1;
  trans1.add_noisy_feb (HWIdentifier (0x1234));
  trans1.add_noisy_feb (HWIdentifier (0x1235));
  trans1.add_noisy_feb (HWIdentifier (0x1236));
  trans1.add_noisy_preamp (HWIdentifier (0x2134), 10);
  trans1.add_noisy_preamp (HWIdentifier (0x2135), 11);
  trans1.add_noisy_hvline (HWIdentifier (0x2236));
  trans1.add_noisy_hvline (HWIdentifier (0x2235));
  trans1.add_noisy_hvline (HWIdentifier (0x2234));
  trans1.SetBadFEBFlaggedPartitions (0x12);
  trans1.SetBadFEB_WFlaggedPartitions (0x23);
  trans1.SetSatMediumFlaggedPartitions (0x34);
  trans1.SetSatTightFlaggedPartitions (0x45);
  trans1.SetMNBTightFlaggedPartitions (0x56);
  trans1.SetMNBLooseFlaggedPartitions (0x67);
  trans1.SetMNBTight_PsVetoFlaggedPartitions (0x48);
  trans1.add_MNBTight_feb(HWIdentifier (0xF1234));
  trans1.add_MNBTight_feb(HWIdentifier (0xF1235));
  trans1.add_MNBLoose_feb(HWIdentifier (0xF4321));
  trans1.add_MNBLoose_feb(HWIdentifier (0xF4322));
  trans1.SetBadHVlinesPartitions (0x21);
    
  testit (trans1);
}


int main()
{
  test6();
  return 0;
}
