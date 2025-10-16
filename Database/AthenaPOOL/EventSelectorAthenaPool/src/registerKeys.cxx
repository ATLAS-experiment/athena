/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file  EventSelectorAthenaPool/src/registerKeys.cxx
 * @author scott snyder
 * @date Dec, 2007
 * @brief Helper functions for registering hash keys with the SG service.
 */

#include "registerKeys.h"
#include "PersistentDataModel/DataHeader.h"
#include "StoreGate/StoreGateSvc.h"

//namespace EventSelectorAthenaPoolUtil {

/**
 * @brief Register all hash keys for one DH Element.
 * @param dhe The DataHeader element.
 * @param store The SG store with which the hashes are to be registered.
 */
void EventSelectorAthenaPoolUtil::registerKeys(const DataHeaderElement& dhe, StoreGateSvc* store) {
   const std::vector<DataHeaderElement::sgkey_t>& hashes = dhe.getHashes();
   if (!hashes.empty()) {
      // May be empty if we're reading an old file.
      const std::set<CLID> clids = dhe.getClassIDs();
      size_t i = 0;
      for (const auto& clid : clids) {
         store->registerKey(hashes[i], dhe.getKey(), clid);
         ++i;
      }
   }
}

/**
 * @brief Register all hash keys from a DataHeader.
 * @param dh The DataHeader.
 * @param store The SG store with which the hashes are to be registered.
 */
void EventSelectorAthenaPoolUtil::registerKeys(const DataHeader& dh, StoreGateSvc* store) {
   for (const auto& dhe : dh) {
      registerKeys(dhe, store);
   }
}
//} // namespace EventSelectorAthenaPoolUtil
