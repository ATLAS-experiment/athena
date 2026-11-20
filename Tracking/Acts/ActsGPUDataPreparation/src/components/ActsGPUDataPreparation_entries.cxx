/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "../DeviceClusterizationAlg.h"
#include "../DeviceSPFormationAlg.h"

DECLARE_COMPONENT(ActsTrk::DeviceClusterizationAlg)
DECLARE_COMPONENT(ActsTrk::DeviceSPFormationAlg)

#ifdef ACTSTRACK_HAVE_CUDA

#include "src/cuda/CUDAClusterizationAlgProviderTool.h"
DECLARE_COMPONENT(ActsTrk::CUDAClusterizationAlgProviderTool)

#include "src/cuda/CUDASPFormationAlgProviderTool.h"
DECLARE_COMPONENT(ActsTrk::CUDASPFormationAlgProviderTool)

#include "src/cuda/CUDAStripSPFormationAlgProviderTool.h"
DECLARE_COMPONENT(ActsTrk::CUDAStripSPFormationAlgProviderTool)

#endif  // ACTSTRACK_HAVE_CUDA

#ifdef ACTSTRACK_HAVE_HIP

#include "src/hip/HIPClusterizationAlgProviderTool.h"
DECLARE_COMPONENT(ActsTrk::HIPClusterizationAlgProviderTool)

#include "src/hip/HIPSPFormationAlgProviderTool.h"
DECLARE_COMPONENT(ActsTrk::HIPSPFormationAlgProviderTool)

#endif  // ACTSTRACK_HAVE_HIP

#include "src/DeviceStripSPFormationAlg.h"
DECLARE_COMPONENT(ActsTrk::DeviceStripSPFormationAlg)

#include "src/StripSpacepointInputsCondAlg.h"
DECLARE_COMPONENT(ActsTrk::StripSpacepointInputsCondAlg)
