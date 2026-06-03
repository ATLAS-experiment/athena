/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDC_SD_ZDC_HITCOLLECTIONBUILDERS_H
#define ZDC_SD_ZDC_HITCOLLECTIONBUILDERS_H

#include "CaloSimEvent/CaloCalibrationHitContainer.h"
#include "ZDC_SimEvent/ZDC_SimFiberHit_Collection.h"

#include <cstdint>
#include <map>
#include <memory>

class ZDC_SimFiberHitCollectionBuilder : public ZDC_SimFiberHit_Collection
{
public:
  using ZDC_SimFiberHit_Collection::ZDC_SimFiberHit_Collection;

  void AddHit(const Identifier& id, const float energy)
  {
    const uint32_t hash = id.get_identifier32().get_compact();
    auto [it, inserted] = m_hitMap.try_emplace(hash, id, 1, energy);
    if (!inserted) {
      it->second.Add(1, energy);
    }
  }

  void Finalize()
  {
    for (const auto& [hash, hit] : m_hitMap) {
      (void)hash;
      Emplace(hit);
    }
    m_hitMap.clear();
  }

private:
  std::map<uint32_t, ZDC_SimFiberHit> m_hitMap;
};

class ZDC_CalibrationHitContainerBuilder : public CaloCalibrationHitContainer
{
public:
  using CaloCalibrationHitContainer::CaloCalibrationHitContainer;

  void MergeHit(std::unique_ptr<CaloCalibrationHit> hit)
  {
    const uint32_t hash = hit->cellID().get_identifier32().get_compact();
    auto it = m_hitMap.find(hash);
    if (it == m_hitMap.end()) {
      m_hitMap.emplace(hash, std::move(hit));
    } else {
      it->second->Add(hit.get());
    }
  }

  void Finalize()
  {
    for (auto& [hash, hit] : m_hitMap) {
      (void)hash;
      push_back(hit.release());
    }
    m_hitMap.clear();
  }

private:
  std::map<uint32_t, std::unique_ptr<CaloCalibrationHit>> m_hitMap;
};

#endif
