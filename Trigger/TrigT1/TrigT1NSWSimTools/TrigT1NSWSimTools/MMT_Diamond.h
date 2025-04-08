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
  slope_t(uint64_t ev=0, int bc=-1, unsigned int tC=999, unsigned int rC=999, int iX=-1, int iU=-1, int iV=-1, unsigned int uvb=999, unsigned int xb=999,
          unsigned int uvm=999, unsigned int xm=999, int age=-1, double mxl=999., double my=999., double uavg=999., double vavg=999., double mx=999.,
          double th=999., double eta=999., double dth=999., char side='-', double phi=999., double phiS=999., bool lowRes=false);
  uint64_t event;
  int BC;
  unsigned int totalCount;
  unsigned int realCount;
  int iRoad;
  int iRoadu;
  int iRoadv;
  unsigned int uvbkg;
  unsigned int xbkg;
  unsigned int uvmuon;
  unsigned int xmuon;
  int age;
  double mxl;
  double my;
  double uavg;
  double vavg;
  double mx;
  double theta;
  double eta;
  double dtheta;
  char side;
  double phi;
  double phiShf;
  bool lowRes;
};

class MMT_Diamond : public AthMessaging {
  public:
    MMT_Diamond(const int diamXthreshold, const bool uv, const int diamUVthreshold, const int roadSize,
                const int olapEtaUp, const int olapEtaDown, const int olapStereoUp, const int olapStereoDown);
    ~MMT_Diamond() = default;

    void createRoads(std::vector<std::shared_ptr<MMT_Road> >& roads, const bool isLarge) const;
    void findDiamonds(std::vector<std::shared_ptr<MMT_Hit> >& hits, std::vector<std::shared_ptr<MMT_Road> >& roads, std::vector<slope_t>& diamondSlopes, const int sectorPhi) const;
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
