/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#undef NDEBUG
#include "TestTools/initGaudi.h"
#include "StoreGate/SegMemSvc.h"

#include <cassert>
#include <iostream>


void segmem_test () {
  ServiceHandle<SegMemSvc> p_sms{"SegMemSvc", "segmem_test"};
  assert ( p_sms.isValid() );

  int* p_int = new ( p_sms->allocate<int>(SegMemSvc::EVENT) ) int(1001);

  // cppcheck doesn't seem to understand placement new.
  // cppcheck-suppress uninitdata
  assert ( *p_int == 1001 );
}


int main() {
  ISvcLocator* svcLoc;
  Athena_test::initGaudi(svcLoc);
  segmem_test();
  std::cout << "*** SegMemSvc_test OK ***" << std::endl;
  return 0; 
}
