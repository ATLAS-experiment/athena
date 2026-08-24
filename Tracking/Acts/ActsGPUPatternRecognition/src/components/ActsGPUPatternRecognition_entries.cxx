/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "../DeviceTripletSeedingAlg.h"
#include "../DeviceGBTSSeedingAlg.h"

DECLARE_COMPONENT(ActsTrk::DeviceTripletSeedingAlg)
DECLARE_COMPONENT(ActsTrk::DeviceGBTSSeedingAlg)

#ifdef ACTSTRACK_HAVE_CUDA

#include "src/cuda/CUDASeedingAlgProviderTool.h"
DECLARE_COMPONENT(ActsTrk::CUDASeedingAlgProviderTool)

#endif