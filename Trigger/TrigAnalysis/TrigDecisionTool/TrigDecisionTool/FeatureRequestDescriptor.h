/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TrigDecisionTool_FeatureRequestDescriptor_h
#define TrigDecisionTool_FeatureRequestDescriptor_h

namespace {
  [[deprecated("Relocated header, please switch to TrigAnalysisHelpers/FeatureRequestDescriptor.h from TrigAnalysisHelpersLib")]]
  constexpr static int tdt_frd_deprecated = 0;
  constexpr static int do_not_use_tdt_frd = tdt_frd_deprecated; // Instantiate in order to trigger deprecated warning
}

#include "TrigAnalysisHelpers/FeatureRequestDescriptor.h"

#endif