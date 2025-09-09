/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGCOMPOSITEUTILS_LINKINFO_H
#define TRIGCOMPOSITEUTILS_LINKINFO_H

#include "xAODTrigger/TrigComposite.h"
#include "AthLinks/ElementLink.h"
#include "AsgMessaging/StatusCode.h"

#include <optional>
#include <unordered_set>

namespace TrigCompositeUtils {
  /**
   * @brief Additional information returned by the TrigerDecisionTool's feature retrieval, contained within the LinkInfo.
   **/
  enum ActiveState {
    UNSET, //!< Default property of state. Indicates that the creator of the LinkInfo did not supply this information
    ACTIVE, //!< The link was still active for one-or-more of the HLT Chains requested in the TDT
    INACTIVE //!< The link was inactive for all of the HLT Chains requested in the TDT. I.e. the object was rejected by these chains.
  };

  /**
   * @brief Helper to keep a Decision object, ElementLink and ActiveState (with respect to some requested ChainGroup) linked together (for convenience)
   **/
  template<typename T>
  struct LinkInfo {
    LinkInfo() = default;
    LinkInfo(const Decision* s, const ElementLink<T>& l, ActiveState as = ActiveState::UNSET)
      : source{s}, link{l}, state{as} {
      if (s) decisions.emplace(s->decisions().begin(), s->decisions().end());
    }

    LinkInfo(const Decision* s, const ElementLink<T>& l, ActiveState as, const DecisionIDContainer &decisionIDs)
      : source{s}, link{l}, state{as} {
      decisions.emplace(decisionIDs.begin(), decisionIDs.end());
    }

    bool isValid() const {
      return source && link.isValid();
    }
    /**
     * @brief helper conversion to make it usable with CHECK macro expecting StatusCode
     */
    operator StatusCode () {
      return (isValid() ? StatusCode::SUCCESS : StatusCode::FAILURE);
    }

    /**
     * @brief The node in the NavGraph for this feature
     *
     * Note that when retrieving features for multi-leg chains the same feature can be
     * attached to multiple nodes and only one of those nodes will be returned here.
    */
    const Decision* source{nullptr};
    /// Link to the feature
    ElementLink<T> link;
    /// Was the linked feature active for any requested chains
    ActiveState state{ActiveState::UNSET};
    /// All decision IDs active for this feature. Only available if filled explicitly via constructor
    /// or findLinks called with TrigDefs::fillDecisions.
    std::optional<std::unordered_set<DecisionID>> decisions;
  };
} //> end namespace TrigCompositeUtils

#endif //> !TRIGCOMPOSITEUTILS_LINKINFO_H
