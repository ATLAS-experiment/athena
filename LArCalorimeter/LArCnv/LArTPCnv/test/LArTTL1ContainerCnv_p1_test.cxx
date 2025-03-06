/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// $Id$
/**
 * @file LArTPCnv/test/LArTTL1ContainerCnv_p1_test.cxx
 * @author Alaettin Serhan Mete <amete@anl.gov>
 * @date Mar, 2016
 * @brief Tests for LArTTL1ContainerCnv_p1.
 */


#undef NDEBUG
#include "LArTPCnv/LArTTL1ContainerCnv_p1.h"
#include "LArRawEvent/LArTTL1.h"
#include "LArRawEvent/LArTTL1Container.h"
#include <cassert>
#include <iostream>


void compare (const LArTTL1& p1,
              const LArTTL1& p2)
{
  assert (p1.ttOfflineID() == p2.ttOfflineID());
  assert (p1.samples() == p2.samples());
}


void compare (const LArTTL1Container& p1,
              const LArTTL1Container& p2)
{
  assert (p1.size() == p2.size());
  for (size_t i=0; i < p1.size(); i++)
    compare (*p1[i], *p2[i]);
}


void testit (const LArTTL1Container& trans1)
{
  MsgStream log (0, "test");
  LArTTL1ContainerCnv_p1 cnv;
  LArTTL1Container_p1 pers;
  cnv.transToPers (&trans1, &pers, log);
  LArTTL1Container trans2;
  cnv.persToTrans (&pers, &trans2, log);

  compare (trans1, trans2);
}


void test1()
{
  LArTTL1Container trans1;
  for (int i=0; i < 10; i++) {
    short o = i*100;
    trans1.push_back (new LArTTL1 (HWIdentifier (1234+o),
                                    Identifier (5678+o),
                                    std::vector<float> {
                                        (float)(1.f+o),
                                        (float)(2.f+o),
                                        (float)(3.f+o),
                                        (float)(4.f+o),
                                        (float)(5.f+o),
                                        (float)(6.f+o),
                                        (float)(7.f+o),
                                        }));
  }
    
  testit (trans1);
}


int main()
{
  test1();
  return 0;
}
