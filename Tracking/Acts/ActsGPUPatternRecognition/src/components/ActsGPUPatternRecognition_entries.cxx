/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "../DeviceTripletSeedingAlg.h"
#include "../DeviceGBTSSeedingAlg.h"
#include "../DeviceTrkParamEstimationAlg.h"

DECLARE_COMPONENT(ActsTrk::DeviceTripletSeedingAlg)
DECLARE_COMPONENT(ActsTrk::DeviceGBTSSeedingAlg)
DECLARE_COMPONENT(ActsTrk::DeviceTrkParamEstimationAlg)

#ifdef ACTSTRACK_HAVE_CUDA

#include "src/cuda/CUDASeedingAlgProviderTool.h"
DECLARE_COMPONENT(ActsTrk::CUDASeedingAlgProviderTool)

#include "src/cuda/CUDATrkParamAlgProviderTool.h"
DECLARE_COMPONENT(ActsTrk::CUDATrkParamAlgProviderTool)

#endif