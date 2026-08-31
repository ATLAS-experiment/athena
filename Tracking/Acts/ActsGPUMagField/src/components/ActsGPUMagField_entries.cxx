/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "../JSONDeviceMagFieldProviderSvc.h"

DECLARE_COMPONENT(ActsTrk::JSONDeviceMagFieldProviderSvc)

#ifdef ACTSTRACK_HAVE_CUDA

#include "../cuda/CUDAMagFieldProviderTool.h"
DECLARE_COMPONENT(ActsTrk::CUDAMagFieldProviderTool)

#endif
