// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

// Local include(s).
#include "AthOnnxComps/OnnxRuntimeSvc.h"
#include "AthOnnxComps/OnnxRuntimeSessionToolCPU.h"
#include "AthOnnxComps/OnnxRuntimeSessionToolCUDA.h"
#include "AthOnnxComps/OnnxRuntimeInferenceTool.h"

// Declare the package's components.
DECLARE_COMPONENT( AthOnnx::OnnxRuntimeSvc )
DECLARE_COMPONENT( AthOnnx::OnnxRuntimeSessionToolCPU )
DECLARE_COMPONENT( AthOnnx::OnnxRuntimeSessionToolCUDA )
DECLARE_COMPONENT( AthOnnx::OnnxRuntimeInferenceTool )
