/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HITMANAGEMENT_HITCOLLECTIONMAP_H
#define HITMANAGEMENT_HITCOLLECTIONMAP_H

#include <functional>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include <GaudiKernel/EventContext.h>
#include <GaudiKernel/StatusCode.h>
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
  template<AthHitVec::isHitVectorBase HitCollectionT, class... CollectionArgs>
  std::pair<StorageIterator, bool> Emplace(std::string const& hitCollectionName, CollectionArgs&&... args) {
      return m_outputCollections.emplace(
            hitCollectionName, std::make_unique<HitCollectionT>(std::forward<CollectionArgs>(args)...));
  }

  /**
   * @brief Get the hit collection for a given SDs.
   */
  template <AthHitVec::isHitVectorBase T>
  T* Find(std::string const& hitCollectionName) {
    auto it = m_outputCollections.find(hitCollectionName);
    if (it != m_outputCollections.end()) {
      return dynamic_cast<T*>(it->second.get());
    }
    return nullptr;
  }

  /**
   * @brief Extract the hit collection for a given SDs downcasted to the
   * template parameter.
   */
  template <AthHitVec::isHitVectorBase T>
  std::unique_ptr<T> Extract(std::string const& hitCollectionName) {
    auto it = m_outputCollections.find(hitCollectionName);
    if (it == m_outputCollections.end()) {
      return nullptr;
    }

    auto* baseCollection = it->second.release();
    auto* collection = dynamic_cast<T*>(baseCollection);
    if (!collection) {
      it->second.reset(baseCollection);
      return nullptr;
    }

    m_outputCollections.erase(it);
    return std::unique_ptr<T>{collection};
  }

  /**
   * @brief Record the hit collection hitCollectionName to the StoreGate sgKey
   */
  template <AthHitVec::isHitVectorBase T>
  StatusCode Record(std::string const& sgKey, std::string const& hitCollectionName, EventContext const& ctx) {
    SG::WriteHandle<T> handle{sgKey, ctx};
    return handle.record(Extract<T>(hitCollectionName));
  }

  /**
   * @brief Overload for Record with the same name for the SG key and hit collection name.
   */
  template <AthHitVec::isHitVectorBase T>
  StatusCode Record(std::string const& hitCollectionName) {
    return Record<T>(hitCollectionName, hitCollectionName, Gaudi::Hive::currentContext());
  }

  /**
   * @brief Record the hit collection hitCollectionName to the StoreGate sgKey, applying a transformation
   * function to the hit collection before recording it.
   */
  template <class T>
  StatusCode TransformAndRecord(
      std::string const& sgKey,
      std::string const& hitCollectionName,
      EventContext const& ctx,
      std::function<void(T&)> transform) {
    auto hitColl = Extract<T>(hitCollectionName);
    transform(*hitColl);
    SG::WriteHandle<T> handle(sgKey, ctx);
    return handle.record(std::move(hitColl));
  }

  /**
   * @brief Overload for TransformAndRecord with the same name for the SG key and hit collection name.
   */
  template <class T>
  StatusCode TransformAndRecord(std::string const& hitCollectionName, std::function<void(T&)> transform) {
    return TransformAndRecord(hitCollectionName, hitCollectionName, Gaudi::Hive::currentContext(), std::move(transform));
  }

 private:
   // Holds the hits container for this event.
   Storage m_outputCollections;
};

#endif
