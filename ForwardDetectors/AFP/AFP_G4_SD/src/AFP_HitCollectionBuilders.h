/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef AFP_G4_SD_AFP_HITCOLLECTIONBUILDERS_H
#define AFP_G4_SD_AFP_HITCOLLECTIONBUILDERS_H

#include "AFP_SimEv/AFP_SIDSimHitCollection.h"
#include "AFP_SimEv/AFP_TDSimHitCollection.h"

#include <array>

class AFP_TDSimHitCollectionBuilder : public AFP_TDSimHitCollection
{
public:
  static constexpr int TDMaxCnt = 4000;

  using AFP_TDSimHitCollection::AFP_TDSimHitCollection;

  bool HasReachedLimit(const int stationID, const int quarticID, const int detectorID) const
  {
    const int index = CounterIndex(stationID, quarticID);
    return index >= 0 && detectorID >= 0 && detectorID < static_cast<int>(m_hitsPerBar[index].size()) &&
           m_hitsPerBar[index][detectorID] >= TDMaxCnt;
  }

  void CountHit(const int stationID, const int quarticID, const int detectorID)
  {
    ++m_numberOfHits;
    const int index = CounterIndex(stationID, quarticID);
    if (index >= 0 && detectorID >= 0 && detectorID < static_cast<int>(m_hitsPerBar[index].size())) {
      ++m_hitsPerBar[index][detectorID];
    }
  }

  void CountHit() { ++m_numberOfHits; }

  int NumberOfHits() const { return m_numberOfHits; }

private:
  static int CounterIndex(const int stationID, const int quarticID)
  {
    if (stationID == 0 && quarticID == 0) return 0;
    if (stationID == 0 && quarticID == 1) return 1;
    if (stationID == 3 && quarticID == 0) return 2;
    if (stationID == 3 && quarticID == 1) return 3;
    return -1;
  }

  int m_numberOfHits{};
  std::array<std::array<int, 32>, 4> m_hitsPerBar{};
};

class AFP_SIDSimHitCollectionBuilder : public AFP_SIDSimHitCollection
{
public:
  static constexpr int SiDMaxCnt = 1000;

  using AFP_SIDSimHitCollection::AFP_SIDSimHitCollection;

  bool HasReachedLimit(const int stationID) const
  {
    return stationID >= 0 && stationID < static_cast<int>(m_hitsPerStation.size()) &&
           m_hitsPerStation[stationID] >= SiDMaxCnt;
  }

  void CountHit(const int stationID)
  {
    ++m_numberOfHits;
    if (stationID >= 0 && stationID < static_cast<int>(m_hitsPerStation.size())) {
      ++m_hitsPerStation[stationID];
    }
  }

  int NumberOfHits() const { return m_numberOfHits; }

private:
  int m_numberOfHits{};
  std::array<int, 4> m_hitsPerStation{};
};

#endif
