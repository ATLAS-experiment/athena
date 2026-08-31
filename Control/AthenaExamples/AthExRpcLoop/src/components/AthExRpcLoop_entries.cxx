/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "../RpcGateAlg.h"
#include "../RpcHiveEventLoopMgr.h"
#include "../RpcPackAlg.h"
#include "../RpcRequestAlg.h"

#include "../demo/DelayAlg.h"
#include "../demo/DemoClientAlgs.h"
#include "../demo/DemoDoublesAdapters.h"
#include "../demo/DemoIntsAdapters.h"
#include "../demo/FailAlg.h"
#include "../demo/OffsetAlg.h"
#include "../demo/RpcCondAlg.h"
#include "../demo/ScaleVectorAlg.h"
#include "../demo/SumAlg.h"

// The mechanism.
DECLARE_COMPONENT( AthExRpc::RpcHiveEventLoopMgr )
DECLARE_COMPONENT( AthExRpc::RpcGateAlg )
DECLARE_COMPONENT( AthExRpc::RpcPackAlg )
DECLARE_COMPONENT( AthExRpc::RpcRequestAlg )

// The demonstration fragment's payload algorithms: ordinary Athena algorithms
// with ordinary typed handles, none of which knows an RPC exists.
DECLARE_COMPONENT( AthExRpc::SumAlg )
DECLARE_COMPONENT( AthExRpc::OffsetAlg )
DECLARE_COMPONENT( AthExRpc::ScaleVectorAlg )
DECLARE_COMPONENT( AthExRpc::RpcCondAlg )
DECLARE_COMPONENT( AthExRpc::DelayAlg )
DECLARE_COMPONENT( AthExRpc::FailAlg )

// ...and the adapters that convert its own schema, which are the only place in
// the package that knows what a demonstration payload contains.
DECLARE_COMPONENT( AthExRpc::DemoIntsUnpackAlg )
DECLARE_COMPONENT( AthExRpc::DemoIntsPackAlg )
DECLARE_COMPONENT( AthExRpc::DemoDoublesUnpackAlg )
DECLARE_COMPONENT( AthExRpc::DemoDoublesPackAlg )
DECLARE_COMPONENT( AthExRpc::DemoNumbersAlg )
DECLARE_COMPONENT( AthExRpc::DemoCheckAlg )
