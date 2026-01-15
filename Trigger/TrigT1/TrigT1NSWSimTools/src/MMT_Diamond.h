/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#ifndef MMT_DIAMOND_H
#define MMT_DIAMOND_H

#include "AthenaBaseComps/AthMessaging.h"
#include "MMT_Road.h"
#include <cmath>
#include <vector>

struct slope_t {
  uint64_t event{0};
  int BC{-1};
  unsigned int totalCount{999};
  unsigned int realCount{999};
  int iRoad{-1};
  int iRoadu{-1};
  int iRoadv{-1};
  unsigned int uvbkg{999};
  unsigned int xbkg{999};
  unsigned int uvmuon{999};
  unsigned int xmuon{999};
  int age{-1};
  double mxl{999};
  double my{999};
  double uavg{999};
  double vavg{999};
  double mx{999};
  double theta{999};
  double eta{999};
  double dtheta{999};
  double phi{999};
  double phiShf{999};
  char side{'-'};
  bool lowRes{false};
};

class MMT_Diamond : public AthMessaging {
  public:
    MMT_Diamond(const int diamXthreshold, const bool uv, const int diamUVthreshold, const int roadSize,
                const int olapEtaUp, const int olapEtaDown, const int olapStereoUp, const int olapStereoDown);
    ~MMT_Diamond() = default;

    void createRoads(std::vector<MMT_Road>& roads, const bool isLarge, const bool isEta1) const;
    void findDiamonds(std::vector<std::shared_ptr<MMT_Hit> >& hits, std::vector<MMT_Road>& roads, std::vector<slope_t>& diamondSlopes, const int sectorPhi) const;
    double phiShift(const int n, const double phi, const char side) const;

    int getRoadSize() const { return m_roadSize; }
    unsigned int getXthreshold() const { return m_xthr; }
    unsigned int getUVthreshold() const { return m_uvthr; }

  private:
    bool m_uvflag{};
    int m_roadSize{}, m_roadSizeUpX{}, m_roadSizeDownX{}, m_roadSizeUpUV{}, m_roadSizeDownUV{};
    int m_xthr{}, m_uvthr{};
};
#endif
