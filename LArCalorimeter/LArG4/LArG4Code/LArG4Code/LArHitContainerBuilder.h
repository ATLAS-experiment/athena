/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARG4CODE_LARHITCONTAINERBUILDER_H
#define LARG4CODE_LARHITCONTAINERBUILDER_H

#include "LArSimEvent/LArHit.h"
#include "LArSimEvent/LArHitContainer.h"

#include "G4Types.hh"

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

/**
 * @brief Event-owned builder for standard LAr hits.
 *
 * The builder owns the merge state for the full Athena event.  Regular
 * sensitive detectors register named partitions during Geant4 initialization;
 * each partition merges hits independently and partitions are finalized in
 * registration order.  Hits with an empty source name go to the direct bucket,
 * which is finalized after regular SD partitions and is used by contributors
 * such as frozen-shower fast simulation.
 *
 * Named per-SD partitions are retained only to preserve the historical output
 * contract: hits from different SDs remain distinct and are emitted in SD setup
 * order.  If that compatibility requirement is intentionally dropped, the
 * implementation can be simplified to a single event-wide merge bucket.
 */
class LArHitContainerBuilder : public LArHitContainer
{
public:
  using LArHitContainer::LArHitContainer;
  using hit_ptr_t = std::unique_ptr<LArHit>;

  class LessHit {
  public:
    bool operator()(const hit_ptr_t& lhs, const hit_ptr_t& rhs) const
    {
      return lhs->Less(rhs.get());
    }
  };

  using hits_t = std::set<hit_ptr_t, LessHit>;
  using timeBins_t = std::map<G4int, hits_t>;

  ~LArHitContainerBuilder() override = default;

  /// Register a regular-SD partition in final output order.
  void RegisterSource(const std::string& sourceName);
  /// Take ownership of a hit and add it to one regular-SD partition.
  void AddHit(const std::string& sourceName, hit_ptr_t hit, G4int timeBin);
  /// Move merged hits into the persisted container in final output order.
  void Finalize();

private:
  struct Partition
  {
    std::string sourceName;
    timeBins_t timeBins;
  };

  static void AddHit(timeBins_t& timeBins, hit_ptr_t hit, G4int timeBin);
  void Finalize(timeBins_t& timeBins);
  Partition& FindOrCreatePartition(const std::string& sourceName);

  std::vector<Partition> m_partitions;
  timeBins_t m_directHits;
};

#endif
