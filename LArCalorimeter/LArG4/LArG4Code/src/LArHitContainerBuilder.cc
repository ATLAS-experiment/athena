/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "LArG4Code/LArHitContainerBuilder.h"

#include <algorithm>
#include <utility>

void LArHitContainerBuilder::RegisterSource(const std::string& sourceName)
{
  if (sourceName.empty()) {
    return;
  }
  (void)FindOrCreatePartition(sourceName);
}

void LArHitContainerBuilder::AddHit(const std::string& sourceName,
                                    hit_ptr_t hit,
                                    G4int timeBin)
{
  if (sourceName.empty()) {
    AddHit(m_directHits, std::move(hit), timeBin);
    return;
  }

  AddHit(FindOrCreatePartition(sourceName).timeBins, std::move(hit), timeBin);
}

void LArHitContainerBuilder::Finalize()
{
  for (auto& partition : m_partitions) {
    Finalize(partition.timeBins);
  }
  Finalize(m_directHits);
  m_partitions.clear();
}

void LArHitContainerBuilder::AddHit(timeBins_t& timeBins,
                                    hit_ptr_t hit,
                                    G4int timeBin)
{
  hits_t& hitCollection = timeBins[timeBin];

  auto bookmark = hitCollection.lower_bound(hit);
  if (bookmark == hitCollection.end() || !(*bookmark)->Equals(hit.get())) {
    hitCollection.insert(bookmark, std::move(hit));
  } else {
    (*bookmark)->Add(hit.get());
  }
}

void LArHitContainerBuilder::Finalize(timeBins_t& timeBins)
{
  for (auto& timeBinHits : timeBins) {
    hits_t& hitSet = timeBinHits.second;
    while (!hitSet.empty()) {
      auto node = hitSet.extract(hitSet.begin());
      hit_ptr_t hit = std::move(node.value());
      hit->finalize();
      this->push_back(std::move(hit));
    }
  }
  timeBins.clear();
}

LArHitContainerBuilder::Partition&
LArHitContainerBuilder::FindOrCreatePartition(const std::string& sourceName)
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
