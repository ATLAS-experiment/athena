/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGCOMPOSITEUTILS_ACTIVESTATE_H
#define TRIGCOMPOSITEUTILS_ACTIVESTATE_H

#include "xAODTrigger/TrigComposite.h"
#include "AthLinks/ElementLink.h"
#include "AsgMessaging/StatusCode.h"

#include <set>

namespace TrigCompositeUtils {
  /**
   * @brief Additional information returned by the TrigerDecisionTool's feature retrieval, contained within the LinkInfo.
   **/
  enum ActiveState {
    UNSET, //!< Default property of state. Indicates that the creator of the LinkInfo did not supply this information
    ACTIVE, //!< The link was still active for one-or-more of the HLT Chains requested in the TDT
    INACTIVE //!< The link was inactive for all of the HLT Chains requested in the TDT. I.e. the object was rejected by these chains.
  };
} //> end namespace TrigCompositeUtils

#endif //> !TRIGCOMPOSITEUTILS_ACTIVESTATE_H