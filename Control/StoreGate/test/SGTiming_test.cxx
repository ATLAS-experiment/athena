/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#undef NDEBUG

#include <print>
#include <string>
#include <vector>
#include <chrono>
#include "TestTools/initGaudi.h"
#include "TestTools/SGassert.h"
#include "GaudiKernel/IHiveWhiteBoard.h"
#include "StoreGate/SGHiveMgrSvc.h"
#include "StoreGate/StoreGateSvc.h"
#include "StoreGate/SGtests.h"

#include "SGTiming_test_objs.h"
#include "SGTiming_test_def.inc"

int main() {
  std::println ("**** SGTimingTest BEGINS ****");

  ISvcLocator* pSvcLoc;
  if (!Athena_test::initGaudi("StoreGate/SGTiming_test.txt", pSvcLoc)) {
    return 1;
  }

  SmartIF<StoreGateSvc> pSG{pSvcLoc->service("StoreGateSvc")};
  assert( pSG.isValid() );

  std::chrono::time_point<std::chrono::high_resolution_clock> start, end;

  std::vector<std::chrono::duration<double>> vd_rec, vd_ret, vd_clr;
  
  for (size_t i=0; i<NITER; ++i) {
    
#   include "SGTiming_test_ptr.inc"
    
    start = std::chrono::high_resolution_clock::now();
#   include "SGTiming_test_rec.inc"
    end = std::chrono::high_resolution_clock::now();
    vd_rec.push_back(end-start);

    start = std::chrono::high_resolution_clock::now();
#   include "SGTiming_test_ret.inc"
    end = std::chrono::high_resolution_clock::now();
    vd_ret.push_back(end-start);
        
#   include "SGTiming_test_chk.inc"

    start = std::chrono::high_resolution_clock::now();
    pSG->clearStore().ignore();
    end = std::chrono::high_resolution_clock::now();
    vd_clr.push_back(end-start);

    std::println ("rec: {:8}  ret: {:8}  clr: {:8}",
                  std::chrono::duration_cast<std::chrono::microseconds>(vd_rec[i]).count(),
                  std::chrono::duration_cast<std::chrono::microseconds>(vd_ret[i]).count(),
                  std::chrono::duration_cast<std::chrono::microseconds>(vd_clr[i]).count());
  }

  unsigned int a_ret{0}, a_rec{0}, a_clr{0};
  for (size_t i=1; i<NITER; ++i) {
    a_rec += std::chrono::duration_cast<std::chrono::microseconds>(vd_rec[i]).count();
    a_ret += std::chrono::duration_cast<std::chrono::microseconds>(vd_ret[i]).count();
    a_clr += std::chrono::duration_cast<std::chrono::microseconds>(vd_clr[i]).count();
  }
  
  std::println ("===== averages  TYPES: {}  KEYS: {}  iter: {} ===============",
                NTYPES, NKEYS, NITER);

  std::print ("{}/{} ", NTYPES, NKEYS);
  std::println ("rec: {:6.2f}  ret: {:6.2f}  clr: {:6.2f}",
                float(a_rec) / ((NITER-1) * NTYPES * NKEYS),
                float(a_ret) / ((NITER-1) * NTYPES * NKEYS),
                float(a_clr) / ((NITER-1) * NTYPES * NKEYS));
 
  std::println ("**** SGTimingTest ENDS ****");

  return 0;

}
