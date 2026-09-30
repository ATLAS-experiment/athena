/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGGER_ANALYSIS_ALGORITHMS__TRIG_CHAIN_NAME_HELPERS_H
#define TRIGGER_ANALYSIS_ALGORITHMS__TRIG_CHAIN_NAME_HELPERS_H

#include <algorithm>
#include <string>

namespace CP
{
  /// \brief convert a trigger chain name into a string usable in
  /// decoration/branch names (`.` -> `p`, `-` -> `_`)
  ///
  /// Keep in sync with `sanitizeTriggerChainName` in
  /// `python/TriggerAnalysisConfig.py`.
  inline std::string sanitizeTriggerChainName (std::string chain)
  {
    std::replace (chain.begin(), chain.end(), '.', 'p');
    std::replace (chain.begin(), chain.end(), '-', '_');
    return chain;
  }
}

#endif
