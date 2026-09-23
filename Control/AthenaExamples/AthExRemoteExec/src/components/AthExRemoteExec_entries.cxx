/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "../RemoteExecGateAlg.h"
#include "../RemoteExecHiveEventLoopMgr.h"
#include "../RemoteExecPackAlg.h"
#include "../RemoteExecRequestAlg.h"

#include "../demo/DelayAlg.h"
#include "../demo/DemoClientAlgs.h"
#include "../demo/DemoDoublesCodec.h"
#include "../demo/DemoIntsCodec.h"
#include "../demo/FailAlg.h"
#include "../demo/OffsetAlg.h"
#include "../demo/RemoteExecCondAlg.h"
#include "../demo/ScaleVectorAlg.h"
#include "../demo/SumAlg.h"

// The mechanism.
DECLARE_COMPONENT( AthExRemoteExec::RemoteExecHiveEventLoopMgr )
DECLARE_COMPONENT( AthExRemoteExec::RemoteExecGateAlg )
DECLARE_COMPONENT( AthExRemoteExec::RemoteExecPackAlg )
DECLARE_COMPONENT( AthExRemoteExec::RemoteExecRequestAlg )

// The demonstration fragment's payload algorithms: ordinary Athena algorithms
// with ordinary typed handles, none of which knows an RemoteExec exists.
DECLARE_COMPONENT( AthExRemoteExec::SumAlg )
DECLARE_COMPONENT( AthExRemoteExec::OffsetAlg )
DECLARE_COMPONENT( AthExRemoteExec::ScaleVectorAlg )
DECLARE_COMPONENT( AthExRemoteExec::RemoteExecCondAlg )
DECLARE_COMPONENT( AthExRemoteExec::DelayAlg )
DECLARE_COMPONENT( AthExRemoteExec::FailAlg )

// ...and the codecs that convert its own schema, which are the only place in
// the package that knows what a demonstration payload contains. They are the
// fragment's, not the framework's: this package implements no encoding of its
// own, and Gaudi's factory is the only registry the design has.
DECLARE_COMPONENT( AthExRemoteExec::DemoIntsCodec )
DECLARE_COMPONENT( AthExRemoteExec::DemoDoublesCodec )
DECLARE_COMPONENT( AthExRemoteExec::DemoNumbersAlg )
DECLARE_COMPONENT( AthExRemoteExec::DemoCheckAlg )
