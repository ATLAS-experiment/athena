/*
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 */

/**
 * @file AthenaBaseComps/test/AthMessaging_test.cxx
 * @author Frank Winklmeier
 * @date Aug, 2022
 * @brief Test AthMessaging (run with --perf to measure performance)
 */

#include "AthenaBaseComps/AthMessaging.h"
#include "AthenaKernel/getMessageSvc.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IMessageSvc.h"
#include "TestTools/initGaudi.h"

#include <chrono>
#include <iostream>
#include <cstring> //for strcmp


struct MyObj : public AthMessaging {
  /// Constructor using implicit MessageSvc retrieval
  MyObj()                    : AthMessaging("MyObj1")         {}
  /// Constructor with explicit MessageSvc
  MyObj(IMessageSvc* msgSvc) : AthMessaging(msgSvc, "MyObj2") {}
  /// Constructor with explicit output level
  MyObj(MSG::Level lvl)      : AthMessaging("MyObj3")         {
    setLevel(lvl);
  }

  void print()
  {
    ATH_MSG_DEBUG("Good morning");
    ATH_MSG_WARNING("Hello");
    ATH_MSG_INFO("World");
  }
};


bool test(IMessageSvc* msgSvc)
{
  MyObj obj1;
  obj1.print();

  MyObj obj2(msgSvc);
  obj2.print();

  MyObj obj3(MSG::DEBUG);
  obj3.print();

  // Checking a level < OutputLevel should not result in getMessageSvc warning
  MyObj obj4(MSG::WARNING);
  //typically used in "if (obj4.msgLvl(MSG::DEBUG)) {  ..debug messages ..}"
  bool outputExpected = obj4.msgLvl(MSG::DEBUG);
  return (not outputExpected);
}


void perftest (IMessageSvc* msgSvc, unsigned int ntry)
{
  using namespace std::chrono;

  auto start = high_resolution_clock::now();
  for (unsigned int i=0; i < ntry; i++) {
    if (msgSvc) MyObj obj(msgSvc);
    else        MyObj obj;
  }
  auto stop = high_resolution_clock::now();
  auto elapsed = duration_cast<nanoseconds>(stop - start);

  std::cout << "--- " << ntry << " times: " << elapsed.count()/1000 << " us"
            << " (" << elapsed.count() / ntry << " ns per call)" << std::endl;
}


int main (int argc, char** argv)
{
  const unsigned int ntry = 100000;
  bool doPerf = false;
  if (argc >= 2 && std::strcmp (argv[1], "--perf") == 0) {
    doPerf = true;
  }

  // --------------------------------------------------------------------------------
  std::cout << "--- Test without MessageSvc" << std::endl;
  test(nullptr);

  if (doPerf) {
    Athena::getMessageSvcQuiet = true;
    perftest(nullptr, ntry);
    Athena::getMessageSvcQuiet = false;
  }

  // --------------------------------------------------------------------------------
  ISvcLocator* svcLoc = nullptr;
  if (!Athena_test::initGaudi ("AthMessaging_test.txt", svcLoc)) return 1;

  SmartIF<IMessageSvc> msgSvc{svcLoc->service("MessageSvc")};
  if (!msgSvc) return 1;

  std::cout << "--- Test with MessageSvc" << std::endl;
  if (not test(msgSvc)) return 1;

  if (doPerf) {
    perftest(msgSvc, ntry);
  }

  return 0;
}
