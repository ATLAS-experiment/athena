/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#ifndef L1MUONNSW_MMT_ROAD_H
#define L1MUONNSW_MMT_ROAD_H

#include "MMT_Hit.h"
#include "L1MMDefinitions.h"
#include <cmath>
#include <numeric>
#include <vector>

namespace L1Muon::L1MM {

  /**
   * @brief One (X, U, V) road of the MM Diamond Road algorithm: collects the hits whose slope falls in
   *        its window, ages them out after the coincidence window, and checks the coincidence.
   */
  class MMT_Road {
    public:
      MMT_Road(const char sector, const int roadSize, const int UpX, const int DownX, const int UpUV, const int DownUV, const int xthr, const int uvthr,
               const int iroadx, const int iroadu = -1, const int iroadv = -1);
      ~MMT_Road()=default;

      /// Offer the hits of the current bunch crossing to the road (accepted hits are copied)
      void addHits(const std::vector<const MMT_Hit*>& hits);
      /// Average R/Z slope of the hits of one layer type
      double avgSofXUV(const LayerType type) const;
      bool checkCoincidences(const int bcwind) const {
        return horizontalCheck() && stereoCheck() && matureCheck(bcwind);
      }
      unsigned int countHits() const { return m_road_hits.size(); }
      unsigned int countUHits() const;
      unsigned int countXHits() const;
      bool evaluateLowRes() const;
      bool horizontalCheck() const;
      void incrementAge(const int bcwind);
      const std::vector<MMT_Hit>& getHitVector() const { return m_road_hits; }
      char getSector() const { return m_sector; }
      int getXthreshold() const { return m_xthr; }
      int getUVthreshold() const { return m_uvthr; }
      int iRoadx() const { return m_iroadx; }
      int iRoadu() const { return m_iroadu; }
      int iRoadv() const { return m_iroadv; }
      bool matureCheck(const int bcwind) const;
      double mxl() const;
      bool stereoCheck() const;

    private:
      std::vector<MMT_Hit> m_road_hits;
      double m_slopeXlow{0.};
      double m_slopeXhigh{0.};
      double m_slopeUlow{0.};
      double m_slopeUhigh{0.};
      double m_slopeVlow{0.};
      double m_slopeVhigh{0.};
      int m_iroadx{-1};
      int m_iroadu{-1};
      int m_iroadv{-1};
      int m_xthr{0};
      int m_uvthr{0};
      char m_sector{'-'};
  };

}  // namespace L1Muon::L1MM

#endif  // L1MUONNSW_MMT_ROAD_H
