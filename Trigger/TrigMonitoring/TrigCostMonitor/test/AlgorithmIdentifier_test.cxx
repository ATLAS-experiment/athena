/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "StoreGate/StoreGateSvc.h"
#include "AthenaKernel/errorcheck.h"
#include "AthenaKernel/ExtendedEventContext.h"
#include "GaudiKernel/EventContext.h"
#include "SGTools/TestStore.h"
#include "TestTools/initGaudi.h"
#include "TestTools/expect.h"

#include "TrigCostMonitor/AlgorithmIdentifier.h"

/// @brief Unit test for AlgorithmIdentifier class
///
int main ATLAS_NOT_THREAD_SAFE () {

  errorcheck::ReportMessage::hideFunctionNames (true);

  // initialize Gaudi, SG
  ISvcLocator* pSvcLoc;
  Athena_test::initGaudi(pSvcLoc); 
  SmartIF<StoreGateSvc> pSG{pSvcLoc->service("StoreGateSvc")};
  assert( pSG );

  // Create a context
  IProxyDict* xdict = &*pSG;
  xdict = pSG->hiveProxyDict();
  EventContext ctx(0,0);
  ctx.setExtension( Atlas::ExtendedEventContext(xdict) );

  // Create a log
  MsgStream log(nullptr, "AlgorithmIdentifier");
  log.setLevel( MSG::DEBUG );

  // Test slot override
  AlgorithmIdentifier ai = AlgorithmIdentifierMaker::make(ctx, "ALG_A", log, 10);

  ai.dump(log);

  // Test hash collision. These two strings are known to collide.
  AlgorithmIdentifier collision_a = AlgorithmIdentifierMaker::make(ctx, "APP_HLT:HLTMPPU-36:HLT-36:tpu-rack-73:pc-tdq-tpu-73012-30", log);
  AlgorithmIdentifier collision_b = AlgorithmIdentifierMaker::make(ctx, "APP_HLT:HLTMPPU-36:HLT-36:tpu-rack-75:pc-tdq-tpu-75033-11", log);

  collision_a.dump(log);
  collision_b.dump(log);
}
