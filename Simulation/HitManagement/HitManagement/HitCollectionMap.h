/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HITMANAGEMENT_HITCOLLECTIONMAP_H
#define HITMANAGEMENT_HITCOLLECTIONMAP_H

#include <memory>
#include <type_traits>
#include <unordered_map>

#include "HitManagement/AthenaHitsVector.h"

/// Small wrapper around hit collection map to facilitate accessing the hit collection
class HitCollectionMap
{
 public:
  /**
   * @brief Set the hit collection for a given SDs.
   */
  void SetSDHitCollection(std::string const& hitCollectionName,
                          std::unique_ptr<HitsVectorBase> hitCollection) {
    // Store the hit collection in a map, using the hitCollectionName as key.
    m_outputCollections[hitCollectionName] = std::move(hitCollection);
  }
  /**
   * @brief Get the hit collection for a given SDs.
   */
  template <class T>
  T* GetSDHitCollection(std::string const& hitCollectionName) {
    static_assert(
        std::is_base_of_v<HitsVectorBase, T>,
        "T must be derived from HitsVectorBase");
    auto it = m_outputCollections.find(hitCollectionName);
    if (it != m_outputCollections.end()) {
      return static_cast<T*>(it->second.get());
    }
    return nullptr;
  }

  /**
   * @brief Extract the hit collection for a given SDs downcasted to the
   * template parameter.
   */
  template <class T>
  std::unique_ptr<T> ExtractSDHitCollection(
      std::string const& hitCollectionName) {
    static_assert(
        std::is_base_of_v<HitsVectorBase, T>,
        "T must be derived from HitsVectorBase");
    if (auto handle = m_outputCollections.extract(hitCollectionName)) {
      // we can static cast, the caller must know the type of the hit collection
      // for a given key
      return std::unique_ptr<T>(static_cast<T*>(handle.mapped().release()));
    }
    return nullptr;
  }
 private:
   // Holds the hits container for this event.
   std::unordered_map<std::string, std::unique_ptr<HitsVectorBase>>
     m_outputCollections;
};

#endif
