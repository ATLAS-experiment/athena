//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//

// Local include(s).
#include "../KernelRunnerSvc.h"
#include "../StreamPoolSvc.h"
#include "../GPUSystemInfoSvc.h"
#include "../HostMemoryResourceTool.h"
#include "../DeviceMemoryResourceTool.h"
#include "../ManagedMemoryResourceTool.h"

// Declare the component(s) to Gaudi.
DECLARE_COMPONENT( AthCUDA::KernelRunnerSvc )
DECLARE_COMPONENT( AthCUDA::StreamPoolSvc )
DECLARE_COMPONENT( AthCUDA::GPUSystemInfoSvc )
DECLARE_COMPONENT( AthCUDA::HostMemoryResourceTool )
DECLARE_COMPONENT( AthCUDA::DeviceMemoryResourceTool )
DECLARE_COMPONENT( AthCUDA::ManagedMemoryResourceTool )
