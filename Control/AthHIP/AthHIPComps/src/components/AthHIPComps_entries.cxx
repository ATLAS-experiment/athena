//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//

// Copy component(s).
#include "../AsyncCopyTool.h"
#include "../CopyTool.h"

// Memory resource component(s).
#include "../DeviceMemoryResourceTool.h"
#include "../HostMemoryResourceTool.h"
#include "../ManagedMemoryResourceTool.h"

// Stream component(s).
#include "../PerComponentStreamTool.h"
#include "../PerEventAndComponentStreamTool.h"
#include "../PerEventStreamSvc.h"
#include "../SingleStreamSvc.h"
#include "../StreamSvcAdaptorTool.h"

// Declare the component(s) to Gaudi.
DECLARE_COMPONENT(AthHIP::AsyncCopyTool)
DECLARE_COMPONENT(AthHIP::CopyTool)

DECLARE_COMPONENT(AthHIP::DeviceMemoryResourceTool)
DECLARE_COMPONENT(AthHIP::HostMemoryResourceTool)
DECLARE_COMPONENT(AthHIP::ManagedMemoryResourceTool)

DECLARE_COMPONENT(AthHIP::PerComponentStreamTool)
DECLARE_COMPONENT(AthHIP::PerEventAndComponentStreamTool)
DECLARE_COMPONENT(AthHIP::PerEventStreamSvc)
DECLARE_COMPONENT(AthHIP::SingleStreamSvc)
DECLARE_COMPONENT(AthHIP::StreamSvcAdaptorTool)
