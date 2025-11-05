/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGCOMPOSITEUTILS_TYPELESSLINKINFO_H
#define TRIGCOMPOSITEUTILS_TYPELESSLINKINFO_H

#include "xAODTrigger/TrigComposite.h"
#include "CxxUtils/sgkey_utilities.h"
#include "ActiveState.h"

#include <unordered_set>

namespace TrigCompositeUtils {
  /**
   * @brief Helper to keep a Decision object, ElementLink and ActiveState (with respect to some requested ChainGroup) linked together (for convenience)
   * This is the internal type erased version, a typed version also exists which is exposed by the public facing API.
   **/
  struct TypelessLinkInfo {
    using index_type = uint32_t;

    TypelessLinkInfo() = default;
    TypelessLinkInfo(const Decision* s, const SG::sgkey_t k, const CLID c, const index_type i, ActiveState as = ActiveState::UNSET)
    : source{s}, key{k}, clid{c}, index{i}, state{as}
    {
      if (s) decisions.emplace(s->decisions().begin(), s->decisions().end());
    }

    TypelessLinkInfo(const Decision* s, const SG::sgkey_t k, const CLID c, const index_type i, ActiveState as, const DecisionIDContainer &decisionIDs) 
    : source{s}, key{k}, clid{c}, index{i}, state{as}
    {
      decisions.emplace(decisionIDs.begin(), decisionIDs.end());
    }

    /**
     * @brief The node in the NavGraph for this feature
     *
     * Note that when retrieving features for multi-leg chains the same feature can be
     * attached to multiple nodes and only one of those nodes will be returned here.
    */
    const Decision* source{nullptr};
    /// Type erased link to the feature
    SG::sgkey_t key;
    CLID clid;
    index_type index{};
    /// Was the linked feature active for any requested chains
    ActiveState state{ActiveState::UNSET};
    /// All decision IDs active for this feature. Only available if filled explicitly via constructor
    /// or findLinks called with TrigDefs::fillDecisions.
    std::optional<std::unordered_set<DecisionID>> decisions;
  };
} //> end namespace TrigCompositeUtils

#endif //> !TRIGCOMPOSITEUTILS_TYPELESSLINKINFO_H
