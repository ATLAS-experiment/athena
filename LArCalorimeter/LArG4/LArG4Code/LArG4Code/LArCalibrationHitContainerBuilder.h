/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARG4CODE_LARCALIBRATIONHITCONTAINERBUILDER_H
#define LARG4CODE_LARCALIBRATIONHITCONTAINERBUILDER_H

#include "CaloSimEvent/CaloCalibrationHit.h"
#include "CaloSimEvent/CaloCalibrationHitContainer.h"
#include "CaloSimEvent/SrCaloCalibrationHitContainer.h"

#include <memory>
#include <set>
#include <string>
#include <vector>

/**
 * @brief Event-owned builder for calibration-hit containers.
 *
 * The partitioning model mirrors `LArHitContainerBuilder`: regular SDs merge
 * within their own named partitions, while direct contributors use the empty
 * source name and are finalized last.  The template is shared by the standard
 * and SR calibration output container types.  The per-SD partitions are a
 * compatibility layer for the historical partitioned output contract; if hits
 * no longer need to remain distinct and ordered by originating SD, this builder
 * can be reduced to one event-wide merge bucket as well.
 */
template <class HitContainerT>
class LArCalibrationHitContainerBuilderBase : public HitContainerT
{
public:
  using HitContainerT::HitContainerT;
  using hit_ptr_t = std::unique_ptr<CaloCalibrationHit>;

  class LessHit {
  public:
    bool operator()(const hit_ptr_t& lhs, const hit_ptr_t& rhs) const
    {
      return lhs->Less(rhs.get());
    }
  };

  using hits_t = std::set<hit_ptr_t, LessHit>;

  ~LArCalibrationHitContainerBuilderBase() override = default;

  /// Register a regular-SD partition in final output order.
  void RegisterSource(const std::string& sourceName);
  /// Take ownership of a hit and add it to one regular-SD partition.
  void AddHit(const std::string& sourceName, hit_ptr_t hit);
  /// Move merged hits into the persisted container in final output order.
  void Finalize();

private:
  struct Partition
  {
    std::string sourceName;
    hits_t hits;
  };

  static void AddHit(hits_t& hits, hit_ptr_t hit);
  void Finalize(hits_t& hits);
  Partition& FindOrCreatePartition(const std::string& sourceName);

  std::vector<Partition> m_partitions;
  hits_t m_directHits;
};

using LArCalibrationHitContainerBuilder =
  LArCalibrationHitContainerBuilderBase<CaloCalibrationHitContainer>;
using LArSrCalibrationHitContainerBuilder =
  LArCalibrationHitContainerBuilderBase<SrCaloCalibrationHitContainer>;

#endif
