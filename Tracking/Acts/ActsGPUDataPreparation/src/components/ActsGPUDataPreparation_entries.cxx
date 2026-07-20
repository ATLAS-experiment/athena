/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "../DeviceClusterizationAlg.h"

DECLARE_COMPONENT(ActsTrk::DeviceClusterizationAlg)

#ifdef ACTSTRACK_HAVE_CUDA
#include "src/cuda/CUDAClusterizationAlgProviderTool.h"
DECLARE_COMPONENT(ActsTrk::CUDAClusterizationAlgProviderTool)
#endif