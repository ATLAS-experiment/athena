/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "LArG4Code/LArCalibrationHitContainerBuilder.h"

#include <algorithm>
#include <utility>

template <class HitContainerT>
void LArCalibrationHitContainerBuilderBase<HitContainerT>::RegisterSource(
    const std::string& sourceName)
{
  if (sourceName.empty()) {
    return;
  }
  (void)FindOrCreatePartition(sourceName);
}

template <class HitContainerT>
void LArCalibrationHitContainerBuilderBase<HitContainerT>::AddHit(
    const std::string& sourceName, hit_ptr_t hit)
{
  if (sourceName.empty()) {
    AddHit(m_directHits, std::move(hit));
    return;
  }

  AddHit(FindOrCreatePartition(sourceName).hits, std::move(hit));
}

template <class HitContainerT>
void LArCalibrationHitContainerBuilderBase<HitContainerT>::Finalize()
{
  for (auto& partition : m_partitions) {
    Finalize(partition.hits);
  }
  Finalize(m_directHits);
  m_partitions.clear();
}

template <class HitContainerT>
void LArCalibrationHitContainerBuilderBase<HitContainerT>::AddHit(
    hits_t& hits, hit_ptr_t hit)
{
  auto bookmark = hits.lower_bound(hit);
  if (bookmark == hits.end() || !(*bookmark)->Equals(hit.get())) {
    hits.insert(bookmark, std::move(hit));
  } else {
    (*bookmark)->Add(hit.get());
  }
}

template <class HitContainerT>
void LArCalibrationHitContainerBuilderBase<HitContainerT>::Finalize(hits_t& hits)
{
  while (!hits.empty()) {
    auto node = hits.extract(hits.begin());
    this->push_back(std::move(node.value()));
  }
}

template <class HitContainerT>
typename LArCalibrationHitContainerBuilderBase<HitContainerT>::Partition&
LArCalibrationHitContainerBuilderBase<HitContainerT>::FindOrCreatePartition(
    const std::string& sourceName)
{
  auto partition = std::find_if(
    m_partitions.begin(), m_partitions.end(),
    [&sourceName](const Partition& candidate)
    {
      return candidate.sourceName == sourceName;
    });
  if (partition == m_partitions.end()) {
    m_partitions.push_back({sourceName, {}});
    return m_partitions.back();
  }
  return *partition;
}

template class LArCalibrationHitContainerBuilderBase<CaloCalibrationHitContainer>;
template class LArCalibrationHitContainerBuilderBase<SrCaloCalibrationHitContainer>;
