/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HITMANAGEMENT_HITCOLLECTIONMAP_H
#define HITMANAGEMENT_HITCOLLECTIONMAP_H

#include <functional>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include <GaudiKernel/EventContext.h>
#include <GaudiKernel/ThreadLocalContext.h>
#include "HitManagement/AthenaHitsVector.h"
#include "StoreGate/WriteHandle.h"

/// Small wrapper around hit collection map to facilitate accessing the hit collection
class HitCollectionMap
{
 public:
  using Storage = std::unordered_map<std::string, std::unique_ptr<HitsVectorBase>>;
  using StorageIterator = typename Storage::iterator;

  /**
   * @brief Insert the hit collection for a given SDs.
   */
  std::pair<StorageIterator, bool> Insert(std::string const& hitCollectionName, std::unique_ptr<HitsVectorBase> hitCollection) {
    // Store the hit collection in a map, using the hitCollectionName as key.
    return m_outputCollections.insert({hitCollectionName, std::move(hitCollection)});
  }

  /**
   * @brief Insert a container in the map with in-place construction.
   */
  template<class HitCollectionT, class... CollectionArgs>
  std::pair<StorageIterator, bool> Emplace(std::string const& hitCollectionName, CollectionArgs&&... args) {
    static_assert(
        std::is_base_of_v<HitsVectorBase, HitCollectionT>,
        "HitCollectionT must be derived from HitsVectorBase");
        return m_outputCollections.emplace(
            hitCollectionName, std::make_unique<HitCollectionT>(std::forward<CollectionArgs>(args)...));
  }

  /**
   * @brief Get the hit collection for a given SDs.
   */
  template <class T>
  T* Find(std::string const& hitCollectionName) {
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
  std::unique_ptr<T> Extract(std::string const& hitCollectionName) {
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

  /**
   * @brief Record the hit collection hitCollectionName to the StoreGate sgKey
   */
  template <class T>
  void Record(std::string const& sgKey, std::string const& hitCollectionName, EventContext const& ctx) {
    SG::WriteHandle<T> handle(sgKey, ctx);
    handle = Extract<T>(hitCollectionName);
  }

  /**
   * @brief Overload for Record with the same name for the SG key and hit collection name.
   */
  template <class T>
  void Record(std::string const& hitCollectionName) {
    Record<T>(hitCollectionName, hitCollectionName, Gaudi::Hive::currentContext());
  }

  /**
   * @brief Record the hit collection hitCollectionName to the StoreGate sgKey, applying a transformation
   * function to the hit collection before recording it.
   */
  template <class T>
  void TransformAndRecord(
      std::string const& sgKey,
      std::string const& hitCollectionName,
      EventContext const& ctx,
      std::function<void(T&)> transform) {
    auto hitColl = Extract<T>(hitCollectionName);
    transform(*hitColl);
    SG::WriteHandle<T> handle(sgKey, ctx);
    handle = std::move(hitColl);
  }

  /**
   * @brief Overload for TransformAndRecord with the same name for the SG key and hit collection name.
   */
  template <class T>
  void TransformAndRecord(std::string const& hitCollectionName, std::function<void(T&)> transform) {
    TransformAndRecord(hitCollectionName, hitCollectionName, Gaudi::Hive::currentContext(), std::move(transform));
  }

 private:
   // Holds the hits container for this event.
   Storage m_outputCollections;
};

#endif
