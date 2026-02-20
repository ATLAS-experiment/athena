/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#ifndef MMT_ROAD_H
#define MMT_ROAD_H

#include "MMT_Hit.h"
#include <cmath>
#include <numeric>

class MMT_Road {
  public:
    MMT_Road(const char sector, const int roadSize, const int UpX, const int DownX, const int UpUV, const int DownUV, const int xthr, const int uvthr,
             const int iroadx, const int iroadu = -1, const int iroadv = -1);
    ~MMT_Road()=default;

    void addHits(const std::vector<std::shared_ptr<MMT_Hit> > &hits);
    double avgSofXUV(const char type) const;
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
    double m_slopeXlow, m_slopeXhigh, m_slopeUlow, m_slopeUhigh, m_slopeVlow, m_slopeVhigh;
    int m_iroadx, m_iroadu, m_iroadv;
    int m_xthr, m_uvthr;
    char m_sector;
};
#endif
