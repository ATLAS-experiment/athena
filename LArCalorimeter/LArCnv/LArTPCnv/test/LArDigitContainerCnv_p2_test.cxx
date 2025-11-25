/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file LArTPCnv/test/LArDigitContainerCnv_p2_test.cxx
 * @brief Tests for LArDigitContainerCnv_p2.
 */


#undef NDEBUG
#include "LArTPCnv/LArDigitContainerCnv_p2.h"
#include "LArRawEvent/LArDigit.h"
#include "LArRawEvent/LArDigitContainer.h"
#include "LArIdentifier/LArOnlineID.h"
#include "IdDictParser/IdDictParser.h"
#include "TestTools/leakcheck.h"
#include "GaudiKernel/MsgStream.h"
#include "CxxUtils/checker_macros.h"
#include <cassert>
#include <iostream>


#include "AthAllocators/DataPool.h"
#include "AthAllocators/ArenaHeader.h"
void compare (const LArDigit& p1,
              const LArDigit& p2)
{
  assert (p1.channelID() == p2.channelID());
  assert (p1.gain() == p2.gain());
  assert (p1.samples() == p2.samples());
}


void compare (const LArDigitContainer& p1,
              const LArDigitContainer& p2)
{
  assert (p1.size() == p2.size());
  for (size_t i=0; i < p1.size(); i++) {
    compare (*p1[i], *p2[i]);
  }
}


void testit (const LArDigitContainer& trans1,
	     const LArOnlineID* idHelper)
{
  MsgStream log (0, "test");
  LArDigitContainerCnv_p2 cnv(idHelper);
  LArDigitContainer_p2 pers;
  cnv.transToPers (&trans1, &pers, log);
  LArDigitContainer trans2;
  cnv.persToTrans (&pers, &trans2, log);

  compare (trans1, trans2);
}


void test1 ATLAS_NOT_THREAD_SAFE ()
{
  std::cout << "test1\n";  

  IdDictParser parser;
  parser.register_external_entity ("LArCalorimeter", "IdDictParser/IdDictLArCalorimeter_DC3-05-Comm-01.xml");
  IdDictMgr& idd = parser.parse ("IdDictParser/ATLAS_IDS.xml");



  // create LArOnlineID helper to be passed to converter
  std::unique_ptr<LArOnlineID> idHelper = std::make_unique<LArOnlineID>();
  idHelper->setLevel(MSG::DEBUG);
  assert (idHelper->initialize_from_dictionary (idd) == 0);
  
  {
    // Need to instantiate an instance of this before Leakcheck,
    // to get the allocator created.  But the instance will also
    // hold a lock on the allocator, so can't leave this live
    // or we'll deadlock.
    DataPool<LArDigit> pooldum;
  }

 
  Athena_test::Leakcheck check;

  CaloGain::CaloGain gains[CaloGain::LARNGAIN] =
    {CaloGain::LARHIGHGAIN, CaloGain::LARMEDIUMGAIN, CaloGain::LARLOWGAIN};

  LArDigitContainer trans;
  for (int i=0; i < 100; i++) {
    trans.push_back (new LArDigit (idHelper->channel_Id(IdentifierHash(i)),
				   gains[i%CaloGain::LARNGAIN],
				   std::vector<short> {
				     (short)(1+i),
				       (short)(2+i),
				       (short)(3+i)}));
  }

  testit (trans, idHelper.get());

  //We use a DataPool/custom allocator. We need to call erase so as
  //to make sure we clean up before we exit.
  SG::ArenaHeader::defaultHeader()->allocator(0)->erase();
}


int main ATLAS_NOT_THREAD_SAFE ()
{
  try{
    test1();
  } catch(std::exception & e){
    std::cerr<<"Exception "<<e.what()<<" in LArDigitContainerCnv_p2_test"<<std::endl;
    return 1;
  }
  return 0;
}
